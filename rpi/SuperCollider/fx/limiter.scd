// Limiter
SynthDef(\limiter, {
    arg out,
    amp=0.8,
    dur=0.01;
    ReplaceOut.ar(out, Limiter.ar(In.ar(out, 2), amp, dur));
}).add;
