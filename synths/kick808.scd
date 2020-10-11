(
SynthDef(\kick808, {
	arg out = 0, freq1 = 240, freq2 = 60, amp = 1, ringTime = 10, rel = 1, dist = 0.5, pan = 0;
	var snd, env;

	snd = Ringz.ar(
		in: Impulse.ar(0),
		freq: XLine.ar(freq1, freq2, 0.1),
		decaytime: ringTime
	);

	env = EnvGen.ar(Env.perc(0.001, rel, amp), doneAction: 2);
	snd = (1.0 - dist) * snd + (dist * (snd.distort));
	snd = snd * env;
	Out.ar(0, Pan2.ar(snd, pan));
}).add;
)


(
Pbind(
	\instrument, \kick808,
	\dur, 1/2,
	\amp, 0.8,
 	\rel, 0.4,
	\dist, 0.7
).play;
)

/*
create new environment

Effects
delay and echo filters

synths
 klank, klang, PMOsc
 GrainBuf
Envelopes

Mix down multiple ugens to mono or stereo

Controls
- control busses? Maybe to manipulate behaviour of several synths, set core frequency?
- could be done with groups as well
- maybe sensors that manipulate sound output based on where users are
- BeatTrack?

Streams, events

scope, plot, record
*/

(
f = {};
f.scope
)

{ PinkNoise.ar(0.2) + SinOsc.ar(440, 0, 0.2) + Saw.ar(660, 0.2) }.scope;
{ PinkNoise.ar(0.2) + Saw.ar(660, 0.2)}.scope;