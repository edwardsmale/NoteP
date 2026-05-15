cmake_minimum_required(VERSION 3.10)
project(NoteP)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base stc)
include(${wxWidgets_USE_FILE})

add_executable(NoteP WIN32
    src/main.cpp
    src/app.cpp
    src/mainframe.cpp
    src/findreplace.cpp
    src/config.cpp
    src/resource.rc
)

target_link_libraries(NoteP ${wxWidgets_LIBRARIES})
target_include_directories(NoteP PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(NoteP PRIVATE /W4)
else()
    target_compile_options(NoteP PRIVATE -Wall -Wextra -Wpedantic)
endif()
