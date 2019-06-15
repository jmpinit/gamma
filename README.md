# Gamma

The Beta is a simple 32 bit RISC processor design which was used in the class
6.004 Computation Structures at MIT to teach how computers work from the
transistor level up to operating systems. In the class students create their
own version of the Beta from gates, and write an operating system to run on a
reference simulator provided by the instructors. The simulator only included a
text terminal, so the graphics possibilities were limited. I wanted to make
games and graphics demos, so I started writing a simulator for a system based on
the Beta called Gamma.

The machine uses the [CGA graphics](https://en.wikipedia.org/wiki/Color_Graphics_Adapter)
color palette, but does not have the same restricted modes as that system.
The last block of memory is used for the graphics, so any changes there will be
visible on the Gamma's screen.

Lua is used for scripting the emulator, to make it easier to write tests.

## Building and Running

Make sure Lua5.1, SDL2, and SDL2_Image libraries are installed on your system.
Then run `make run` to have the emulator built and the basic graphics demo run.
