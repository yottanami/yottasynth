# Yottasynth Documentation

This folder contains the project documentation for two different audiences:

1. people who want to use the instrument
2. people who want to understand or change the firmware

If you are new to the codebase, read the documents in this order:

1. [User Guide](user-guide.md)
2. [Source Reading Guide](source-reading-guide.md)
3. [Codebase Guide](codebase-guide.md)

## Document Map

### End-user documentation

- [User Guide](user-guide.md)
  Explains the screen, pages, knobs, touch controls, and normal playing workflow.

### Developer documentation

- [Source Reading Guide](source-reading-guide.md)
  Explains the C++ language features and embedded-programming patterns used by this firmware.

- [Codebase Guide](codebase-guide.md)
  Explains how the firmware is structured, how data moves through it, and what each active file is responsible for.

- [Synth V1 Plan](synth-v1-plan.md)
  Historical product/UI planning notes for the current panel concept.

## Scope Notes

This repository contains both:

- the current firmware implementation in `src/` and `include/`
- older prototype code in `lib/`

The active firmware entry point is [`src/main.cpp`](../src/main.cpp). The `lib/` directory is still useful for historical context, but it is not the main implementation path described by the current firmware docs.

## Repository Areas

- `src/`
  Main firmware implementation.

- `include/`
  Header files for the active firmware.

- `docs/`
  Project documentation.

- `etc/`
  Hardware photos, board design files, and related assets.

- `lib/`
  Legacy prototype code and earlier experiments.

- `test/`
  Placeholder test directory. There are currently no automated firmware tests in this repository.
