cmake_minimum_required(VERSION 3.1)
project(cadmium)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(cadmium INTERFACE)
target_include_directories(cadmium INTERFACE include/ json/include)

FILE(GLOB srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
foreach(srcSrc ${Examples})
    get_filename_component(srcName ${exampleSrc} NAME_WE)
    get_filename_component(dirname ${srcSrc} DIRECTORY)
    file(GLOB directorySrc CONFIGURE_DEPENDS "${dirname}/src/*.cpp")
    add_executable(${srcName} ${exampleSrc} ${directorySrc})
    target_include_directories(${srcName} PUBLIC "${dirname}/include")
    target_link_libraries(${srcName} cadmium)
endforeach(srcSrc)

