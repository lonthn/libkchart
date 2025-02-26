cmake_minimum_required(VERSION 3.8)

set(PROJ_NAME kchart)
project(${PROJ_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
add_definitions(-D_WIN32)

message(${CMAKE_EXE_LINKER_FLAGS})

if (DEFINED BUILD_EXAMPLE)
  unset(BUILD_EXAMPLE CACHE)
endif ()
option(BUILD_EXAMPLE "Enable to build examples(KLine & Timeshare)." ON)

add_library(${PROJ_NAME} STATIC kchart.cpp)


if(${BUILD_EXAMPLE})
  message(STATUS "Build examples")

  link_libraries(${PROJ_NAME})
  add_executable(example_dk
  		example/DayKLine.cpp)
  add_executable(example_ts
  		example/Timeshare.cpp)

  target_link_libraries(example_dk ws2_32)

  set(DST_PATH ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_TYPE})
  file(COPY example/SZ000001.csv     DESTINATION ${DST_PATH})
  file(COPY example/SH000001-min.csv DESTINATION ${DST_PATH})
endif(${BUILD_EXAMPLE})