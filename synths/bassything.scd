// low-end, glitchy mids, stochasic, but with a somewhat rhythmic bass
// that low-end sounds decent actually
// stereo, full gain
// could be good to work with
SynthDef(\bassything, {|out, amp, freq=55, maxdelay=0.4, decaytime=2|
	var sig;
	sig = AllpassC.ar(SinOsc.ar(freq).tanh, maxdelay, TExpRand.ar(
			2e-4, 0.4, Impulse.ar(8)).round([2e-3, 4e-3]), decaytime);
	Out.ar(out, sig * amp);
}).add;

// ~bassything = {
// 	AllpassC.ar(
// 		SinOsc.ar(55).tanh, 0.4, TExpRand.ar(
// 	2e-4, 0.4, Impulse.ar(8)).round([2e-3,4e-3]),2)
// }.play;// #supercollider with bass please...
// )