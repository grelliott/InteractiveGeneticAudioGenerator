/*
 * IncFile1.h
 *
 * Created: 2018-12-29 10:25:05 PM
 *  Author: Grant
 */


#ifndef MIDI_H_
#define MIDI_H_

#include <stdint.h>
#include <stdlib.h>

// a struct to hold the note info
struct _midi {
	// track if the note is on or off
	uint8_t status;
	uint8_t note_number;
	uint8_t velocity;
};
typedef struct _midi midi_t;

/*!
 * uint8_t index A 0-based index of the input index
 * uint8_t on If 0, off, otherwise on.
 */
midi_t* Midi(uint8_t index, uint8_t on);

// eventually need a way to transmit the midi info via some protocol(s)


#endif /* MIDI_H_ */