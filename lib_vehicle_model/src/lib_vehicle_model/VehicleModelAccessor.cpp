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

#include "lib_vehicle_model/VehicleModelAccessor.h"



/**
 * Cpp containing the implementation of VehicleModelAccessor
 */

namespace lib_vehicle_model {
  namespace VehicleModelAccessor {
    //
    // Private Namespace
    //
    namespace {
      std::mutex init_mutex;
      std::shared_ptr<ParameterServer> param_server_;

      // Parameters
      std::string vehicle_model_lib_path_;
      double max_forward_speed_;
      double max_reverse_speed_;
      double forward_acceleration_limit_;
      double forward_deceleration_limit_;
      double reverse_acceleration_limit_;
      double reverse_deceleration_limit_;
      double max_steering_angle_;
      double min_steering_angle_;
      double max_steering_angle_rate_;
      double max_trailer_angle_;
      double min_trailer_angle_;

      // Vehicle Model
      std::shared_ptr<VehicleMotionModel> vehicle_model_;
      bool modelLoaded_ = false;

      // The pointers to the loaded libraries create and destroy functions
      create_fnc_ptr create_fnc_;
      destroy_fnc_ptr destroy_fnc_;

      void loadModel() {
        // Load library from path
        void *lib_handle;
        lib_handle = dlopen(vehicle_model_lib_path_.c_str(), RTLD_NOW);

        // Check if load successfull
        if (!lib_handle)
        {
          std::string errorLog(dlerror());
          throw std::invalid_argument("Failed to open vehicle model shared library at " + vehicle_model_lib_path_ + " Reported Error: " + errorLog);
        }

        // Get pointers to the create and destroy functions
        create_fnc_ = (create_fnc_ptr)dlsym(lib_handle,"create");
        destroy_fnc_ = (destroy_fnc_ptr)dlsym(lib_handle,"destroy");

        // Check if create and destroy functions could be found
        if (!create_fnc_)
        {
          std::string errorLog(dlerror());
          throw std::invalid_argument("Failed to find pointer to vehicle model shared library create function Reported Error: " + errorLog);
        }

        if (!destroy_fnc_)
        {
          std::string errorLog(dlerror());
          throw std::invalid_argument("Failed to find pointer to vehicle model shared library destroy function Reported Error: " + errorLog);
        }

        // Set the vehicle model to the object returned by create_fnc
        // Pass in the destroy_fnc as the smart pointer deletor
        vehicle_model_.reset(create_fnc_(), destroy_fnc_);
      }

      void validateInitialState(const VehicleState& initial_state) {
        std::ostringstream msg;

        if (initial_state.steering_angle < min_steering_angle_) {
          msg << "Invalid initial_state with steering angle: " << initial_state.steering_angle << " is below min of: " << min_steering_angle_;
          throw std::invalid_argument(msg.str());
        }
        
        if (initial_state.steering_angle > max_steering_angle_) {
          msg << "Invalid initial_state with steering angle: " << initial_state.steering_angle << " is above max of: " << max_steering_angle_;
          throw std::invalid_argument(msg.str());
        }

        if (initial_state.trailer_angle < min_trailer_angle_) {
          msg << "Invalid initial_state with trailer angle: " << initial_state.trailer_angle << " is below min of: " << min_trailer_angle_;
          throw std::invalid_argument(msg.str());
        }

        if (initial_state.trailer_angle > max_trailer_angle_) {
          msg << "Invalid initial_state with trailer angle: " << initial_state.trailer_angle << " is above max of: " << max_trailer_angle_;
          throw std::invalid_argument(msg.str());
        }

      } 

      void validateControlInputs(const VehicleState& initial_state, const std::vector<VehicleModelControlInput>& control_inputs, double timestep) {

        // Last steering angle used to compute rate of steering angle change between control inputs
        double last_steer_angle = initial_state.steering_angle;

        size_t count = 0;
        std::ostringstream msg;
        // Validate each control input in sequence
        for (const VehicleModelControlInput& control : control_inputs) {
          if (control.target_acceleration < forward_deceleration_limit_) {
            msg << "Invalid control_input " << count << " with target_acceleration: " << control.target_acceleration << " is below min of: " << forward_deceleration_limit_;
            throw std::invalid_argument(msg.str());
          }

          if (control.target_acceleration > forward_acceleration_limit_) {
            msg << "Invalid control_input " << count << " with target_acceleration: " << control.target_acceleration << " is above max of: " << forward_acceleration_limit_;
            throw std::invalid_argument(msg.str());
          }

          if (control.target_steering_angle < min_steering_angle_) {
            msg << "Invalid control_input " << count << " with target_steering_angle: " << control.target_steering_angle << " is below min of: " << min_steering_angle_;
            throw std::invalid_argument(msg.str());
          }

          if (control.target_steering_angle > max_steering_angle_) {
            msg << "Invalid control_input " << count << " with target_steering_angle: " << control.target_steering_angle << " is above max of: " << max_steering_angle_;
            throw std::invalid_argument(msg.str());
          }

          const double delta_steer = control.target_steering_angle - last_steer_angle;
          const double steering_rate = abs(delta_steer / timestep);
          if (steering_rate > max_steering_angle_rate_) {
            msg << "Invalid control_input " << count << " with rate of steering change : " << steering_rate << " is above max of: " << max_steering_angle_rate_;
            throw std::invalid_argument(msg.str());
          }

          last_steer_angle = control.target_steering_angle;
          count++;
        }
      }
    }

