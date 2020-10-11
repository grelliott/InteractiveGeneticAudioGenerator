SynthDef(\synthbass, {
    arg out, freq=440, amp=1.0, gate=1,
    atk=0.03, dec=0.01, sus=1, rel=0.07, crv= 0,
    atkf=0.03, decf=0.1, susf=0.2, relf=0.01, crvf= 2,
    lpfFreq=700,  lpfRq=1, detune=0.05;
    var osc1, osc2, osc3, sig,
    env, envf;

    osc1 = Saw.ar(freq);
    osc2 = Saw.ar(freq+detune);
    osc3 = SinOsc.ar(freq/2);

    env = EnvGen.kr(Env.adsr(atk, dec, sus, rel, curve:crv), gate, doneAction:2);
    envf = EnvGen.kr(Env.adsr(atkf, decf, susf, relf, curve:crvf), gate);

    sig = Mix.ar([osc1, osc2, osc3])!2;
    sig = RLPF.ar(sig, lpfFreq*envf, lpfRq);
    sig.softclip;

    Out.ar(out, sig * env * amp);
}).add;
