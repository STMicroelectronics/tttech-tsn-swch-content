# ieee802-dot1cb-stream-identification-params-yang-module plugin
cmake_minimum_required(VERSION 3.5)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# ieee802-dot1cb-stream-identification plugin
add_library(ieee802-dot1cb-stream-identification-params-yang-module SHARED ieee802-dot1cb-stream-identification-params-yang-module.c ${CMAKE_SOURCE_DIR}/../common/common.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(ieee802-dot1cb-stream-identification-params-yang-module ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR /usr/lib/sysrepo/plugins)

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(ieee802-dot1cb-stream-identification-params-yang-module ${LIBBASE})

# dependencies - libtsn
find_library(LIBTSN tsn)
target_link_libraries(ieee802-dot1cb-stream-identification-params-yang-module ${LIBTSN})

# dependencies - libbsd
find_library(LIBBSD bsd)
target_link_libraries(ieee802-dot1cb-stream-identification-params-yang-module ${LIBBSD})

# dependencies - libyang
find_library(LIBYANG yang)
target_link_libraries(ieee802-dot1cb-stream-identification-params-yang-module ${LIBYANG})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

target_compile_options(ieee802-dot1cb-stream-identification-params-yang-module PRIVATE -Werror -Wall -Wextra)

install(TARGETS ieee802-dot1cb-stream-identification-params-yang-module DESTINATION ${SR_PLUGINS_DIR})
