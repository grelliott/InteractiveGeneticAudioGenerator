// Limiter
SynthDef(\limiter, {
    arg in, out,
    amp=0.8,
    dur=0.01;
    Out.ar(out, Limiter.ar(In.ar(in, 2), amp, dur));
}).add;
