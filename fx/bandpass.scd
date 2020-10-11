// cutoff
SynthDef(\cutoff, {
    arg in, out,
	lpCutoff = 18000,  // low-pass hard cuttof
	lpGain = 0.0,
	hpCutoff = 60,  // high-pass hard cutoff
	hpGain = 0.0;

	var sig = In.ar(in, 2);
	sig = LPF.ar(sig, lpCutoff, lpGain.dbamp);
	sig = HPF(sig, hpCutoff, hpGain.dbamp);

    Out.ar(out, sig);
}).add;
