SynthDef(\squeekyPad, {
    arg out,
    freq=440,
    gate=1,
    chatter=0.5, // 0 = no chatter, 1.0 = lots of chatter (how much chatter from the squeeky parts slipping)
    squeek=0.0,
    amp = 1.0;

    var sig, env,
    atk = 2, sus = 1, rel = 2,
    atkRate = 1.1, susRate = 1.5, relRate = 1.1;
    // Stretch should increase the atk, sust, rel
    // if it's 0 then the whole thing shoudld start fairly slow, almost no sustain, fairly slow release
    // if it's -1 then it should start almost immediately
    // if it's 1 then
    // atk/rel could grow the same rate, sustains growth should be greater
    // atk = atk * (atkRate**squeek);
    // sus = sus * (susRate**squeek);
    // rel = rel * (relRate**squeek);

    sig = SawDPW.ar(freq, 0); //mayyybe modulate the iphase?
    env = EnvGen.ar(Env.asr(atk, sus, rel), gate: gate, doneAction: Done.freeSelf);

    Out.ar(out, sig * env * amp);
}).add;

