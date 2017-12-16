project(redis_monitor)

#**************************************************************************************************
# General cMake settings 
#**************************************************************************************************
cmake_minimum_required(VERSION 3.5)

#**************************************************************************************************
# Find Package **************************************************************************************************
find_package(OpenCV 3 REQUIRED) 
MESSAGE( " *** OpenCV_INCLUDE_DIRS : " ${OpenCV_INCLUDE_DIRS} ) 
MESSAGE( " *** OpenCV_LIB_DIRS : " ${OpenCV_LIB_DIRS} ) 
MESSAGE( " *** OpenCV_LIBS : " ${OpenCV_LIBS} )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/cxxopts)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/includes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/cpp_redis/tacopie/includes)

#**************************************************************************************************
# Include **************************************************************************************************
include_directories(${OpenCV_INCLUDE_DIRS}) 
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
# Linker **************************************************************************************************
LINK_DIRECTORIES(
  ${OpenCV_LIB_DIR}
)

#**************************************************************************************************
# Make configuration 
#**************************************************************************************************
add_executable(redis_monitor ${SOURCES})
target_link_libraries(redis_monitor -pthread ${OpenCV_LIBS} cxxopts cpp_redis)
install(TARGETS redis_monitor DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
