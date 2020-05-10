SynthDef(\kick, {
    arg freq=400, freqB=50, freqC=1,
    freqDur1=0.01, freqDur2=0.2, freqC1=1, freqC2=(-1),
    atk=0.01, rel=1,
    c1=1, c2=(-12),
    amp=0.8, pan=0,
    out;

    var sig, env, freqSweep;

    freqSweep = Env([freq, freqB, freqC], [freqDur1, freqDur2], [freqC1, freqC2]).ar;
    env = EnvGen.kr(Env([0,1,0],[atk,rel],[c1,c2]), doneAction: Done.freeSelf);
    sig = SinOsc.ar(freqSweep, pi/2);
    sig = sig * env;
    sig = Pan2.ar(sig, pan, amp);
    Out.ar(out, sig);
}).add;
