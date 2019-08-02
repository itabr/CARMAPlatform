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


# Build the software and its dependencies

set -ex

source /opt/autoware.ai/setup.bash
cd ~/carma_ws
rosdep update
rosdep install --from-paths src --ignore-src -y
./carma_build -c ~/carma_ws -a /opt/autoware.ai/ -x


# Copy the installed files
cd ~/carma_ws 
cp -r install/. /opt/carma/app/bin/ 
chmod -R +x /opt/carma/app/bin 
cp -r src/CARMAPlatform/carmajava/launch/params/* /opt/carma/params/
cp -r src/CARMAPlatform/carmajava/launch/*.launch /opt/carma/launch/
ln -s  /opt/carma/launch/* /opt/carma/app/bin/share/carma
cp -r src/CARMAPlatform/engineering_tools/* /opt/carma/app/engineering_tools/
cp -r src/CARMAPlatform/engineering_tools /opt/carma/app/bin/share
cp -r src/CARMAPlatform/carmajava/mock_drivers/src/test/data/. /opt/carma/app/mock_data
