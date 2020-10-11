SynthDef(\apad_mh, {
	arg
	//Standard Values:
	out = 0, pan = 0, freq = 880, amp = 0.5, att = 0.4, dec = 0.5, sus = 0.8, rel = 1.0, gate = 1,
	//Other Controls:
	vibratoRate = 4, vibratoDepth = 0.015, tremoloRate = 5,
	//These controls go from 0 to 1:
	tremoloDepth = 0.5, reverbMix = 0.5, roomSize = 1, damp = 0.5;

	var env, snd, vibrato, tremolo, mod2, mod3;

	env = Env.adsr(att, dec, sus, rel).kr(gate: gate);
	vibrato = SinOsc.kr(vibratoRate).range(freq * (1 - vibratoDepth), freq * (1 + vibratoDepth));
	tremolo = LFNoise2.kr(1).range(0.01, 1) * SinOsc.kr(tremoloRate).range((1 - tremoloDepth), 1);

	snd = FSinOsc.ar(freq: [freq, vibrato], mul:(env * tremolo * amp)).distort;
	snd = Mix.ar(snd);
	snd = FreeVerb.ar(snd, reverbMix, roomSize, damp);
	// snd = ;
	DetectSilence.ar(snd, 0.0001, 0.2, doneAction: 2);
	Out.ar(out, Splay.ar(snd, 0.8));
}).add;