cmake_minimum_required(VERSION 4.1)
project(modstatstable LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME}
    SHARED
    statstable.cpp
    parser_.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../../../include
    ../../../src
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    stdc++exp
)
target_link_directories(${PROJECT_NAME}
    PRIVATE
    ../../../lib
)
install(TARGETS ${PROJECT_NAME} DESTINATION .)

# target_link_options(${PROJECT_NAME} PRIVATE
#     -static -static-libstdc++ -static-libgcc)

# include(GNUInstallDirs)