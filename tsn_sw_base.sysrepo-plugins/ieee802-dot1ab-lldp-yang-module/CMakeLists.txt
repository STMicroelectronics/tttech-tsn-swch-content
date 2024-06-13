# ieee802-dot1ab-lldp-yang-module plugin
cmake_minimum_required(VERSION 3.5)
project(ieee802-dot1ab-lldp-plugin C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# ieee802-dot1ab-lldp plugin
add_library(ieee802-dot1ab-lldp-yang-module SHARED ieee802-dot1ab-lldp-yang-module.c ${CMAKE_SOURCE_DIR}/../common/common.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(ieee802-dot1ab-lldp-yang-module ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sysrepo/plugins/")

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(ieee802-dot1ab-lldp-yang-module ${LIBBASE})

# dependencies - liblldpctl
find_library(LIBLLDPCTL lldpctl)
target_link_libraries(ieee802-dot1ab-lldp-yang-module ${LIBLLDPCTL})

# dependencies - libyang
find_library(LIBYANG yang)
target_link_libraries(ieee802-dot1ab-lldp-yang-module ${LIBYANG})

# dependencies - libbsd
find_library(LIBBSD bsd)
target_link_libraries(ieee802-dot1ab-lldp-yang-module ${LIBBSD})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

target_compile_options(ieee802-dot1ab-lldp-yang-module PRIVATE -Werror -Wall -Wextra)

install(TARGETS ieee802-dot1ab-lldp-yang-module DESTINATION ${SR_PLUGINS_DIR})
