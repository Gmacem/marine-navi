# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


# -------- Options ----------

set(OCPN_TEST_REPO
    "opencpn/shipdriver-alpha"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "opencpn/shipdriver-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "opencpn/shipdriver-prod"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
#
# -------  Plugin setup --------
#
set(PKG_NAME ShipDriver_pi)
set(PKG_VERSION  3.3.4)
set(PKG_PRERELEASE "")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME ShipDriver)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME ShipDriver) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Simulate ship movements")
set(PKG_DESCRIPTION [=[
Simulates navigation of a vessel. Using the sail option and a current
grib file for wind data, simulates how a sailing vessel might react in
those conditions. Using 'Preferences' the simulator is able to record AIS
data from itself. This can be replayed to simulate collision situations.
]=])

set(PKG_AUTHOR "Mike Rossiter")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/Rasbats/shipdriver_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/shipdriver.html)

set(SRC
    src/cases/CheckPathCase.cpp
    src/cases/CheckPathCase.h
    src/cases/ForecastsLoader.cpp
    src/cases/ForecastsLoader.h

    src/clients/DbClient.cpp
    src/clients/DbClient.h
    src/clients/Esimo.cpp
    src/clients/Esimo.h
    src/clients/ForecastsProvider.h

    src/common/csv_parser.cpp
    src/common/csv_parser.h
    src/common/HttpClient.cpp
    src/common/HttpClient.h
    src/common/Utils.cpp
    src/common/Utils.h

    src/entities/DepthGrid.cpp
    src/entities/DepthGrid.h
    src/entities/Diagnostic.cpp
    src/entities/Diagnostic.h
    src/entities/Route.cpp
    src/entities/Route.h
    src/entities/Ship.h
    src/entities/WeatherForecast.h
    src/entities/WeatherGrid.h

    src/dialogs/MainNotebook.cpp
    src/dialogs/MainNotebook.h

    src/Dependencies.cpp
    src/Dependencies.h
    src/icons.cpp
    src/icons.h
    src/MarineNaviPi.cpp
    src/MarineNaviPi.h
    src/RenderOverlay.cpp
    src/RenderOverlay.h
)

include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

set(PKG_API_LIB api-18)  #  A dir in opencpn-libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/jsoncpp")
  target_link_libraries(${PACKAGE_NAME} ocpn::jsoncpp)

  # The wxsvg library enables SVG overall in the plugin
  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxsvg")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxsvg)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/sqlite")
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::sqlite)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/SQLiteCpp")
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::sqlite_cpp)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/curl")
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::libcurl)

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/wxcurl")
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::wxcurl)
endmacro ()
