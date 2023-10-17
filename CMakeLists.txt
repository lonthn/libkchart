cmake_minimum_required(VERSION 3.8)

project(kchart)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

if (DEFINED BUILD_EXAMPLE)
  unset(BUILD_EXAMPLE CACHE)
endif ()
option(BUILD_EXAMPLE "Enable to build examples(KLine & Timeshare)." ON)

include_directories(.)
include_directories(include/kchart)

add_subdirectory(src)

if(${BUILD_EXAMPLE})
  message(STATUS "Build examples")

  link_libraries(kchart)
  add_executable(example_dk
  		example/DayKLine.cpp)
  add_executable(example_ts
  		example/Timeshare.cpp)

  target_link_libraries(example_dk ws2_32)

  set(DST_PATH ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_TYPE})
  file(COPY example/SZ000001.csv     DESTINATION ${DST_PATH})
  file(COPY example/SH000001-min.csv DESTINATION ${DST_PATH})
endif(${BUILD_EXAMPLE})