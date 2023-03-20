Since d51cf57865e88bc090202f8a7a21f05264d25b2d
- Fix memory leaks
- Add more debug information to memory operations

Since ab0af1a5c4fa5bfd5abec5beb0de87df73c24055
- Improve env frame handling
- Fix execution bugs
- Rename module `stdlib` to `builtin`
- Add built-ins

Since 0d9a9462f1ea886ba8696af0a237b53f79b7e14e
- Update readme

Since 66fa1ab0089257e9d1e3cb3ae3544d92f9048f5a
- Add license file

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