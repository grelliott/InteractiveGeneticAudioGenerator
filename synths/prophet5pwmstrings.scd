(
SynthDef(\prophet5pwmstrings, {
	arg out = 0, pan = 0.0, freq = 440, amp = 1.0, gate = 1, att = 0.01, rel = 0, sus = 1, dec = 0.5, lforate = 10, lfowidth = 0.5, cutoff = 12000, rq = 0.5;

	var lfo, pulse, filter, env;

	lfo = LFTri.kr(lforate * [1, 1.01], Rand(0, 2.0) ! 2);
	pulse = Pulse.ar(freq * [1, 1.01], lfo * lfowidth + 0.5);
	filter = RLPF.ar(pulse, cutoff, rq);
	env = EnvGen.ar(envelope: Env.adsr(att, dec, sus, rel, amp),
		gate: gate,
		doneAction: 2);
	Out.ar(out, Pan2.ar(
		in: Mix(filter) * env * 0.5,
		pos: pan));
}).add;
)
// (
// p = Pbind(
// 	\instrument, \prophet5pwmstrings,
// 	\midinote, Pseq([24, 36, 48, 36, 35, 36, 48], inf),
// 	\dur, Pstutter(8, Pseq([0.5, 0.25], inf))
// );
// p.play;
// )