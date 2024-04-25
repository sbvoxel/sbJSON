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
- Removes broken locale handling
	- cJSON would try to call 'strtod' correctly by taking into account the current locale when locale support is enabled. But at the same time, it would also use that locale to print out JSON with the locale's decimal point, against the JSON standard which only uses a '.' decimal point. These issues are orthogonal and should not sit behind the same flag.
	- Until sbJSON replaces the usage of the stdlib's strtod function, it assumes the C locale to be set (as is the default).
- Replaces the build system options/files with a simple CMake one (WIP). I recommend copy pasting sbJSON.c and sbJSON.h into your project.

The API is diverging from cJSON.

For whatever reason, cJSON has multiple times been my go-to JSON library, across compiler and video game development. I'm happy to maintain a fork and try to elevate the library's quality even further over time.

For more documentation, we defer to [cJSON's](https://github.com/DaveGamble/cJSON) for now.
