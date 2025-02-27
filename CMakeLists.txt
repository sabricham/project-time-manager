cmake_minimum_required(VERSION 3.16)

set(HOMEKIT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/esp-homekit-sdk)

set(EXTRA_COMPONENT_DIRS ${HOMEKIT_PATH}/components ${HOMEKIT_PATH}/components/homekit ${HOMEKIT_PATH}/components/button ${HOMEKIT_PATH}/components/common)

list(APPEND EXTRA_COMPONENT_DIRS "lib")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(time-manager-device)
