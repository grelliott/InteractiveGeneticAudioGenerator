// low-end (0hx -> 150hz primary, drops down ~250hz), sounds kind of like a symphonic bass, but with some weird creaking/popping sounds, interesting harmonics
// could be fun to play around with and stretch out
// it's quiet though, but is a stereo signal
// I like this :)
SynthDef(\bassogettato, {|out, amp, freq=165|
	var p, y, z, sig;
	p = Trig.ar(Saw.ar(freq),1);
	y = SinOsc.ar(p*freq);
	z = SinOsc.ar(p);
	sig = (GVerb.ar(GrainIn.ar(2,y,y/2,z,p*z,-1),9))/9;
	Out.ar(out, sig * amp);
}).add;

// ~bassogettato = {
// 	x=165;
// 	b=SinOsc;
// 	p=Trig.ar(Saw.ar(x),1);
// 	y=b.ar(p*x);
// 	z=b.ar(p);
// 	(GVerb.ar(GrainIn.ar(2,y,y/2,z,p*z,-1),9))/9
// }.asSynthDef.add //basso gettato #SuperCollide
// )
