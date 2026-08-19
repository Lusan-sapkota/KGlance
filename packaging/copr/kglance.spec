Name:           kglance
Version:        0.1.0
Release:        1%{?dist}
Summary:        Shortcut-triggered glance panel for KDE Plasma

License:        MIT
URL:            https://github.com/Lusan-sapkota/KGlance
Source0:        %{url}/archive/refs/tags/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  qt6-qtbase-devel
BuildRequires:  kf6-kglobalaccel-devel
BuildRequires:  kf6-kwindowsystem-devel
BuildRequires:  kf6-kconfig-devel
BuildRequires:  layer-shell-qt-devel

Requires:       qt6-qtbase
Requires:       kf6-kglobalaccel
Requires:       kf6-kwindowsystem
Requires:       kf6-kconfig
Requires:       layer-shell-qt

%description
KGlance is a small popup for KDE Plasma 6 on Wayland showing local time,
a world clock, a calendar, and a live notification history, triggered
by a global keyboard shortcut and dismissed on click-outside or Escape.
It reuses Plasma's own color scheme, icon theme, global shortcut system,
and Digital Clock world-clock configuration wherever possible instead
of duplicating settings.

%prep
%autosetup -n KGlance-%{version}

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%{_bindir}/kglance
%{_datadir}/applications/kglance.desktop
%{_datadir}/icons/hicolor/scalable/apps/io.github.lusan-sapkota.KGlance.svg
%{_prefix}/lib/systemd/user/kglance.service

%changelog
* Wed Aug 19 2026 Lusan Sapkota <sapkotalusan@gmail.com> - 0.1.0-1
- Initial release
