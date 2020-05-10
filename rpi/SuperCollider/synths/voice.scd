SynthDef(\voice, {
	arg out,
	midinote=31,
	amp = 1.0,
	pan=0.0,
	atk=10,
	rel=10;

	var sig, sig2, env;
	sig = Array.fill(3,{|i| VarSaw.ar(midinote.midicps*(i+1.0001), mul:0.05 / (i+1))}).sum;
	sig2 = Ringz.ar(WhiteNoise.ar(0.0003), TRand.ar(midinote.midicps, (midinote+1).midicps, Impulse.ar(10)));
	env = EnvGen.kr(Env.linen(atk, 1, rel), gate:1, doneAction:2);

	Out.ar(out, Pan2.ar((sig + sig2) * env * (0.8 + SinOsc.kr(0.1, 0, 0.2)), pan));
}).add;

/*
Tdef(\voices, {
	inf.do{
		10.do{
			if ((0.8).coin, {
				Synth(\voice, [
					\out, ~bus[\delayBus],
					\note:[24,28,29,48].choose,
					\pan:{rrand(-0.5,0.5)},
					\atk:{rrand(4,14)},
					\rel:{rrand(3,11)}]);
			});
			rrand(0.1,1).wait;
		};
		18.wait;
	};
});
*/