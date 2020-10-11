// see about adjusting the timing of the rain drops and thunder
// probably send it through a mixing channel to adjust the volume and maybe reverb or delay
// figure out how to load/unload this and trigger it
SynthDef(\thunderstorm, { |out, amp|
	var sig, p;
	p = PinkNoise.ar(1!2);
	sig = BRF.ar(p + Blip.ar(p + 2, 400), 150, 2, 0.1) + LPF.ar(
		FreeVerb2.ar(*LPF.ar(p + 0.2 * Dust.ar(0.1) , 60)++[1,1,0.2,1e4]).tanh, 2000);
	Out.ar(out, sig * amp);
}).add;
