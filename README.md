# sbjson

Ultralightweight JSON parser in C99.

sbjson is a work in progress fork of [cJSON](https://github.com/DaveGamble/cJSON) with a growing number of changes:

- Improved API design.
- Adds 64-bit integer support.
- Case sensitive by default (spec conforming).
- Uses C99 over cJSON's C89.
- Formats code with Clang Format.
- Removes special casing platform/compiler preprocessor code. One simple codebase with one setting, so to speak.
- Alters preconditions
	- cJSON in general will check for multiple kinds of issues and return early if it detects any of them. This can hide issues.
	- SbJSON will respect preconditions by either assuming or asserting them to be met, which is made easier by the improved API design.
- Simple CMake build system, or alternatively you can copy paste sbjson.c and sbjson.h directly into your project.
- Bug fixes (heap buffer overflow).
- Much quicker development with an attention to details. Needed because cJSON is still unfortunately fairly flawed.

sbjson is not yet ready for usage due to it being in the middle of the API redesign.
