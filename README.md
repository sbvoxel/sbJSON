# sbJSON

Ultralightweight JSON parser in C99. Fork of [cJSON](https://github.com/DaveGamble/cJSON).

sbJSON tracks cJSON development with the following changes:

- Adds 64-bit integer support
- Uses C99 over cJSON's C89
- Formats code with Clang Format
- Gets rid of archaic preprocessor platform/compiler handling
- Alters/respects preconditions (WIP)
	- cJSON is unclear about what the preconditions are and what's done about them. It tries to handle some preconditions by detecting them and returning an early NULL, without doing so for others (see parent==item checks, yet no general cycle checks).
	- sbJSON will respect preconditions by either assuming or asserting them to be met.
	- This is made slightly difficult because it's not clear to me yet what in cJSON is part of intentional API design, and what's misguided precondition handling.
- Removes locale handling
    - Ideally, we don't want to depend on strtod for parsing as its locale dependent, and locales are not thread safe. So while we depend on strtod, we require the locale to be set to 'C', which is the default.
- Replaces the build system options/files with a simple CMake one (WIP). I recommend copy pasting sbJSON.c and sbJSON.h into your project.

The API is diverging from cJSON.

For whatever reason, cJSON has multiple times been my go-to JSON library, across compiler and video game development. I'm happy to maintain a fork and try to elevate the library's quality even further over time.

For more documentation, we defer to [cJSON's](https://github.com/DaveGamble/cJSON) for now.
