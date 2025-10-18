cmake_minimum_required(VERSION 4.1)
project(mdstats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(SRCS
    src/main.cpp
    src/utils.cpp
    src/mainwindow.cpp
    src/databasemodel.cpp
    src/matcher.cpp
    src/delegate.cpp
)

# with console
qt_add_executable(${PROJECT_NAME}_with_console
    # WIN32
    MACOSX_BUNDLE
    ${SRCS}
)
target_include_directories(${PROJECT_NAME}_with_console
    PRIVATE
    include
)
target_link_libraries(${PROJECT_NAME}_with_console
    PRIVATE
    opencv_world4130
    stdc++exp
    Qt::Core
    Qt::Widgets
)
target_link_directories(${PROJECT_NAME}_with_console
    PRIVATE
    lib
)

if(WITHOUTCONSOLE)
    qt_add_executable(${PROJECT_NAME}_without_console
        WIN32
        MACOSX_BUNDLE
        ${SRCS}
    )
    target_include_directories(${PROJECT_NAME}_without_console
        PRIVATE
        include
    )
    target_link_libraries(${PROJECT_NAME}_without_console
        PRIVATE
        opencv_world4130
        stdc++exp
        Qt::Core
        Qt::Widgets
    )
    target_link_directories(${PROJECT_NAME}_without_console
        PRIVATE
        lib
    )
    install(TARGETS ${PROJECT_NAME}_without_console DESTINATION .)

    add_executable(${PROJECT_NAME}
        WIN32
        src/launcher/launcher.cpp
    )
    target_include_directories(${PROJECT_NAME}
        PRIVATE
        include
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        stdc++exp
    )
    target_link_directories(${PROJECT_NAME}
        PRIVATE
        lib
    )
    install(TARGETS ${PROJECT_NAME} DESTINATION .)

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
install(TARGETS ${PROJECT_NAME}_with_console DESTINATION .)
install(FILES ${DLLS} ${PROGFILES} DESTINATION .)
install(DIRECTORY resource DESTINATION . PATTERN resource/csv EXCLUDE)
