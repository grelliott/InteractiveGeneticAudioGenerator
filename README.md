# audiogene

An interactive audio generator that evolves the sound based on interactions.

Composed of two primary components: a microcontroller connected to multiple interactive inputs, and a Raspberry Pi that generates audio, evolving it over time based on how users interact with the inputs.

## Build

### Raspberry Pi
The following dependencies should be installed first

**Note: This should be done on a Raspberry Pi**

`sudo apt-get install libgflags-dev libyaml-cpp-dev libspdlog-dev liblo-dev librtmidi-dev wiringpi cmake`

`cd gene`

`mkdir _install && mkdir _build && cd _build`

`cmake -DCMAKE_INSTALL_PREFIX=../_install ..`

`make`

`make install`

### Microcontroller
`cd avr`

`mkdir _build && cd _build`

`cmake ..`

`make`

