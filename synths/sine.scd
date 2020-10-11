SynthDef(\sine_a, {
    arg out=~out,
    freq=440,
    amp=1,
    atk=0.01, dec=1, sus=0.1, rel=0.1,
    gate=1;
    var sig, env;
    sig = SinOsc.ar(freq, 0);
    env = EnvGen.kr(Env.adsr(atk, rel), gate, doneAction:2);
    sig = sig * env * amp;
    Out.ar(out, sig);
}).add;

