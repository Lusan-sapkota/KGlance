<p align="center">
  <img src="data/icons/io.github.lusan-sapkota.KGlance.svg" width="128" alt="KGlance logo">
</p>

<h1 align="center">KGlance</h1>

<p align="center"><i>Because opening the system tray just to check what time it is in Tokyo felt like a personal insult.</i></p>

## What is this thing

KGlance is a tiny popup for KDE Plasma. Press a shortcut, it appears. Click somewhere else, or hit Escape, and it vanishes like it was never there. Inside: your local time, a world clock, a calendar you can flip through, and a live feed of your recent notifications with search and a Do Not Disturb switch.

It borrows literally everything it can from Plasma itself (colors, icons, the global shortcut system, the D-Bus session) so it doesn't have to reinvent a single wheel, and so it stays out of your RAM's way.

## Why does this exist

On KDE, checking the clock, the calendar, or your notification history usually means diving into the system tray and clicking around like you're defusing something. KGlance skips all of that. One shortcut, one glance, done. Hence the name. I'm very proud of it.

## Installing

**Debian / Ubuntu** (via PPA):

```
sudo add-apt-repository ppa:lusan/kglance
sudo apt update
sudo apt install kglance
```

**Fedora** (via COPR):

```
sudo dnf copr enable lusan/kglance
sudo dnf install kglance
```

**Arch**: AUR registration is temporarily closed, so no `kglance-git` package up there yet. You use Arch btw, so building from source (see below) shouldn't scare you. The `PKGBUILD` is already sitting in `packaging/aur/`, ready to publish the moment registration reopens.

## Features

* Global shortcut toggles the panel (default `Meta+\``, fully remappable from System Settings, Shortcuts, since it registers just like any other app shortcut)
* Local time and date
* World clock, reusing whatever cities you've already set up in Plasma's own Digital Clock widget, so you never have to configure them twice. Want a KGlance-only list instead? Drop one in `~/.config/kglance/worldclocks.json` and it takes over.
* A calendar with month navigation
* Notification history, built by quietly watching the session bus. Icons are pulled straight from each notification (favicons, avatars, whatever the app actually sent), falling back to a plain Plasma bell icon when nothing usable is available.
* Search, a real Do Not Disturb toggle (genuinely inhibits Plasma's notification popups while it's on, not just KGlance's own list), and a Clear All that also closes the real notifications in Plasma, not only KGlance's copy of them
* Vertically centers itself on whichever monitor Plasma considers active, so it shows up where your cursor actually is

## What this is not

It is not a notification daemon. Plasma's real one still receives every notification, renders every popup, and plays every sound, KGlance just watches the same traffic and asks nicely to close things or pause things through the same interfaces any well-behaved app would use. Think of it as a polite houseguest, just one who's also allowed to answer the door.

## How light is it, actually

Measured on a real Plasma 6 Wayland session, panel open or closed makes almost no difference:

| Metric | Value |
| --- | --- |
| RSS (total resident memory) | ~104 MB, mostly Qt6/KF6 pages Plasma already loaded for itself |
| PSS (fair-share of that memory) | ~60 MB |
| Private Dirty (memory that is genuinely KGlance's own) | ~17 MB |
| Compiled binary, stripped, Release build | ~147 KB |

RSS looks big at a glance, but almost all of it is shared library pages that were already resident before KGlance even started, since it deliberately reuses Qt6, KDE Frameworks, and LayerShellQt instead of bundling anything of its own. Private Dirty is the number that actually reflects KGlance's marginal cost to your system, and it barely moves whether the panel is open or has been sitting hidden for hours.

## Building it yourself

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/kglance
```

You'll need Qt6, KDE Frameworks 6 (GlobalAccel, WindowSystem, Config), LayerShellQt, and libdbus 1 development headers. If you're already running Plasma 6, chances are you have most of this lying around already.

## Requirements

* KDE Plasma 6 on Wayland
* A healthy tolerance for an app that eavesdrops on your own notifications, for entirely good reasons

## Contributing

Bug reports and pull requests are welcome, see [CONTRIBUTING.md](CONTRIBUTING.md). What changed in each release is in [CHANGELOG.md](CHANGELOG.md).

## License

MIT. Do whatever you want with it. If your world clock ends up set to the wrong Tokyo, that one's on you though.
