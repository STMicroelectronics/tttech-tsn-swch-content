# sysrepo-module-versions-yang-module plugin
cmake_minimum_required(VERSION 3.5)
project(sysrepo-module-versions-plugin C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# sysrepo-module-versions plugin
add_library(sysrepo-module-versions SHARED sysrepo-module-versions.c ${CMAKE_SOURCE_DIR}/../common/common.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(sysrepo-module-versions ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sysrepo/plugins/")

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(sysrepo-module-versions ${LIBBASE})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

target_compile_options(sysrepo-module-versions PRIVATE -Werror -Wall -Wextra)

install(TARGETS sysrepo-module-versions DESTINATION ${SR_PLUGINS_DIR})
