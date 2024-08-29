cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)

project(chordnamer)

add_library(${PROJECT_NAME}
    STATIC
        src/demo.cpp
        src/chord.cpp
        src/interval.cpp
        src/note.cpp
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

set(DEMO ${PROJECT_NAME}_Demo)
add_executable(${DEMO} src/demo.cpp)

target_include_directories(${DEMO}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${DEMO}
    PUBLIC
        ${PROJECT_NAME}
)
