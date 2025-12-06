cmake_minimum_required(VERSION 3.16)
project(DesktopWidgetForLaundry)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set(SOURCES
        main.cpp
        LaundryTimer.cpp
        LaundryTimer.h
)

if(APPLE)
    enable_language(OBJCXX)
    list(APPEND SOURCES
            StatusItemBridge.h
            StatusItemBridge.mm
    )

    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist)
    add_executable(DesktopWidgetForLaundry MACOSX_BUNDLE ${SOURCES})
    set_target_properties(DesktopWidgetForLaundry PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    )

    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    target_link_libraries(DesktopWidgetForLaundry ${COCOA_LIBRARY})
else()
    add_executable(DesktopWidgetForLaundry ${SOURCES})
endif()

target_link_libraries(DesktopWidgetForLaundry
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)
