# sbJSON

Ultralightweight JSON parser in C99. Fork of [cJSON](https://github.com/DaveGamble/cJSON).

sbJSON tracks cJSON development with the following changes:

- Adds 64-bit integer support
- Always case sensitive
- Uses C99 over cJSON's C89
- Formats code with Clang Format
- Removes special casing platform/compiler preprocessor code. One simple codebase with one setting, so to speak.
- Alters preconditions
	- cJSON in general will check for multiple kinds of issues and return early if it detects any of them. This can hide issues.
	- sbJSON will respect preconditions by either assuming or asserting them to be met.
- Removes locale handling
    - Ideally, we don't want to depend on strtod for parsing as its locale dependent, and locales are not thread safe. But while we depend on strtod, we require the locale to be set to 'C', which is the default.
- Replaces the build system files with a simple CMake one. You can also copy paste sbJSON.c and sbJSON.h directly into your project.

The API is diverging from cJSON.

For whatever reason, cJSON has multiple times been my go-to JSON library, across compiler and video game development. I'm happy to maintain a fork and try to elevate the library's quality even further over time.

For more documentation, we defer to [cJSON's](https://github.com/DaveGamble/cJSON) for now.
