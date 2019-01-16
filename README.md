# audiogene

An interactive audio generator that evolves the sound based on interactions.

Composed of two primary components: a microcontroller connected to multiple interactive inputs, and a Raspberry Pi that generates audio, evolving it over time based on how users interact with the inputs.

## Build

### Raspberry Pi
`cd rpi`
`mkdir _install && mkdir _build && cd _build`
`cmake -DCMAKE_INSTALL_PREFIX=../_install ..`
`make`

### Microcontroller
`cd avr`
`mkdir _build && cd _build`
`cmake ..`
`make`
