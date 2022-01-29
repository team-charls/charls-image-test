# Comments on disabled C++ Core Guidelines Rules

C26429: Use a not_null to indicate that "null" is not a valid value
-> Rationale: Prefast attributes are better.

C26446: Prefer to use gsl::at() instead of unchecked subscript operator.
-> Rationale: gsl:at() cannot be used. debug STL already checks.


C26472: Don't use static_cast for arithmetic conversions
-> Rationale: can only be solved with gsl::narrow_cast

C26474: No implicit cast
-> Rationale: many false warnings

C26481: Don't use pointer arithmetic. Use span instead (bounds.1)
-> Rationale: many false warnings.

C26482:Only index into arrays using constant expressions.
-> Rationale: static analysis can verify access.

C26486: Lifetime problem.
-> Rationale: many false warnings.

C26489: Don't dereference a pointer that may be invalid
-> Rationale: many false warnings

C26490: Don't use reinterpret_cast
-> Rationale: required to work with win32 API

C26494: Variable 'x' is uninitialized. Always initialize an object
-> Rationale: many false warnings, other analyzers are better.

C26821: Consider using gsl::span instead of std::span to guarantee runtime bounds safety (gsl.view).
-> Rationale: MSVC std:span() will check runtime bounds in debug mode: use std::span.