    //
    // Public Namespace
    //

    void init(std::shared_ptr<ParameterServer> parameter_server) {
      // Mutex lock to ensure thread safety of lib loading and parameter loading
      // Since this function is the only place static data members are modified all other functions should be thread safe
      std::lock_guard<std::mutex> guard(init_mutex); 

      if (modelLoaded_) {
        throw std::runtime_error("Attempted to load the vehicle model a second time from the same process");
      }

      param_server_ = parameter_server;

      // Load Parameters
      // TODO remove reverse parameters from class diagram
      bool pathParam = param_server_->getParam("vehicle_model_lib_path", vehicle_model_lib_path_);
      bool speedParam = param_server_->getParam("max_forward_speed", max_forward_speed_);
      bool accelParam = param_server_->getParam("forward_acceleration_limit", forward_acceleration_limit_);
      bool decelParam = param_server_->getParam("forward_deceleration_limit", forward_deceleration_limit_);
      bool maxSteerParam = param_server_->getParam("max_steering_angle", max_steering_angle_);
      bool minSteerParam = param_server_->getParam("min_steering_angle", min_steering_angle_);
      bool maxSteerRate = param_server_->getParam("max_steering_angle_rate", max_steering_angle_rate_);
      bool maxTrailerAngle = param_server_->getParam("max_trailer_angle", max_trailer_angle_);
      bool minTrailerAngle = param_server_->getParam("min_trailer_angle", min_trailer_angle_);

      // Check if all the required parameters could be loaded
      if (!(pathParam && speedParam && accelParam && decelParam && maxSteerParam && minSteerParam && maxSteerRate && maxTrailerAngle && minTrailerAngle)) {
        std::ostringstream msg;
        msg << "One of the required parameters could not be found or read " 
          << " vehicle_model_lib_path: " << pathParam 
          << " max_forward_speed: " << speedParam 
          << " forward_acceleration_limit: " << accelParam 
          << " forward_deceleration_limit: " << decelParam 
          << " max_steering_angle: " << maxSteerParam 
          << " min_steering_angle: " << minSteerParam 
          << " max_steering_angle_rate: " << maxSteerRate 
          << " max_trailer_angle: " << maxTrailerAngle 
          << " min_trailer_angle: " << minTrailerAngle;

        throw std::invalid_argument(msg.str());
      }

      // Load the vehicle model to be used
      loadModel();
      vehicle_model_->setParameterServer(param_server_);

      // Mark that the model has been loaded
      modelLoaded_ = true;
    }

    std::vector<VehicleState> predict(VehicleState initial_state,
      double timestep, double delta_t) {
        
        if (!modelLoaded_) {
          throw std::runtime_error("Attempted to use VehicleModelAccessor before model was loaded with call to VehicleModelAccessor.init()");
        }

        // Validate inputs
        if (timestep > delta_t) {
          std::ostringstream msg;
          msg << "Invalid timestep: " << timestep << " is smaller than delta_t : " << delta_t;
          throw std::invalid_argument(msg.str());
        }
        
        validateInitialState(initial_state);
        // Pass request to loaded vehicle model
        return vehicle_model_->predict(initial_state, timestep, delta_t);
      }

    std::vector<VehicleState> predict(VehicleState initial_state,
      std::vector<VehicleModelControlInput> control_inputs, double timestep) {

        if (!modelLoaded_) {
          throw std::runtime_error("Attempted to use VehicleModelAccessor before model was loaded with call to VehicleModelAccessor.init()");
        }

        // Validate inputs
        validateInitialState(initial_state);
        validateControlInputs(initial_state, control_inputs, timestep);

        // Pass request to loaded vehicle model
        return vehicle_model_->predict(initial_state, control_inputs, timestep);
      }
  }
}
