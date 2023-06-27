To add a new language option in the "Create script" dialog, a new resource needs to be registered. Relevant PR: https://github.com/godotengine/godot/pull/59553

requirements:
- safe embedding (no IO)
- portable
- a type system that's:
	- pluggable (works without changing the runtime)
	- optional (can mix typed and untyped code)
	- static (checks before running the code)
- debugging and IDE / LSP capabilities

Some notes from Godot 2: https://godotengine.org/article/godot-getting-more-languages