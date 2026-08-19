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

## Features

* Global shortcut toggles the panel (default `Meta+\``, fully remappable from System Settings, Shortcuts, since it registers just like any other app shortcut)
* Local time and date
* World clock, configured in `~/.config/kglance/worldclocks.json`
* A calendar with month navigation
* Notification history, built by quietly watching the session bus. Plasma's real notification daemon still does all the actual work (popups, sounds, actions). KGlance just takes notes.
* Search, Clear All, and a Do Not Disturb toggle for that history
* Vertically centers itself on whichever monitor Plasma considers active, so it shows up where your cursor actually is

## What this is not

It is not a notification daemon. It does not intercept, silence, or replace your real notifications. Think of it as a polite houseguest who quietly writes things down instead of interrupting the conversation.

## How light is it, actually

Measured on a real Plasma 6 Wayland session, panel open or closed makes almost no difference:

| Metric | Value |
| --- | --- |
| RSS (total resident memory) | ~100 MB, mostly Qt6/KF6 pages Plasma already loaded for itself |
| PSS (fair-share of that memory) | ~58 MB |
| Private Dirty (memory that is genuinely KGlance's own) | ~17 MB |
| Compiled binary, stripped, Release build | ~119 KB |

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

## License

MIT. Do whatever you want with it. If your world clock ends up set to the wrong Tokyo, that one's on you though.
