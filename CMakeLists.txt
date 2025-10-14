cmake_minimum_required(VERSION 4.1)
project(mdstats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    # WIN32
    MACOSX_BUNDLE
    src/main.cpp
    src/utils.cpp
    src/mainwindow.cpp
    src/databasemodel.cpp
    src/matcher.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    opencv_world4130
    stdc++exp
    Qt::Core
    Qt::Widgets
)
target_link_directories(${PROJECT_NAME}
    PRIVATE
    lib
)
# target_link_options(${PROJECT_NAME} PRIVATE
#     -static -static-libstdc++ -static-libgcc)

# include(GNUInstallDirs)

set(DLLS
    lib/libstdc++-6.dll
    lib/libgcc_s_seh-1.dll
    lib/libopencv_world4130.dll
)
set(PROGFILES
    config.toml
    README.md
)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(FILES ${DLLS} ${PROGFILES} DESTINATION .)
install(DIRECTORY resource DESTINATION . PATTERN resource/csv EXCLUDE)