Since caf0393b0bff08593cfc21be71635ac6897c9dbe
- Make API more consistent

Since df71e16a33c6669717577e536ed508e043dea080
- Refactor modules into directories (rational, parse, execute)
- Rename destroy_* to free_*, and make them null out the parameter pointers

Since 7df89ba3ce9a5aeff56554126bd58f3d76ceceb5
- Change Expr representation to byte array
- Rename Alint to Alnat
- Introduce convenience macros for bools and error codes
- Fix memory leaks
- Refactor code structure
- Add tests