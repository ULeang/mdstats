cmake_minimum_required(VERSION 4.1)
project(mdstats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
    launcher.cpp
)

add_executable(${PROJECT_NAME}
    WIN32
    ${SRCS}
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../../include
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    stdc++exp
)
target_link_directories(${PROJECT_NAME}
    PRIVATE
    ../../lib
)
install(TARGETS ${PROJECT_NAME} DESTINATION .)