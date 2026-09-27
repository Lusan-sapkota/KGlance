cmake_minimum_required(VERSION 3.16)
project(kglance VERSION 0.1.3 LANGUAGES CXX)

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

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBus REQUIRED IMPORTED_TARGET dbus-1)

add_executable(kglance
    src/main.cpp
    src/GlanceWindow.cpp
    src/LocalTimeWidget.cpp
    src/WorldClockWidget.cpp
    src/CalendarWidget.cpp
    src/WorldClockConfig.cpp
    src/NotificationMonitor.cpp
    src/NotificationsPanel.cpp
)

target_link_libraries(kglance PRIVATE
    Qt6::Widgets
    Qt6::DBus
    KF6::GlobalAccel
    KF6::WindowSystem
    KF6::ConfigCore
    LayerShellQt::Interface
    PkgConfig::DBus
)

target_link_options(kglance PRIVATE $<$<CONFIG:Release>:-s>)

if(NOT KDE_INSTALL_SYSTEMDUSERUNITDIR)
    set(KDE_INSTALL_SYSTEMDUSERUNITDIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/user")
endif()

install(TARGETS kglance DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS data/kglance.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/kglance.service DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES data/icons/io.github.lusan-sapkota.KGlance.svg
    DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
