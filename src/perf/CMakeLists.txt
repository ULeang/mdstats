cmake_minimum_required(VERSION 4.1)
project(mdstats_perf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
    perf.cpp
)

add_executable(${PROJECT_NAME}
    ${SRCS}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../../include
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    stdc++exp
    common
)
target_link_directories(${PROJECT_NAME}
    PRIVATE
    ../../lib
    ../../module/toast_notify/target/release
)