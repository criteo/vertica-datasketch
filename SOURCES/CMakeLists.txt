cmake_minimum_required(VERSION 3.14)

# Make PROJECT_SOURCE_DIR, PROJECT_BINARY_DIR, and PROJECT_NAME available.
set(PROJECT_NAME vertica-datasketches)
project(${PROJECT_NAME})

#####################
##     OPTIONS     ##
#####################

option(VEC_REPORT "Display compilation vectorization reports." OFF)
option(DEBUG_BUILD "Attach debug information to the executables." OFF)
option(BUILD_VERTICA_LIB "Build the .so library with UDFs for Vertica" ON)
option(BUILD_VERTICA_TEST_DRIVER "Build a test program to show basic functionality of the underlying algorithm" OFF)
option(BUILD_TESTS "Build all tests." OFF)


#############################
##  EXTERNAL DEPENDENCIES  ##
#############################

include(ExternalProject)

# Set default ExternalProject root directory
# Using EP_BASE also indicates to cmake comprehenseible directory structure.
SET_DIRECTORY_PROPERTIES(PROPERTIES EP_BASE ${CMAKE_BINARY_DIR}/third_party)

ExternalProject_Add(
        datasketches
        # Using an URL as you would need a public key for GitHub otherwise.
        URL https://github.com/apache/incubator-datasketches-cpp/archive/bfcc54d598cd9f72186596002e9ee4883ebd8ce6.zip
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
ExternalProject_Get_Property(datasketches install_dir)
ExternalProject_Get_Property(datasketches source_dir)
# Need to include common header which aren't included even though they include *_impl.hpp
# headers which requires them.
set(DATASKETCHES_INCLUDE ${install_dir}/include/DataSketches ${source_dir}/common/include)
set(DATASKETCHES_LIB_DIR ${install_dir}/lib64)

#####################
##  COMPILE FLAGS  ##
#####################

set(CMAKE_CXX_FLAGS "-Wall -O3 -std=c++11 -Wno-unused-value")

if(DEBUG_BUILD)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

if(VEC_REPORT)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopt-info-vec-all=vecinfo.txt -ftree-vectorizer-verbose=7")
endif()

#####################
##  BUILD TARGETS  ##
#####################

if (BUILD_VERTICA_LIB)
  # Here we say where g++ should look for include files
  set(SDK_HOME /opt/vertica/sdk CACHE FILEPATH "Path to the Vertica SDK, by default /opt/vertica/sdk")
  if(NOT EXISTS ${SDK_HOME})
    message(FATAL_ERROR "Could not build. No SDK found at ${SDK_HOME} (maybe retry with -DSDK_HOME=<sdk_path>).")
  endif()

  set(VERTICA_INCLUDE ${SDK_HOME}/include)
  include_directories(${VERTICA_INCLUDE} include src)

  # datasketches SRC
  set(VERTICA_SRC ${VERTICA_INCLUDE}/Vertica.h ${VERTICA_INCLUDE}/BuildInfo.h ${VERTICA_INCLUDE}/Vertica.cpp)
  file(GLOB VERTICA_DATASKETCHES_SRC src/datasketches/**/* src/datasketches/*)

  add_library(vertica-datasketches SHARED ${VERTICA_DATASKETCHES_SRC} ${VERTICA_SRC})
  add_dependencies(vertica-datasketches datasketches)

  set_target_properties(vertica-datasketches PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
  include_directories(${DATASKETCHES_INCLUDE})
  # Installation process just copies the binary to Vertica lib folder
  set(LIB_DIR /opt/vertica/lib)
  install(
    TARGETS vertica-datasketches
    DESTINATION ${LIB_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
endif()

if (BUILD_VERTICA_TEST_DRIVER)
  add_executable(theta_driver tests/datasketches/theta_driver.cpp src/datasketches/custom_alloc.cpp)
endif()

add_custom_target(check COMMAND ctest -V)
