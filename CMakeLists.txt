cmake_minimum_required(VERSION 3.8)

project(libkchart)

set(CMAKE_CXX_STANDARD 11)

IF(WIN32)
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
ENDIF(WIN32)

include_directories(.)

set(source
  ./KChartWnd.cpp
  ./Graphics.cpp
  ./Struct.cpp
  ./GraphArea.cpp
  ./graph/GdiPlusGC.cpp
  ./VerticalAxis.cpp
)


add_library(${PROJECT_NAME}
		${source})

link_libraries(gdiplus)
link_libraries(${PROJECT_NAME})

add_executable(example
		example/DayKLine.cpp
        #example/Timeshare.cpp)
		)