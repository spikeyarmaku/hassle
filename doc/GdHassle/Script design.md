parts of a script (gdscript 2):

- member variables
	- const
	- enum
	- var
- member methods
	- can be `static`
- signals
- classes

ideas:
- annotations (@onready, @export, etc.)
- `is` - tests if X extends a class, or is a builtin type
- `as` -  cast
- `tool` - Executes the script in the editor.
- breakpoint
- `preload`
- attribute reference (X.something)

```scheme
; a var
(define *speed* 1)
; a var with type
(define *speed* : Float 1)
; an exported var
(define *speed* 1)
(export *speed*)
; an exported var with type
(define *speed* : Float 1)
(export *speed*)

; a const
; ???
; a const with type
; ???

; an enum
(define speed (make-enum-type '(slow average fast)))

; a function
(define (_process delta) (print delta))
; a function with type
(: _process (-> Float Void))
(define (_process delta) (print delta))
; a static function
(define (static (class-count)) 0)
; a static function with type
(: class-count (-> Int Float))
(define (static (get-default-speed n)) (if (= n 0) *speed* (* *speed* 2)))

; a signal
(signal (connected x))
; a signal with a type
(signal (: (connected (: x int)) void))
```