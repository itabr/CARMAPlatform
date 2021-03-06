/*
 * Copyright (C) 2018-2019 LEIDOS.
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

#pragma once

#include <string>
#include <ros/ros.h>
#include <atomic>
#include <carma_utils/CARMAUtils.h>
#include <cav_srvs/PluginList.h>
#include <cav_srvs/PluginActivation.h>
#include <cav_srvs/SetGuidanceActive.h>
#include <cav_srvs/SetEnableRobotic.h>
#include <cav_msgs/PluginList.h>
#include <std_msgs/Bool.h>
#include <cav_msgs/GuidanceState.h>
#include <cav_msgs/RobotEnabled.h>

namespace ui_integration
{
    class UIIntegrationWorker
    {
        public:
            /*!
             * \brief Default constructor for UIIntegrationWorker
             */
            UIIntegrationWorker();

            /*!
             * \brief Begin normal execution of UIIntegration worker. Will take over control flow of program and exit from here.
             * 
             * \return The exit status of this program
             */
            int run();
        protected:
            // Message/service callbacks
            bool registered_plugin_cb(cav_srvs::PluginListRequest& req, cav_srvs::PluginListResponse& res);
            bool active_plugin_cb(cav_srvs::PluginListRequest& req, cav_srvs::PluginListResponse& res);
            bool activate_plugin_cb(cav_srvs::PluginActivationRequest& req, cav_srvs::PluginActivationResponse& res);
            bool guidance_acivation_cb(cav_srvs::SetGuidanceActiveRequest& req, cav_srvs::SetGuidanceActiveResponse& res);
            void robot_status_cb(cav_msgs::RobotEnabled msg);

            // Helper functions
            void populate_plugin_list_response(cav_srvs::PluginListResponse& res);

            // Service servers 
            ros::ServiceServer registered_plugin_service_server_;
            ros::ServiceServer active_plugin_service_server_;
            ros::ServiceServer activate_plugin_service_server_;
            ros::ServiceServer guidance_activate_service_server_;

            // Publishers
            ros::Publisher plugin_publisher_;
            ros::Publisher state_publisher_;
            ros::ServiceClient enable_client_;

            // Subscribers
            ros::Subscriber robot_status_subscriber_;

            // Node handles
            ros::CARMANodeHandle nh_, pnh_;

            std::string plugin_name_;
            std::string plugin_version_;

            std::atomic<bool> guidance_activated_;
    };
}
