# CWriter

This is a single header library implementing the "writer" monad in C.


- Keep in mind.
! All logs wrapping a value must have a lifetime greater than or equal to that of the value they are wrapping.
