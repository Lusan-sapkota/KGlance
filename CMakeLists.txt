cmake_minimum_required(VERSION 3.16)
project(kglance VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(KF6_MIN_VERSION "6.0.0")
set(QT_MIN_VERSION "6.6.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Widgets DBus)
find_package(KF6GlobalAccel ${KF6_MIN_VERSION} REQUIRED)
find_package(KF6WindowSystem ${KF6_MIN_VERSION} REQUIRED)
find_package(KF6Config ${KF6_MIN_VERSION} REQUIRED)
find_package(LayerShellQt REQUIRED)

add_executable(kglance
    src/main.cpp
    src/GlanceWindow.cpp
    src/LocalTimeWidget.cpp
    src/WorldClockWidget.cpp
    src/CalendarWidget.cpp
    src/WorldClockConfig.cpp
)

target_link_libraries(kglance PRIVATE
    Qt6::Widgets
    Qt6::DBus
    KF6::GlobalAccel
    KF6::WindowSystem
    KF6::ConfigCore
    LayerShellQt::Interface
)

install(TARGETS kglance DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS data/kglance.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(PROGRAMS data/kglance-autostart.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
