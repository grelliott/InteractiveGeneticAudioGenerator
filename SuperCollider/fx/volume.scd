SynthDef(\volumeCtl, { |out, amp = 1, gate = 1|
    var sig = In.ar(out, 2) * amp;
    sig = sig * EnvGen.kr(Env(#[1, 1, 0], #[1, 1], -3, releaseNode: 1), gate, doneAction: Done.freeSelf);
    ReplaceOut.ar(out, sig)
}).add;
