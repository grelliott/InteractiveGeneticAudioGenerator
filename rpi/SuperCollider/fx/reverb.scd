// Reverb
//TODO look into reverb generators (freeverb)
SynthDef(\reverb, {
    arg in, out,
    predelay=0.1,
    revtime=1.8,
    lpfFreq=4500,
    mix=0.15,
    wet=0,
    amp=1;

    var sig, dry, temp;
    dry = In.ar(in, 2);
    temp = In.ar(in, 2);
    temp = DelayN.ar(temp, 0.2, predelay);
    temp = AllpassN.ar(temp, 0.05, {Rand(0.001, 0.05)}!2, revtime);
    temp = LPF.ar(temp, lpfFreq);
    2.do({
        arg item;
        temp = AllpassN.ar(temp, 0.05, {Rand(0.001, 0.05)}!2, revtime);
        temp = LPF.ar(temp, lpfFreq);
        wet = wet + temp;
    });
    //temp = AllpassN.ar(temp, 0.05, {Rand(0.001, 0.05)}!2, revtime);
    //temp = LPF.ar(temp, lpfFreq);

    sig = XFade2(dry, wet, mix*2-1, amp);
    Out.ar(out, sig);
}).add;
