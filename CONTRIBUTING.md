# Contributing to KGlance

Thanks for wanting to help. KGlance is small on purpose, so the best contributions are small too: a fixed bug, a rough edge smoothed, or a Plasma feature reused instead of rebuilt.

## Before you start

- **Bugs:** open an issue with your distro, Plasma version, and steps to reproduce. If it's about notifications, include the `notify-send` command (or the app) that triggers it.
- **Features:** open an issue first to talk it over. KGlance tries to stay a glance-and-go panel, so bigger features may be declined even when they're good ideas.

## Building and running

You need Qt 6, KDE Frameworks 6 (GlobalAccel, WindowSystem, Config), LayerShellQt, and the libdbus-1 development headers, on a Plasma 6 Wayland session.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

If KGlance is already installed, stop it first so the two copies don't both claim the global shortcut:

```
systemctl --user stop kglance
./build/bin/kglance
```

Press `Meta+\`` to open the panel. When you're done, run `systemctl --user start kglance` to go back to your installed copy.

## Testing notifications

Send a few with `notify-send` and check how the panel handles them:

```
notify-send -a Test "Short one" "Hello"
notify-send -a Test "Long one" "A body long enough to wrap across several lines and get cut at three"
ID=$(notify-send -p -a Player "Now playing" "Song 1")
notify-send -r "$ID" -a Player "Now playing" "Song 2"   # should replace the card, not add one
```

Changes to notification parsing should also survive broken input. `NotificationMonitor.cpp` reads data sent by any app on the session bus, so treat every field as untrusted.

## Code style

- Match the code around you: Qt/KDE naming, 4-space indents, `QStringLiteral` for literals.
- Prefer Qt, KDE Frameworks, or Plasma's own D-Bus interfaces over new dependencies. A new dependency needs a good reason.
- Only comment on the *why* when it isn't obvious from the code.

## Pull requests

- `main` is protected: open a pull request instead of pushing to it directly, and resolve review threads before merging.
- Keep each PR to one topic, and describe what changed and how you tested it. Screenshots help for anything visual.
- Add a line under `## [Unreleased]` in [CHANGELOG.md](CHANGELOG.md) for anything a user would notice.

## Releasing (maintainers)

1. Move the `[Unreleased]` entries in `CHANGELOG.md` under a new version heading.
2. Bump the version in `CMakeLists.txt`, `packaging/copr/kglance.spec` (`Version` plus a `%changelog` entry), `debian/changelog`, and the fallback in `packaging/aur/PKGBUILD`.
3. Merge to `main`, then tag and push:
   ```
   git tag -a vX.Y.Z -m "vX.Y.Z: short summary"
   git push origin vX.Y.Z
   ```
   Release tags can't be moved or deleted once pushed, so double-check the version first.
4. Pushing the tag builds on COPR (webhook) and uploads to the Launchpad PPA (`.github/workflows/ppa.yml`) automatically. Then create the GitHub release.

The PPA workflow fails early if the tag doesn't match the version in `debian/changelog`. To retry a failed upload, run **Upload to PPA** from the Actions tab with the existing tag.
