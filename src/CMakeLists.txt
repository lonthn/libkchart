cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include/kchart)

set(source
  ./KChartWnd.cpp
  ./Graphics.cpp
  ./Struct.cpp
  ./GraphArea.cpp
  ./VerticalAxis.cpp
  ./HorizontalAxis.cpp
  ./CrosshairDelegate.cpp
  ./StrUtils.cpp
  ./graph/GdiPlusGC.cpp
)

add_library(${PROJECT_NAME} ${source})