project(redis-monitor)

#**************************************************************************************************
# General cMake settings
#**************************************************************************************************
cmake_minimum_required(VERSION 3.5)

#**************************************************************************************************
# Find Package **************************************************************************************************

# cxxopt deps
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cxxopts/include)

# json deps
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/json/single_include)

# inja deps
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/inja/src)

# catch deps
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/Catch2/single_include)

# cpp_redis deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/tacopie/includes)

# rocksdb deps
SET(ROCKSDB_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/modules/rocksdb/librocksdb.a)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/rocksdb/include)

add_custom_command(
  OUTPUT ${ROCKSDB_LIBS}
  COMMAND make ROCKSDB_LITE=ON static_lib
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/rocksdb
)

add_custom_target(
  rocksdb_lib
  DEPENDS ${ROCKSDB_LIBS}
)

# uwebsocket deps
SET(UWEBSOCKET_LIBS ${CMAKE_CURRENT_SOURCE_DIR}/modules/uWebSockets/libuWS.so)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/uWebSockets/src)

add_custom_command(
  OUTPUT ${UWEBSOCKET_LIBS}
  COMMAND make
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/uWebSockets
)

add_custom_target(
  uwebsocket_lib
  DEPENDS ${UWEBSOCKET_LIBS}
)

#**************************************************************************************************
# Include **************************************************************************************************
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)


#**************************************************************************************************
# Set variable **************************************************************************************************
SET(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)


#**************************************************************************************************
# Set compiler **************************************************************************************************
SET(CMAKE_CXX_FLAGS "-std=c++14 -Wall -O3 -fPIC")


#**************************************************************************************************
# Make configuration
#**************************************************************************************************
add_executable(redis-monitor ${SOURCES})
target_link_libraries(redis-monitor -pthread -lz -luv -lssl -lcrypt cpp_redis ${UWEBSOCKET_LIBS} ${ROCKSDB_LIBS})
add_dependencies(redis-monitor uwebsocket_lib rocksdb_lib)


#**************************************************************************************************
# Test configuration
#**************************************************************************************************
option(BUILD_UNIT_TESTS "Build the unit tests" ON)
if(BUILD_UNIT_TESTS)
  enable_testing()

  SET(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit-tic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit-database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit-monitor.cpp
  )

  add_executable(unit_test ${TEST_SOURCES})
  target_link_libraries(unit_test -pthread -lz -luv -lssl -lcrypt cpp_redis ${UWEBSOCKET_LIBS} ${ROCKSDB_LIBS})
  add_dependencies(unit_test uwebsocket_lib rocksdb_lib)

  add_test(NAME "unit_test_default"
    COMMAND unit_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
endif()