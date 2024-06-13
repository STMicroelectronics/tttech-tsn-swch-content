# ieee802-dot1q-fqts-yang-module plugin
cmake_minimum_required(VERSION 3.5)
project(ieee802-dot1q-fqts-plugin C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

# ieee802-dot1q-fqtss plugin
add_library(ieee802-dot1q-fqtss SHARED ieee802-dot1q-fqtss.c ${CMAKE_SOURCE_DIR}/../common/common.c)

# dependencies - sysrepo
find_package(SYSREPO REQUIRED)
target_link_libraries(ieee802-dot1q-fqtss ${SYSREPO_LIBRARIES})
include_directories(${SYSREPO_INCLUDE_DIRS})

# set sysrepo plugins directory
set(SR_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/sysrepo/plugins/")

# dependencies - libbase
find_library(LIBBASE base)
target_link_libraries(ieee802-dot1q-fqtss ${LIBBASE})

# dependencies - libtsn
find_library(LIBTSN tsn)
target_link_libraries(ieee802-dot1q-fqtss ${LIBTSN})

# dependencies - libbsd
find_library(LIBBSD bsd)
target_link_libraries(ieee802-dot1q-fqtss ${LIBBSD})

# dependencies - libyang
find_library(LIBYANG yang)
target_link_libraries(ieee802-dot1q-fqtss ${LIBYANG})

# include common directory
include_directories(${CMAKE_SOURCE_DIR}/../common)

target_compile_options(ieee802-dot1q-fqtss PRIVATE -Werror -Wall -Wextra)

install(TARGETS ieee802-dot1q-fqtss DESTINATION ${SR_PLUGINS_DIR})
