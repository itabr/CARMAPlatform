#!/bin/bash

#  Copyright (C) 2018-2019 LEIDOS.
# 
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not
#  use this file except in compliance with the License. You may obtain a copy of
#  the License at
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations under
#  the License.

# This scipt builds the CARMA platform and its Autoware.ai dependancies. 

usage() { 
  echo "USAGE carma_build.bash [OPTION]
  carma_build will build the CARMA Platform including any drivers in the same workspace as well as any required Autoware.ai components
  
  -a Path to Autoware.ai workspace. If this is not specified it is assumed CARMA and Autoware.ai share a workspace 
  -c Path to CARMA workspace.
  -x Skip Autoware.ai build. In this case Autoware.ai will be sourced from the location specified by -a
  -r CARMA rebuild flag. This will do a clean build of the CARMA code. Autoware.ai code is always cleaned before building even without this flag.
  -h Print help
  ";
}


# Default environment variables
carma_workspace="../.."
autoware_src="${carma_workspace}/src/autoware.ai"
skip_autoware=false
rebuild_carma=false
PACKAGE_SELECTION="lidar_localizer map_file deadreckoner points_downsampler lane_planner waypoint_maker"

# Read Options
while getopts a:c:xrh option
do
	case "${option}"
	in
		a) autoware_src=${OPTARG};;
		c) carma_workspace=${OPTARG};;
    x) skip_autoware=true;;
    r) rebuild_carma=true;;
    h) usage; exit 0;;
		\?) echo "Unknown option: -$OPTARG" >&2; exit 1;;
		:) echo "Missing option argument for -$OPTARG" >&2; exit 1;;
		*) echo "Unimplemented option: -$OPTARG" >&2; exit 1;;

	esac
done

# Clean workspace if needed
old_pwd="${PWD}"
cd ${carma_workspace}

if [ "${rebuild_carma}" = true ]; then
  echo "Clean CARMA build requested with -r option"
  echo "Claning carma workspace"
  rm -rf build devel install
fi

cd ${old_pwd}

# Build autoware or skip if requested
if [ "${skip_autoware}" = true ]; then
  echo "Skipping Autoware build due to -x option"
  source "${autoware_src}/ros/install/local_setup.bash"
else
  echo "Building Autoware required packages ${PACKAGE_SELECTION}"

  cd ${autoware_src}/ros
  ./colcon_release --packages-up-to "${PACKAGE_SELECTION}"
  source ./install/local_setup.bash
  echo "Autoware built successfuly. Binaries sourced from $(readlink -f ./install/local_setup.bash)"
fi

# Build CARMA 
echo "Building CARMA"
cd ${carma_workspace}
catkin_make install
echo echo "CARMA built successfuly. Binaries sourced from $(readlink -f ./devel/setup.bash)"
