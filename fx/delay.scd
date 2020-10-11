// Delay
SynthDef(\delay, {
    arg in, out,
    amp=1.0,
    maxdelaytime=0.2,
    delaytime=0.2,
    decaytime=1,
    atk=0.01, sustain=0.5, rel=0.01,
    gate=1;

    var sig, env;
    env = EnvGen.kr(Env.asr(atk, sustain, rel), gate);
    sig = In.ar(in, 2);
    sig = CombN.ar(sig, maxdelaytime, delaytime, decaytime);
    Out.ar(out, sig * env * amp);
}).add;
