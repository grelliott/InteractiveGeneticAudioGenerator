SynthDef(\schroederReverb, {
	arg in, out,
	predelay = 0.048,
	mix = 0.2,
	maxDelay = 0.1,
	decayTime = 15;

	var sig, z, y;
	sig = In.ar(in, 2);
	// reverb predelay time :
	z = DelayN.ar(sig, predelay);

	// 7 length modulated comb delays in parallel :
	y = Mix.ar(Array.fill(7,{ CombL.ar(z, maxDelay, LFNoise1.kr(0.1.rand, 0.04, 0.05), decayTime) }));

	// two parallel chains of 4 allpass delays (8 total) :
	4.do({ y = AllpassN.ar(y, 0.050, [0.050.rand, 0.050.rand], 1) });

	// add original sound to reverb and play it :
	Out.ar(out, sig+(mix*y));
}).add;
