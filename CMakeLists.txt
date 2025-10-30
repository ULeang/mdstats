cmake_minimum_required(VERSION 4.1)
project(mdstats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

add_subdirectory(module/statstable/v2)

set(MAIN_SRC src/main.cpp)

set(SRCS
    src/prog.cpp
    src/mainwindow.cpp
    src/databasemodel.cpp
    src/matcher.cpp
    src/delegate.cpp
)

qt_add_library(common
    STATIC
    ${SRCS}
)
target_include_directories(common
    PRIVATE
    include
)
target_link_libraries(common
    PRIVATE
    opencv_world4130
    stdc++exp
    Qt::Core
    Qt::Widgets
    modstatstable
)
target_link_directories(common
    PRIVATE
    lib
)

# with console
qt_add_executable(${PROJECT_NAME}_with_console
    # WIN32
    ${MAIN_SRC}
)
target_include_directories(${PROJECT_NAME}_with_console
    PRIVATE
    include
)
target_link_libraries(${PROJECT_NAME}_with_console
    PRIVATE
    common
    opencv_world4130
    stdc++exp
    Qt::Core
    Qt::Widgets
    modstatstable
)
target_link_directories(${PROJECT_NAME}_with_console
    PRIVATE
    lib
)
install(TARGETS ${PROJECT_NAME}_with_console DESTINATION .)

if(WITHOUTCONSOLE)
    qt_add_executable(${PROJECT_NAME}_without_console
        WIN32
        ${MAIN_SRC}
        # $<TARGET_OBJECTS:${PROJECT_NAME}>
    )
    target_include_directories(${PROJECT_NAME}_without_console
        PRIVATE
        include
    )
    target_link_libraries(${PROJECT_NAME}_without_console
        PRIVATE
        common
        opencv_world4130
        stdc++exp
        Qt::Core
        Qt::Widgets
        modstatstable
    )
    target_link_directories(${PROJECT_NAME}_without_console
        PRIVATE
        lib
    )
    install(TARGETS ${PROJECT_NAME}_without_console DESTINATION .)

    add_subdirectory(src/launcher)

endif()

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
install(FILES ${DLLS} ${PROGFILES} DESTINATION .)
install(DIRECTORY resource DESTINATION . PATTERN resource/csv EXCLUDE)
