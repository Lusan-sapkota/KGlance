# Changelog

All notable changes to KGlance are listed here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/).

## [0.1.3] - 2026-09-27

### Fixed
- Notifications that update in place (music players, download progress, chat apps) no longer pile up as duplicate cards; each one now shows once, with its latest content.
- A malformed notification image could make KGlance read past the end of the image data and crash. Image sizes are now checked against the bytes actually received, and images larger than 1024×1024 are ignored.
- Clear All could close the wrong notification when two apps' D-Bus message serials collided. Replies are now matched per sender.
- The internal table of pending notification replies no longer grows forever when Plasma never answers (for example after a plasmashell restart).

### Changed
- App names and summaries are shown as plain text. Bodies keep only bold, italic, underline and line breaks, so notifications can't embed images or links in the panel.
- Notification cards have their own rounded background, so they are easy to tell apart.
- Long bodies are cut to three lines. Hover a card to see the full text.
- Notification times are relative ("now", "5m ago") for the last hour, the clock time for earlier today, then "Yesterday" or the date. Hover the time for the exact timestamp.

## [0.1.2] - 2026-09-27

GitHub-only release; PPA and COPR went straight from 0.1.0 to 0.1.3.

### Fixed
- Notification cards were squashed to about 100px wide and cut off their text. Cards now use the full panel width and wrap long text.

## [0.1.1] - 2026-08-19

GitHub-only release.

### Fixed
- Fedora/COPR spec: the source tarball extracts to `KGlance-VERSION`, not `kglance-VERSION`.

## [0.1.0] - 2026-08-19

### Added
- First release: a shortcut-triggered panel with local time, a world clock, a calendar and notification history, including search, Do Not Disturb and Clear All.
- Debian, Fedora/COPR and AUR packaging.

[Unreleased]: https://github.com/Lusan-sapkota/KGlance/compare/v0.1.3...HEAD
[0.1.3]: https://github.com/Lusan-sapkota/KGlance/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/Lusan-sapkota/KGlance/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/Lusan-sapkota/KGlance/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/Lusan-sapkota/KGlance/releases/tag/v0.1.0
