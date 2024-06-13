# ieee8021-mstp-yang-module plugin
cmake_minimum_required(VERSION 3.5)
project(iee8021-mstp-plugin C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# ieee8021-mstp plugin
add_library(ieee8021-mstp SHARED ieee8021-mstp.c ${CMAKE_SOURCE_DIR}/../common/common.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(ieee8021-mstp ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sysrepo/plugins/")

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(ieee8021-mstp ${LIBBASE})

# dependencies - libbsd
find_library(LIBBSD bsd)
target_link_libraries(ieee8021-mstp ${LIBBSD})

# dependencies - libyang
find_library(LIBYANG yang)
target_link_libraries(ieee8021-mstp ${LIBYANG})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

target_compile_options(ieee8021-mstp PRIVATE -Werror -Wall -Wextra)

install(TARGETS ieee8021-mstp DESTINATION ${SR_PLUGINS_DIR})
