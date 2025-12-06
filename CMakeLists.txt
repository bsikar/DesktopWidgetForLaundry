cmake_minimum_required(VERSION 3.16)
project(DesktopWidgetForLaundry)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra)
endif()

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
            NotificationManager.h
            NotificationManager.mm
    )

    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist)
    set(APP_ICON ${CMAKE_SOURCE_DIR}/AppIcon.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(DesktopWidgetForLaundry MACOSX_BUNDLE ${SOURCES} ${APP_ICON})
    set_target_properties(DesktopWidgetForLaundry PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    )

    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(USERNOTIFICATIONS_LIBRARY UserNotifications REQUIRED)
    find_library(SERVICE_MANAGEMENT_LIBRARY ServiceManagement REQUIRED)
    target_link_libraries(DesktopWidgetForLaundry
            ${COCOA_LIBRARY}
            ${USERNOTIFICATIONS_LIBRARY}
            ${SERVICE_MANAGEMENT_LIBRARY}
    )
else()
    add_executable(DesktopWidgetForLaundry ${SOURCES})
endif()

target_link_libraries(DesktopWidgetForLaundry
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)
