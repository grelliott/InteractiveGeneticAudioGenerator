// cutoff
SynthDef(\cutoff, {
    arg out,
	lpCutoff = 18000,  // low-pass hard cuttof
	hpCutoff = 60;  // high-pass hard cutoff

	var sig = In.ar(out, 2);
	sig = BLowPass.ar(sig, lpCutoff);
	sig = BHiPass.ar(sig, hpCutoff);
    ReplaceOut.ar(out, sig);
}).add;
