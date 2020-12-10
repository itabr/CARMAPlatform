# pragma once
/*
 * Copyright (C) 2019-2020 LEIDOS.
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
 */

#include <vector>
#include <cav_msgs/TrajectoryPlan.h>
#include <cav_msgs/TrajectoryPlanPoint.h>
#include <cav_msgs/Plugin.h>
#include <boost/shared_ptr.hpp>
#include <carma_utils/CARMAUtils.h>
#include <boost/geometry.hpp>
#include <carma_wm/Geometry.h>
#include <cav_srvs/PlanTrajectory.h>
#include <carma_wm/WMListener.h>
#include <functional>
#include <carma_wm/CARMAWorldModel.h>

#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/LineString.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>


namespace stopandwait_plugin
{
    using PublishPluginDiscoveryCB = std::function<void(const cav_msgs::Plugin&)>;
    struct PointSpeedPair
    {
        lanelet::BasicPoint2d point;
        double speed=0;
    };

    struct DiscreteCurve
    {
        Eigen::Isometry2d frame;
        std::vector<PointSpeedPair> points;
    };

    class StopandWait
    {
    public:
        
        StopandWait(){};
    /**
        * \brief Constructor
        * 
        * \param wm Pointer to intialized instance of the carma world model for accessing semantic map data
        * \param plugin_discovery_publisher Callback which will publish the current plugin discovery state
        */
        StopandWait(carma_wm::WorldModelConstPtr wm, PublishPluginDiscoveryCB plugin_discovery_publisher);


          /**
         * \brief Service callback for trajectory planning
         * 
         * \param req The service request
         * \param resp The service response
         * 
         * \return True if success. False otherwise
         */ 
        bool plan_trajectory_cb(cav_srvs::PlanTrajectoryRequest& req, cav_srvs::PlanTrajectoryResponse& resp);
        
        /**
         * \brief Method to call at fixed rate in execution loop. Will publish plugin discovery updates
         * 
         * \return True if the node should continue running. False otherwise
         */ 
        bool onSpin();

        /**
         * \brief Converts a set of requested STOP_AND_WAIT maneuvers to point speed limit pairs. 
         * 
         * \param maneuvers The list of maneuvers to convert
         * \param max_starting_downtrack The maximum downtrack that is allowed for the first maneuver. This should be set to the vehicle position or earlier.
         *                               If the first maneuver exceeds this then it's downtrack will be shifted to this value.
         * 
         * \param wm Pointer to intialized world model for semantic map access
         * 
         * \return List of centerline points paired with speed limits
         */ 
        std::vector<PointSpeedPair> maneuvers_to_points(const std::vector<cav_msgs::Maneuver>& maneuvers,
                                                                      double max_starting_downtrack,
                                                                      const carma_wm::WorldModelConstPtr& wm);
          /**
         * \brief Method converts a list of lanelet centerline points and current vehicle state into a usable list of trajectory points for trajectory planning
         * 
         * \param points The set of points that define the current lane the vehicle is in and are defined based on the request planning maneuvers. 
         *               These points must be in the same lane as the vehicle and must extend in front of it though it is fine if they also extend behind it. 
         * \param state The current state of the vehicle
         * 
         * \return A list of trajectory points to send to the carma planning stack
         */ 
        std::vector<cav_msgs::TrajectoryPlanPoint> compose_trajectory_from_centerline(
        const std::vector<PointSpeedPair>& points, const cav_msgs::VehicleState& state);

        /**
         * \brief Returns the nearest point to the provided vehicle pose in the provided list
         * 
         * \param points The points to evaluate
         * \param state The current vehicle state
         * 
         * \return index of nearest point in points
         */
        int getNearestPointIndex(const std::vector<PointSpeedPair>& points, const cav_msgs::VehicleState& state);

        /**
         * \brief Helper method to split a list of PointSpeedPair into separate point and speed lists 
         */ 
        void splitPointSpeedPairs(const std::vector<PointSpeedPair>& points, std::vector<lanelet::BasicPoint2d>* basic_points,
                            std::vector<double>* speeds);

        //wm pointer to the actual wm object
        carma_wm::WorldModelConstPtr wm_;

        double minimal_trajectory_duration_ = 6.0;
        double max_jerk_limit_ = 3.0;

    private:
        //CARMA ROS node handles
        std::shared_ptr<ros::CARMANodeHandle> nh_,pnh_;

        //ROS publishers and subscribers
        ros::Publisher plugin_discovery_pub_;
        ros::Subscriber pose_sub_;
        ros::Subscriber twist_sub_;

        /**
         * \brief Callback for the pose subscriber, which will store latest pose locally
         * \param msg Latest pose message
         */
        void pose_cb(const geometry_msgs::PoseStampedConstPtr& msg);
        /**
         * \brief Callback for the twist subscriber, which will store latest twist locally
         * \param msg Latest twist message
         */
        void twist_cb(const geometry_msgs::TwistStampedConstPtr& msg);
        
        PublishPluginDiscoveryCB plugin_discovery_publisher_;

        //Plugin discovery message
        cav_msgs::Plugin plugin_discovery_msg_;
        
    };
}