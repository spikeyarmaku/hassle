# Hassle
A programming language.
## General points
- Have a specification and a reference implementation.
- Should be minimal and simple to implement.
- Whatever can be moved to a standard lib should be moved there.
- ~~Opinionated. Enough of unassuming lisp variants, amplifying the lisp-curse.~~
- Parser: s-expressions.
- Runtime: untyped lambda calculus.
- Expression-oriented.
- Clearly separated and tracked mutation.
- Curry-by-default.

## Think about these points
- Purely functional, referentially transparent. How to solve IO?
- Optional strictness / laziness? Lenient-by-default? Does curry-by-default imply laziness?
- Gradual typing / gradual contracts / gradual correctness?
- Optional turing-incompleteness? Clearly separated and tracked non-termination?