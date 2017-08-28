/*
 * TODO: Copyright (C) 2017 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 ************************************************************************************
 *
 * This is the worker class that contains all of the logic for the interface manager.
 **/

package gov.dot.fhwa.saxton.carma.interfacemgr;

import org.apache.commons.logging.Log;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class InterfaceWorker {

    protected ArrayList<DriverInfo>         drivers_ = new ArrayList<DriverInfo>();
    protected int                           waitTime_ = 10;  //seconds that we must wait after last driver registered
    protected IInterfaceMgr                 mgr_;
    protected Log                           log_;
    protected long                          startedWaiting_ = System.nanoTime();
    protected boolean                       systemOperational_ = false;

    public InterfaceWorker(IInterfaceMgr mgr, Log log) {
        mgr_ = mgr;
        log_ = log;
    }

    /**
     * Stores the desired wait time for drivers to be discovered before declaring the system operational.
     *
     * @param wait - number of seconds to wait for new drivers to announce themselves
     */
    public void setWaitTime(int wait) {
        waitTime_ = wait;
        if (log_ != null) {
            log_.debug("InterfaceWorker: driver wait time set at " + wait + " seconds.");
        }
    }

    /**
     * Updates the info known about a current driver.  Note that this is expected to be called frequently throughout
     * the life of the node.
     *
     * @param newDriver - all available details about the driver publishing its status
     */
    public void handleNewDriverStatus(DriverInfo newDriver) {
        //if we already know about this driver then
        int index = getDriverIndex(newDriver.getName());
        if (index >= 0) {

            //if its info has changed then
            if (!isSame(newDriver, drivers_.get(index))) {
                //record the updates
                drivers_.set(index, newDriver);
                log_.debug("InterfaceWorker.handleNewDriverStatus: status changed for " + newDriver.getName());
            }
        //else it's a newly discovered driver
        }else {
            //get its list of capabilities (getDriverApi)
            List<String> cap = mgr_.getDriverApi(newDriver.getName());

            //add the info to the list of known drivers
            DriverInfo d = drivers_.get(index);
            d.setCapabilities(cap);

            //request InterfaceMgr to bind with it
            mgr_.bindWithDriver(d.getName());

            //reset the wait timer
            startedWaiting_ = System.nanoTime();
        }
    }

    /**
     * Updates the status of a driver that has broken its bond.  A broken bond simply indicates a status change for
     * that driver; it may still be alive and functioning, but at a different level of capability (it may have even
     * corrected a previous deficiency, e.g. gone from DEGRADED to fully functional).
     *
     * @param driverName - unique ID of the driver
     */
    public void handleBrokenBond(String driverName) throws Exception {
        //look up the driver and determine its new set of properties (they will have been stored via the
        // /driver_discovery topic update)
        int index = getDriverIndex(driverName);
        if (index < 0) {
            String msg = "InterfaceWorker.handleBrokenBond can't find driver" + driverName;
            log_.warn(msg);
            throw new Exception(msg);
        }
        DriverInfo driver = drivers_.get(index);

        //if functionality is totally unavailable then
        DriverState state = driver.getStatus();
        if (state == DriverState.FAULT  ||  state == DriverState.OFF) {
            //remove the driver from the list of available drivers
            drivers_.remove(index);
            log_.warn("InterfaceWorker.handleBrokenBond: driver " + driverName + " is no longer available.");
        }

        //if the system is OPERATIONAL and the new state of this driver is not "fully operational" then
        if (systemOperational_  &&  state != DriverState.OPERATIONAL) {

            //formulate an alert message at the appropriate level depending on the type of driver that is reporting
            // (notifyBrokenBond)
            AlertSeverity sev = AlertSeverity.CAUTION;
            String msg = null;

            if (driver.isController()) {
                if (state == DriverState.FAULT  ||  state == DriverState.OFF) {
                    sev = AlertSeverity.FATAL;
                    msg = "Controller driver " + driverName + " is no longer available.";
                }else if (state == DriverState.DEGRADED) {
                    sev = AlertSeverity.WARNING;
                    msg = "Controller driver " + driverName + " is operating on degraded capability.";
                }
            }

            if (driver.isPosition()) {
                String level = (state == DriverState.DEGRADED) ? "degraded" : "gone";
                sev = AlertSeverity.WARNING;
                msg = "Position driver " + driverName + " is " + level;
            }

            if (driver.isComms()) {
                String level = (state == DriverState.DEGRADED) ? "degraded" : "gone";
                sev = AlertSeverity.WARNING;
                msg = "Comms driver " + driverName + " is " + level;
            }

            if (driver.isSensor()  ||  driver.isCan()) {
                String level = (state == DriverState.DEGRADED) ? "degraded" : "gone";
                sev = AlertSeverity.CAUTION;
                msg = "Driver " + driverName + " is " + level;
            }

            mgr_.notifyBrokenBond(sev, msg);
        }
    }

    /**
     * Returns a list of drivers that each provide all of the given capabilities once the system is OPERATIONAL.
     *
     * @param capabilities - a list of capabilities that must be met (inclusive)
     * @return - a list of driver names that meet all the capabilities
     */
    public List<String> getDrivers(DriverCategory cat, List<String> capabilities) {
        List<String> result = new ArrayList<String>();

        //if the system is ready for operation then
        if (systemOperational_) {

            //loop through all known drivers
            for (int index = 0;  index < drivers_.size();  ++index) {

                //if it matches the requested category then
                DriverInfo driver = drivers_.get(index);
                if (hasCategory(driver, cat)) {

                    //loop through all requested capabilities
                    boolean foundAllCapabilities = true;
                    List<String> driverCaps = driver.getCapabilities();
                    for (int req = 0;  req < capabilities.size();  ++req) {

                        //if this driver cannot provide this capability then break out of loop
                        boolean foundThisCapability = false;
                        for (int driverIndex = 0;  driverIndex < driverCaps.size();  ++driverIndex) {
                            if (capabilities.get(req).equals(driverCaps.get(driverIndex))) {
                                foundThisCapability = true;
                                break;
                            }
                        }
                        if (!foundThisCapability) {
                            foundAllCapabilities = false;
                            break;
                        }
                    }

                    //if the driver is satisfactory then
                    if (foundAllCapabilities) {
                        //add the driver to the return list
                        result.add(driver.getName());
                    }
                }
            }
        }

        return result;
    }

    /**
     * Indicates whether the system has just become OPERATIONAL.  If it is either not ready or it has
     * been OPERATIONAL in a previous call then it will return false.
     * Waits an additional amount of time after the latest detected driver in case any further drivers
     * come on line.
     *
     * @return - true if the system is newly OPERATIONAL (one call only will return true)
     */
    public boolean isSystemReady() {
        //if system is not yet OPERATIONAL then
        if (!systemOperational_) {

            //if wait timer has expired then
            long elapsed = System.nanoTime() - startedWaiting_;
            if (TimeUnit.NANOSECONDS.toSeconds(elapsed) > waitTime_) {

                //indicate that it is now OPERATIONAL
                systemOperational_ = true;
                //log the time required to get to this point
                log_.info("///// InterfaceWorker declaring SYSTEM OPERATIONAL.");
            }
        }

        return systemOperational_;

    }

    //////////

    /**
     * Returns the index in the drivers_ array that matches the name of the given driver.
     *
     * @param givenName - the one we are looking for
     * @return - index of the driver that matches given
     */
    protected int getDriverIndex(String givenName) {

        if (drivers_.size() > 0) {
            for (int i = 0;  i < drivers_.size();  ++i) {
                if (drivers_.get(i).getName().equals(givenName)) {
                    return i;
                }
            }
        }

        return -1;
    }

    /**
     * Determines if two driver specifications are identical.
     *
     * @param a - the first driver
     * @param b - the second driver
     * @return - true if they are identical; false otherwise
     */
    protected boolean isSame(DriverInfo a, DriverInfo b) {
        if (!a.getName().equals(b.getName())) {
            return false;
        }
        if (a.getStatus() != b.getStatus()) {
            return false;
        }
        if (a.isCan() != b.isCan()) {
            return false;
        }
        if (a.isComms() != b.isComms()) {
            return false;
        }
        if (a.isController() != b.isController()) {
            return false;
        }
        if (a.isPosition() != b.isPosition()) {
            return false;
        }
        if (a.isSensor() != b.isSensor()) {
            return false;
        }

        //look through all of the individual "capabilities" (api messages)
        List<String> aCapList = a.getCapabilities();
        List<String> bCapList = b.getCapabilities();
        if (aCapList.size() != bCapList.size()) {
            return false;
        }

        for (String aCap : aCapList) {
            boolean found = false;
            for (String bCap : bCapList) {
                if (aCap.equals(bCap)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }

        return true;
    }

    /**
     * Determines if the given category is provided by the driver.
     *
     * @param d - driver in question
     * @param cat - category in question
     * @return true if the driver does fall into the given category
     */
    protected boolean hasCategory(DriverInfo d, DriverCategory cat) {
        if ((cat == DriverCategory.CONTROLLER   &&  d.isController())   ||
                (cat == DriverCategory.COMMS    &&  d.isComms())        ||
                (cat == DriverCategory.CAN      &&  d.isCan())          ||
                (cat == DriverCategory.POSITION &&  d.isPosition())     ||
                (cat == DriverCategory.SENSOR   &&  d.isSensor())) {
            return true;
        }

        return false;
    }
}
