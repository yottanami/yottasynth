# Yottasynth

Yottasynth is an under-development digital synthesizer. The project started after
FOSDEM 2024, when I saw a digital synth demo and decided to build my own. It is an
open hardware project built around an ARM microcontroller, with custom hardware,
3D-printed parts, and ongoing firmware development.

![Yottasynth prototype](etc/yottasynth.jpg)

## Status

This is an active, work-in-progress journey to build a full synth. The touch
screen UI is in place and multiple states are implemented, but several parts are
still under development and will be added over time.

## Hardware

- Custom board design (my first PCB), source in `etc/`.
- Touch screen interface.
- Digital synth core running on an ARM microcontroller.

## 3D Design

I learned 3D modeling for this project and iterated across multiple tools. The
latest design files live in `etc/`. STL files will be uploaded later under
`etc/enclosure/`.

## Contributing

Contributions are welcome. Feel free to open an issue or a PR if you want to
help with firmware, hardware, or design.

## Issues

If you spot a bug or want to request a feature, please create an issue:
https://github.com/yottanami/yottainst/issues/new

## Repo Layout

- `src/`: firmware source code
- `include/`: headers
- `lib/`: libraries
- `test/`: tests
- `etc/`: hardware and 3D design sources

## Roadmap (Short)

- Expand synth engine features
- Complete remaining UI states
- Publish STL files

## Contact

https://yottanami.com

## Buy Me a Coffee

- GitHub Sponsors: https://github.com/sponsors/yottanami
- Ko-fi: https://ko-fi.com/yottanami
