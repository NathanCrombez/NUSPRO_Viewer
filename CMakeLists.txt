project(NUSPRO_Viewer)
cmake_minimum_required(VERSION 2.6)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS filesystem)
find_package(OpenCV REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

SET(SOURCES
main.cpp
)

add_executable (
  ${PROJECT_NAME} ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES}  -lboost_system -lopencv_nonfree)

