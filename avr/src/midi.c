/*
 * midi.c
 *
 * Created: 2018-12-29 10:46:40 PM
 *  Author: Grant
 */

#include "midi.h"

midi_t* Midi(uint8_t index, uint8_t on) {
	midi_t* r = malloc(sizeof(midi_t));
	// Note on/off, Channel 0
	r->status = on ? 0x80: 0x90;
	// We'll only consider the 12 notes of C3 (60-71)
	r->note_number = index % 12 + 60;
	// Full velocity
	r->velocity = 0x7F;
}