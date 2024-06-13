# ietf-interfaces-yang-module plugin
cmake_minimum_required(VERSION 3.5)
project(ietf-interfaces-plugin C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# ietf-interfaces plugin
add_library(ietf-interfaces SHARED ietf-interfaces.c source/ethernet.c source/preemption.c source/preempt.c source/bridge_port.c source/sched.c source/stream_identification_statistics.c source/frer_statistics.c source/bridge_port_delays.c ${CMAKE_SOURCE_DIR}/../common/common.c ${CMAKE_SOURCE_DIR}/../common/bmp.c ${CMAKE_SOURCE_DIR}/../common/config_change.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(ietf-interfaces ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sysrepo/plugins/")

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(ietf-interfaces ${LIBBASE})

# dependencies - libtsn
find_library(LIBTSN tsn)
target_link_libraries(ietf-interfaces ${LIBTSN})

# dependencies - libbsd
find_library(LIBBSD bsd)
target_link_libraries(ietf-interfaces ${LIBBSD})

# dependencies - libyang
find_library(LIBYANG yang)
target_link_libraries(ietf-interfaces ${LIBYANG})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

# include source directory
include_directories(${CMAKE_SOURCE_DIR}/source)

target_compile_options(ietf-interfaces PRIVATE -Werror -Wall -Wextra)

install(TARGETS ietf-interfaces DESTINATION ${SR_PLUGINS_DIR})
