project(redis_monitor)

#**************************************************************************************************
# General cMake settings 
#**************************************************************************************************
cmake_minimum_required(VERSION 3.5)

#**************************************************************************************************
# Find Package **************************************************************************************************

# cxxopt deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/cxxopts EXCLUDE_FROM_ALL)

# cpp_redis deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/tacopie/includes)

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

# json deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/json EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/json/src)

# inja deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/inja EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/inja/src)

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
SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -O3 -fPIC")

#**************************************************************************************************
# Make configuration 
#**************************************************************************************************
add_executable(redis_monitor ${SOURCES})
target_link_libraries(redis_monitor -pthread -lz -luv -lssl -lcrypt cxxopts cpp_redis ${UWEBSOCKET_LIBS})
add_dependencies(redis_monitor uwebsocket_lib)
install(TARGETS redis_monitor DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
