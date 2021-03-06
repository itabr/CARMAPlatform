| CicleCI Build Status | Sonar Code Quality |
|----------------------|---------------------|
[![CircleCI](https://circleci.com/gh/usdot-fhwa-stol/CARMAPlatform.svg?style=svg)](https://circleci.com/gh/usdot-fhwa-stol/CARMAPlatform) | [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=usdot-fhwa-stol_CARMAPlatform&metric=alert_status)](https://sonarcloud.io/dashboard?id=usdot-fhwa-stol_CARMAPlatform) |

# CARMA
![CARMA Arch](docs/image/CARMA_web_banner_18oct18_5.png)

CARMA<sup>SM</sup>  advances research and development to accelerate market readiness and deployment of cooperative driving automation, while advancing automated driving technology safety, security, data, and artificial intelligence. It encourages collaboration and participation by a community of engineers and researchers to advance understanding of cooperative driving automation using open source software (OSS) and agile project management practices. 

CARMA is a reusable, extensible platform for controlling SAE level 2 connected, automated vehicles (AVs). It provides a rich, generic API for third party plugins that implement vehicle guidance algorithms that plan vehicle trajectories. It is written in Java and C++ and runs in a Robot Operating System (ROS) environment. The platform can be reused on a variety of properly equipped vehicles.

This readme updated: 26 July 2019

## What Is CARMA
![CARMA Arch](docs/image/CARMA3_Architecture.png)

Managing automated vehicle motion involves three aspects. The first is **navigation, also known as localization**, which is the act of determining where the vehicle currently is with respect to the earth and with respect to the desired path of travel (its planned route). The second is **guidance, also known as trajectory planning**, which includes the processes of determining how the vehicle is to move from its current location to its destination. The destination and route will be handed to the guidance algorithms, and they then determine how the vehicle’s motion needs to be adjusted at any time in order to follow the route. The third aspect of automated vehicle motion is **control**, which covers the actuation of the vehicle’s physical devices to induce changes in motion (for land vehicles these are typically causing the wheels to rotate faster or slower and turning the steering wheel). Therefore, the navigation solution becomes an input to the guidance function, and the guidance solution becomes an input to the control function. As the vehicle moves, obviously its location changes so that the navigation function constantly needs to update its solution and the cycle iterates as quickly as necessary to produce a smooth and accurate vehicle motion. The rate of iteration is largely determined by the expected speed of the vehicle.

CARMA provides the navigation and guidance functions for its host vehicle, as well as some of the control functions.  It depends on low level controller hardware to provide the rest of the control function.  The current version of CARMA provides SAE level 2 autonomy, with both speed and steering control.

## Documentation

![CARMA Demo](docs/image/CARMA2_Platooning_from_Office.jpg)

<!-- We would like to publish this section, but are not ready to at this time:
## System Specifications
The current CARMA requirements specification: [CARMA Platform Requirements](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/56786945/Platform+Requirements+Document?preview=/56786945/56852481/CAV%20Platform%20Requirements.docx)
-->

## Release Notes
The current version and release history of the CARMA software platform: [CARMA Release Notes](<docs/Release_notes.md>)

**Repo Structure Note:**  The master and develop branches of this repo (and all other repos in the usdot-fhwa-stol GitHub organization) now reflects the third generation of CARMA code and documentation, called CARMA3.  It uses [Autoware](https://github.com/autowarefoundation/autoware) to provide SAE level 2 and 3 automation capability.  The legacy CARMA2 code is still being supported.  It was used extensively by FHWA during 2018 for SAE level 1 experiments.  To work with that code base, please check out the CARMA2-integration branch and make pull requests to it (using the contribution process outlined below).

## Roadmap
The current CARMA development direction and release plans. [CARMA Roadmap](<docs/Roadmap.md>)

## Architecture Guide
The documentation describes the software architecture within a single CARMA vehicle.  There may be several of these vehicles operating in concert, communicating with each other via DSRC or cellular means.  They can also communicate with roadside infrastructure using DSRC or cellular.  The communication among software components within a single vehicle, however, is mostly done via the Robot Operating System (ROS) framework.  There are no ROS communications between neighboring vehicles. [CARMA3 System Architecture](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/89587713/CARMA3+System+Architecture?preview=/89587713/128680006/CARMA%203.0%20Platform%20Architecture%20v0.docx)

For information on the CARMA2 architecture, please see [CARMA2 System Architecture](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/11403311/CARMA2+System+Architecture?preview=/11403311/130678837/CAV%20Platform%20Architecture.docx)

## Detailed Design Documents
Please see the Detail Design document page for detailed design specification for CARMA and other additional information.
[Detail Design Page](Detail_Design.md)

## Developers Guide 
A plug-in developers guide for CARMA3 will be coming soon.

For a description of the approach to develop a plug-in for CARMA2, please see [CARMA 2.7 Developers Guide](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/23330913/CARMA+Project+Documentation?preview=/23330913/29556796/CARMA%202.7%20Developers%20Guide.docx)

## Users Guide
A user guide for CARMA3 will be coming soon.

For a description of the functionality specific to CARMA2 on the Cadillac SRX, see [CARMA User Guide](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/23330913/CARMA+Project+Documentation?preview=/23330913/29392940/CARMA%202.7%20USER%20GUIDE.docx)

## Administrator Guide
An administrator guide for CARMA3 will be coming soon.

For administrative information on CARMA2, including vehicle and developer PC configuration, build, deployment and testing steps necessary to install and run the CARMA platform, please see [Administrator Guide](https://usdot-carma.atlassian.net/wiki/spaces/CAR/pages/23330913/CARMA+Project+Documentation?preview=/23330913/29196388/CARMA%20Administrator%20Guide.docx)

## Other CARMA Packages
CARMA Platform<sup>SM</sup> is a downloadable, open source software (OSS) platform architected to be extensible and reusable for a wide variety of research purposes to advance innovation for cooperative driving automation. It enables communication between vehicles, road users such as pedestrians, bicyclists, and scooters, and infrastructure devices capable of communication. It promotes collaboration between a community of engineers and researchers to accelerate the development, testing, and evaluation of cooperative driving automation while advancing the safety, security, data, and use of artificial intelligence in automated driving technology. 

The CARMA Platform is distributed as a set of multiple independent packages hosted in separate Github repositories. These packages facilitate operation of the CARMA platform with different hardware configurations or allow it to support different modes of operation. To include one of these packages in your build of the CARMAPlatform system please clone the Github repository into the same Catkin workspace `src/` folder as this repository. The Catkin build system will verify that dependencies are resolved appropriately and build the newly included package when you next run `catkin_make`. An incomplete listing of available packages for CARMA2 includes:

### Vehicle Controller Interface Drivers
* [CARMACadillacSrx2013ControllerDriver](https://github.com/usdot-fhwa-stol/CARMACadillacSrx2013ControllerDriver)
* [CARMAFreightliner2012ControllerDriver](https://github.com/usdot-fhwa-stol/CARMAFreightliner2012ControllerDriver)
* [CARMATorcXgvControllerDriver](https://github.com/usdot-fhwa-stol/CARMATorcXgvControllerDriver)

### Sensor Drivers
* [CARMATorcPinpointDriver](https://github.com/usdot-fhwa-stol/CARMATorcPinpointDriver)
* [CARMADelphiEsrDriver](https://github.com/usdot-fhwa-stol/CARMADelphiEsrDriver)
* [CARMACohdaDsrcDriver](https://github.com/usdot-fhwa-stol/CARMACohdaDsrcDriver)

### General System Utilites
* [CARMAWebUi](https://github.com/usdot-fhwa-stol/CARMAWebUi)
* [CARMAMsgs](https://github.com/usdot-fhwa-stol/CARMAMsgs)
* [CARMADriverUtils](https://github.com/usdot-fhwa-stol/CARMADriverUtils)

A full list of available packages may be found at in the [USDOT FHWA STOL](https://github.com/usdot-fhwa-stol) Github organization.

CARMA Cloud<sup>SM</sup> is a downloadable, cloud-based open source software (OSS) service that provides information to support and enable cooperative driving automation. It enables communication with cloud services and vehicles, road users such as pedestrians, bicyclists, and scooters, and infrastructure devices capable of communication. It promotes collaboration between a community of engineers and researchers to accelerate the development, testing, and evaluation of cooperative driving automation while advancing the safety, security, data, and use of artificial intelligence in automated driving technology.

## Contribution
Welcome to the CARMA contributing guide. Please read this guide to learn about our development process, how to propose pull requests and improvements, and how to build and test your changes to this project. [CARMA Contributing Guide](Contributing.md) 

## Code of Conduct 
Please read our [CARMA Code of Conduct](Code_of_Conduct.md) which outlines our expectations for participants within the CARMA community, as well as steps to reporting unacceptable behavior. We are committed to providing a welcoming and inspiring community for all and expect our code of conduct to be honored. Anyone who violates this code of conduct may be banned from the community.

## Attribution
The development team would like to acknowledge the people who have made direct contributions to the design and code in this repository. [CARMA Attribution](ATTRIBUTION.md) 

## License
By contributing to the Federal Highway Administration (FHWA) Connected Automated Research Mobility Applications (CARMA), you agree that your contributions will be licensed under its Apache License 2.0 license. [CARMA License](<docs/License.md>) 

## Contact
Please click on the CARMA logo below to visit the Federal Highway Adminstration(FHWA) CARMA website. For more information, contact CARMA@dot.gov.
[![CARMA Image](docs/image/CARMA_icon2.png)](https://highways.dot.gov/research/research-programs/operations/CARMA)
