Instructions for building dc42 fork of RepRapFirmware
=====================================================

**Important!**

RepRapFirmware is built from several Github projects. You need to use compatible branches of these projects. As at 08 January 2020, the latest RRF 2.x source code is on these branches:

- RepRapFirmware: master
- CoreNG: master
- FreeRTOS: master
- RRFLibraries: master
- DuetWiFiSocketServer: master

As at 30 August 2020, the latest RRF 3.2beta source code is on these branches:

- RepRapFirmware: v3.02-dev
- CoreNG: dev
- FreeRTOS: dev
- RRFLibraries: dev
- DuetWiFiSocketServer: dev (provides an include file to RepRapFirmware; does not need to be built unless you want to build DuetWiFiServer)
- CoreESP8266: dev (only needed if you want to build DuetWiFiServer)
- LwipESP8266: master (only needed if you want to build DuetWiFiServer)

To build the 3.1.1 stable release, pick the commit of each project labelled 3.1.1 if it exists, otherwise the commit labelled 3.1.0.

**Instructions for building under Windows**

1. Download and install the gcc cross-compiler from https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads:
- To build firmware version 3.1.x use version 2019-q4-major
- To build firmware version 2.05.1 use 2019-q4-major

You should be able to use a later version of the compiler than the one specified, but you may get warning messages if you do.

2. Download and install Eclipse IDE for C/C++ Developers, from http://www.eclipse.org/downloads/eclipse-packages/. You do not need the Arduino add-on.

3. Download and install GNU Arm Eclipse from https://sourceforge.net/projects/gnuarmeclipse/files/Build%20Tools/gnuarmeclipse-build-tools-win64-2.6-201507152002-setup.exe/download. This provides versions of make.exe, rm.exe and other tools without the 8192-character command line limitation of some other versions.

4. In Eclipse create new workspace C:/Eclipse/Firmware. In Window->Preferences->C/C++->Build->Build Variables, create variable ArmGccPath and set it to the folder containing the GCC compiler binaries, for example C:\Program Files (x86)\GNU Tools ARM Embedded\9 2019-q4-major\bin. Then exit Eclipse.

5. Download this github project as a zip file and unzip it into C:/Eclipse/Firmware. Then rename folder ReprapFirmware-dev in that folder to RepRapFirmware.

6. Repeat the previous step for github project CoreNG. The folder name should be left as CoreNG (or renamed from CoreNG-dev to CoreNG if you downloaded a dev build).

7. If you want to build firmware for Duet WiFi then you also need to download and add project DuetWiFiSocketServer. Alternatively, just download file src/include/MessageFormats.h from that project and put it somewhere on the include path for RepRapFirmware.

8. Also download projects FreeRTOS and RRFLibraries from my github repo and add those projects to the workspace.

9. If you want to build firmware for Duet 3, also download and project CANLib from my github repo and add that project to the workspace.

10. Load Eclipse and tell it to import the CoreNG and RepRapFirmware projects, also FreeRTOS, DuetWiFiSocketServer and RRFLibraries if you have included them.

11. The build depends on the Eclipse workspace variable 'ArmGccPath" being set to the directory where your arm-none-eabi-g++ compiler resides. For example "C:\Program Files (x86)\GNU Tools ARM Embedded\7 2018-q2-update\bin" on Windows. To set it, go to Windows -> Preferences -> C/C++ -> Build -> Build Variables and click "Add..."

12. Build CoreNG first, also build FreeRTOS, RRFLibraries and CANlib if needed. Then clean and build RepRapFirmware. The Duet WiFi and Duet Ethernet builds of RRF use the SAM4E_RTOS builds of CoreNG and RRFLibraries and the SAM4E build of FreeRTOS. The Duet Maestro uses the SAM4S_RTOS build of CoreNG and RRFLibraries, and the SAM4S build of FreeRTOS. The Duet085 build of RRF (which also runs on the Duet06) uses the SAM3X build of CoreNG and RRFLibraries. The RADDS build of RRF uses the RADDS_RTOS build of CoreNG and the SAM3X_RTOS build of RRFLibraries.

Note: you do not need to build the DuetWiFiSocketServer project, but it does need to be in the workspace because the RepRapFirmware project uses one of its include fies.

**Instructions for building under macOS**

Using Homebrew-Cask makes it very easy to install new software on macOS: https://caskroom.github.io/

1. Download and install the gcc-arm-embedded: brew cask install gcc-arm-embedded

3. Download and install Eclipse for C++ : brew cask install eclipse-cpp

4. Download or clone the RepRapFirmware, CoreNG, FreeRTOS, RRFLibraries and DuetWiFiSocketServer projects into your workspace (also CANlib if you are building firmware for Duet 3). Keep the folder names as is.

5. Open Eclipse and import RepRapFirmware, FreeRTOS, RRFLibraries and CoreNG projects.

6. The build depends on the Eclipse workspace variable 'ArmGccPath" being set to the directory where your arm-none-eabi-g++ compiler resides. To set it, go to Windows -> Preferences -> C/C++ -> Build -> Build Variables and click "Add..."

7. Build CoreNG, FreeRTOS and RRFLibraries first, then RepRapFirmware. See the instructions for Windows (above) for the configurations needed.

**Building under Debian Linux**

See this forum post https://forum.duet3d.com/topic/11703/building-reprap-firmware-on-debian-buster

