*(The documentation of the Kernel language can be used as reference - https://ftp.cs.wpi.edu/pub/techreports/pdf/05-07.pdf)*
# Specification principles
## P0 - The language should be easy to implement, but still usable by humans with tools
The language should contain the minimal amount of features to be useful (for some definition of the word), but enough to make it extendable by users. This should be achieved by having a very minimalistic core and a standard library to aid human usage.
The syntax should be very easy to parse while still being easy to read and write with structural editors. Because of this, S-Expressions are recommended as syntax.
## P1 - Any feature that can be implemented as a library should be moved to a library
The goal of the specification is to make the language easy to implement. Because of this, the specification calls for moving as many things to libraries as it is possible, to keep the core language simple. Performance is not a concern, however language implementors are free to ignore these recommendations and add more things to the core for the sake of the performance.