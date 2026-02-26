cmake_minimum_required(VERSION 4.1)
project(mdstats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(MODULE_STATSTABLE_VERSION v2)
add_subdirectory(module/statstable/${MODULE_STATSTABLE_VERSION})

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

set(MAIN_SRC src/main.cpp)

set(SRCS
    src/prog.cpp
    src/mainwindow.cpp
    src/databasemodel.cpp
    src/matcher.cpp
    src/delegate.cpp
    src/floatwindow.cpp
)

set(INCS
    include
    module/statstable
    module/toast_notify/binding
)

set(LIBS
    opencv_world4130
    stdc++exp
    Qt::Core
    Qt::Widgets
    modstatstable
    toast_notify
)
set(LIBDIRS
    lib
    module/toast_notify/target/release
)

qt_add_library(mainobj
    OBJECT
    ${MAIN_SRC}
)
target_include_directories(mainobj
    PRIVATE
    ${INCS}
)
target_link_libraries(mainobj
    PRIVATE
    ${LIBS}
)
target_link_directories(mainobj
    PRIVATE
    ${LIBDIRS}
)

qt_add_library(common
    OBJECT
    ${SRCS}
)
target_include_directories(common
    PRIVATE
    ${INCS}
)
target_link_libraries(common
    PRIVATE
    ${LIBS}
)
target_link_directories(common
    PRIVATE
    ${LIBDIRS}
)

add_subdirectory(src/perf)

# with console
qt_add_executable(${PROJECT_NAME}_with_console
    $<TARGET_OBJECTS:mainobj>
    $<TARGET_OBJECTS:common>
)
target_include_directories(${PROJECT_NAME}_with_console
    PRIVATE
    ${INCS}
)
target_link_libraries(${PROJECT_NAME}_with_console
    PRIVATE
    ${LIBS}
)
target_link_directories(${PROJECT_NAME}_with_console
    PRIVATE
    ${LIBDIRS}
)
install(TARGETS ${PROJECT_NAME}_with_console DESTINATION .)

if(WITHOUTCONSOLE)
    qt_add_executable(${PROJECT_NAME}_without_console
        WIN32
        $<TARGET_OBJECTS:mainobj>
        $<TARGET_OBJECTS:common>
    )
    target_include_directories(${PROJECT_NAME}_without_console
        PRIVATE
        ${INCS}
    )
    target_link_libraries(${PROJECT_NAME}_without_console
        PRIVATE
        ${LIBS}
    )
    target_link_directories(${PROJECT_NAME}_without_console
        PRIVATE
        ${LIBDIRS}
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
    module/toast_notify/target/release/toast_notify.dll
)
set(PROGFILES
    config.toml
    README.md
)
install(FILES ${DLLS} ${PROGFILES} DESTINATION .)
install(DIRECTORY resource DESTINATION . PATTERN resource/csv EXCLUDE)
