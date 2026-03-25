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


# Here we say where g++ should look for include files
set(SDK_HOME /opt/vertica/sdk CACHE FILEPATH "Path to the Vertica SDK, by default /opt/vertica/sdk")
if(NOT EXISTS ${SDK_HOME})
  message(FATAL_ERROR "Could not build. No SDK found at ${SDK_HOME} (maybe retry with -DSDK_HOME=<sdk_path>).")
endif()
set(VERTICA_INCLUDE ${SDK_HOME}/include)

## When building for Vertica 12, you need to set specific defines
file(READ "${VERTICA_INCLUDE}/BuildInfo.h" buildinfosdk)
string(REGEX MATCH "#define[ ]+VERTICA_BUILD_ID_SDK_Version_Major[ ]+([0-9]+)" _ ${buildinfosdk})
set(ver_major ${CMAKE_MATCH_1})
message("Version", ${ver_major})
if (${ver_major} LESS_EQUAL 12)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
  message("Using CXX11 ABI because Vertica <= 12:", ${ver_major})
else()
  message("Not using CXX11 ABI because newer SDK than Vertica 12", ${ver_major})
endif()

## For Vertica 24, you don't need it.

#############################
##  EXTERNAL DEPENDENCIES  ##
#############################

set(DATASKETCHES_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/datasketches-cpp"
    CACHE PATH "Path to local datasketches-cpp sources")

if(NOT EXISTS "${DATASKETCHES_SOURCE_DIR}/common/include")
  message(FATAL_ERROR
    "datasketches-cpp was not found at ${DATASKETCHES_SOURCE_DIR}. "
    "Put the sources into SOURCES/third_party/datasketches-cpp or pass "
    "-DDATASKETCHES_SOURCE_DIR=<path>.")
endif()

set(DATASKETCHES_INCLUDE
    ${DATASKETCHES_SOURCE_DIR}/common/include
    ${DATASKETCHES_SOURCE_DIR}/theta/include
)

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

  set_target_properties(vertica-datasketches PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS}")
  include_directories(${DATASKETCHES_INCLUDE})

  # Installation process just copies the binary to Vertica lib folder
  if (INSTALL_PREFIX_PATH)
    set(LIB_DIR ${INSTALL_PREFIX_PATH}/opt/vertica/lib)
  else()
    set(LIB_DIR /opt/vertica/lib)
  endif()

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
