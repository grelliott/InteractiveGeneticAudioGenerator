SynthDef(\ambientpad, {
    arg out,
    freq=440,
    amp=1.0,
    gate=1,
    width=0.5,
    detune=1/4,
    atk=1.0, dec=1.0, sustain=2.0, rel=0.5,
    atkf=1.0, sustainf=0.5, relf=1.0,
    lpfFreq=2000,
    lpfRq = 0.5;

    var sig, env, envf, osc1, osc2;

    osc1 = Saw.ar(freq);
    //osc2 = PulseDPW.ar(freq+detune, width);
    envf = EnvGen.kr(Env.asr(atkf, sustainf, relf), gate, doneAction:2);
    //sig = Mix.ar([osc1, osc2]);
    sig = RLPF.ar(sig, lpfFreq * envf, lpfRq);

    sig = osc1!2;
    env = EnvGen.kr(Env.adsr(atk, dec, sustain, rel), gate, doneAction:2);
    Out.ar(out, sig * env * amp);
}).add;
