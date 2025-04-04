cmake_minimum_required(VERSION 3.24) 
project(XyetaKakayaTo) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/*.cpp"
)

add_executable(XyetaKakayaTo ${SOURCE_FILES})