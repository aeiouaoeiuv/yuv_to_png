cmake_minimum_required(VERSION 3.15)

project(yuv2png)

find_package(OpenCV REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCES)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

link_directories(

)

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_OPTIONS -s) # strip

target_link_libraries(${PROJECT_NAME}
    pthread
    ${OpenCV_LIBS}
)
