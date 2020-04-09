(
s.quit;

// 1. Server config
o = ServerOptions.new;

// RaspberryPi
o.device = "maudiogene";
o.numOutputBusChannels = 2;
o.numInputBusChannels = 0;
o.sampleRate = 48000;
o.hardwareBufferSize = 2.pow(8);  // TODO determine correct size for this
o.blockSize = 64;
o.memSize = 2.pow(16);  // Or 2.pow(20)
o.maxLogins = 2;

// Laptop
/*
o.device = "ASIO";
o.numOutputBusChannels = 12;
o.numInputBusChannels = 2;
o.hardwareBufferSize = 2.pow(5);
o.blockSize = 64;
o.sampleRate = 48000;
o.memSize = 2.pow(16);  // Or 2.pow(20)
o.maxLogins = 2;
*/

// s = Server(\newServer, NetAddr("127.0.0.1", 57110), o);
s = Server.local;
s.options = o;

ServerBoot.removeAll;
ServerTree.removeAll;
ServerQuit.removeAll;

//2. Global variables
~masterout = Bus.new(\audio, 0, 2, s);  // lowest index of main out
//~micin = Bus.new(\audio, 0, 1, s); // Mic In channel


~bpm = 120;
~t = TempoClock.new(~bpm/60).permanent_(true);

~loadFiles = { |base, files|
    files.do({ |file, i|
        (base++file).load;
    });
};

~basePath = "/home/pi/src/audiogene/rpi/SuperCollider/";

~makeBusses = {
    s.newBusAllocators;
    ~bus = Dictionary.new;
    ~bus.add(\out -> Bus.audio(s, 2));
    ~out = ~bus[\out];
    //TODO patch \out into masterFxBus
    ~bus.add(\reverbBus -> Bus.audio(s, 2));
    ~bus.add(\delayBus -> Bus.audio(s, 2));
    ~bus.add(\masterFxBus -> Bus.audio(s, 2));
    //TODO patch masterFxBus to masterout
};

// Set up a few key groups
~makeNodes = {
    s.bind({
        ~mainGroup = Group.head;
        // dd groups for each instrument group
        ~fxGroup = Group.after(~mainGrp);
        ~masterFxGroup = Group.tail;
    });
};

//12.ctranspose
// ~masterout
// ~out
// ~bus[\jpverbBus]
// ~bus[\masterFxBus]



~addRoutines = {
    ~setTempo = { |bpm|
        // Schedule tempo change to next bar
        ~t.schedAbs(~t.nextBar, {
            ~bpm = bpm;
            "Setting BPM to ".post;
            ~bpm.postln;
            ~t.tempo = ~bpm / 60;
        });
    };

    ~adjustTempo = { |dir|
        ~setBpm.value(~bpm + (5 * dir.sign));
    };

    ~setScale = { |scale|
        "Setting scale to ".post;
        scale.postln;
    };
};

~setupOSC = {
    ~oscPort = 51730;
    ~osc = NetAddr("localhost", ~oscPort);
    OSCdef.new(\notifyOSC, {
        arg msg, time, addr, port;
        [msg, addr].postln;
        if (msg[1] == 1) {
            "Starting music".postln;
            //e[\event1].value;
            ~masterFxGroup.set(\gate, 1);
            ~fxGroup.set(\gate, 1);
            ~mainGroup.set(\gate, 1);
            addr.sendMsg("/done", "awake");
        }
    },
    '\notify'
    );

    OSCdef.new(\tempo, { |msg, time, addr, port|
        ~setTempo.value(msg[1]);
    },
    '/gene/tempo'
    );

    OSCdef.new(\scaleOSC, { |msg, time, addr, port|
        ~setScale.value(msg[1]);
    },
    '/gene/scale'
    );
};


~makeEvents = {
    e = Dictionary.new;

    e.add(\event1 -> {
        Pbind(
            \instrument, \kick,
            \rel, 0.1,
            \dur, 1/2,
            \stretch, TempoClock.tempo,
            \gate, 0,
            \group, ~mainGrp,
            \out, ~out,
        ).play(quant: 15) ;
    });
    e.add(\event2 -> {"Event 1".postln;});
    e.add(\event3 -> {"Event 1".postln;});

    // Use quant to time entry/exit of sequences to bars/beats of tempo
};

~loadBuffers = {
    var bufferPath, b;
    bufferPath = ~basePath++"buffers/";
    b = Dictionary.new;
    PathName(bufferPath).entries.do{
        arg subfolder;
        b.add(
            subfolder.folderName.asSymbol ->
            Array.fill(
                subfolder.entries.size,
                {
                    arg i;
                    Buffer.read(s, subfolder.entries[i].fullPath);
                }
            )
        );
    };
};

~loadSynths = {
    ~loadFiles.value(~basePath++"synths/", [
        "kick.scd",
        "synthbass.scd",
        "ambientpad.scd"
    ]);
};

~makeSynths = {
    SynthDef(\metronome, {
        arg out=~out, freq=440, amp=1, atk=0.01, dec=1, sus=0.1, rel=0.1, gate=1;
        var sig, env;
        sig = SinOsc.ar(freq, 0);
        env = EnvGen.kr(Env.adsr(atk, dec, sus, rel), gate, doneAction:2);
        sig = sig * env * amp;
        Out.ar(out, sig);
    }).add;

/*
    SynthDef(\sine_a, {
        arg out=~out, freq=440, amp=1, atk=0.01, dec=1, sus=0.1, rel=0.1, gate=1;
        var sig, env;
        sig = SinOsc.ar(freq, 0);
        env = EnvGen.kr(Env.adsr(atk, dec, sus, rel), gate, doneAction:2);
        sig = sig * env * amp;
        Out.ar(out, sig);
    }).add;
*/

};

~loadFx = {
    ~loadFiles.value(~basePath++"fx/", [
        "reverb.scd",
        "delay.scd",
        "limiter.scd",
    ]);
};

~makeFx = {
    // Compressor

    // Saturation
};

// Create some sequences to work with
~makeSequences = {

};

~cleanup = {
    s.newBusAllocators;
    ServerBoot.removeAll;
    ServerTree.removeAll;
    ServerQuit.removeAll;
};


//4.  Register functions with ServerBoot/Quit/Tree
ServerQuit.add(~cleanup);

ServerBoot.add(~makeBusses);
ServerBoot.add(~makeNodes);
//ServerBoot.add(~loadBuffers);
ServerBoot.add(~loadSynths);
//ServerBoot.add(~makeSynths);
//ServerBoot.add(~loadFx);
//ServerBoot.add(~makeFx);

ServerBoot.add(~addRoutines);
//ServerBoot.add(~makeSequences);
//ServerBoot.add(~makeEvents);
ServerBoot.add(~setupOSC);


//5. Boot server
s.waitForBoot({
    // wait for previous server actions to complete
    s.sync;

/*
    // Mic In
    SynthDef(\micin, {
        arg in=~micin, out=~out, amp=0.5;
        var sig;
        sig = SoundIn.ar(in);
        Out.ar(out, sig*amp);
    }).add;
*/
    ~kickSynth = Synth.new(\ambientpad, [
        \amp, 1.0,
        \out, ~masterOut
    ], ~mainGroup);

/*
    ~reverbSynth = Synth.new(\reverb, [
        \amp, 1,
        \predelay, 0.1,
        \revtime, 1.8,
        \lpf, 4500,
        \mix, 0.35,
        \in, ~bus[\reverb],
        \out, ~out
    ], ~reverbGrp);

    ~delaySynth = Synth(\delay, [
        \amp, 1.0,
        \maxdelaytime, 0.2,
        \delaytime, 0.2,
        \decaytime, 1,
        \atk, 0.01,
        \sustain, 0.5,
        \rel, 0.01,
        \in, ~bus[\delayBus],
        \out, ~out
    ], ~fxGroup);


    // Add a master limiter/compressor
    // Add in between out and master out
    ~limiter = Synth(\limiter, [
        \level, 0.8,
        \dur, 0.02,
        \in, ~out,
        \out, ~masterout
    ], ~masterFxFinalGrp);
*/
    s.sync;

    ~osc.sendMsg('/done', "awake");
    "done".postln;
});
)  // Done server load



/*
(
Pbindef(\mykick,
    \instrument, \kick,
    \amp, 1,
    \note, Pseq([0,0,7,7, 0], inf),
    \dur, Pseq([1,1,1/2,1/2,1], inf),
    \rel, 0.2,
    \out, 0,
    \group, ~mainGrp
);
)

Pbindef(\mykick, \out, ~bus[\jpverbBus]);
Pbindef(\mykick, \out, 0);
Pbindef(\mykick).play(~t);
Pbindef(\mykick).stop;

~setTempo.value(126);
~adjustTempo.value(1);


(
Pbindef(\t,
    \instrument, \synthbass,
    \detune, 4/24,
    \midinote, Pseq([69, 74, 76, 77, 79, 81, Pseq([81, 79, 81, 82, 79, 81], 2), 82, 81, 79, 77, 76, 74, 74], inf),
    //\note, Pseq([[0, 3, 7], [2, 5, 8], [3, 7, 10], [5, 8, 12]], inf),
    \dur, Pseq([Pn(1, 5), 3, Pn(1, 5), 3, Pn(1, 6), 1/2, 1/2, 1, 1, 3, 1, 3], inf),
    //\dur, Pseq([1/4,1/4,1/4,1/4], inf),
    \amp, 0.2,
    \group, ~mainGrp,
    \out, ~bus[\jpverbBus]
).play;
)

~mainGrp.freeAll;
Pbindef(\t, \out, ~out);
Pbindef(\t, \out, ~bus[\jpverbBus]);
Pbindef(\t, \instrument, \ambientpad1);
Pbindef(\t, \dur, Pseq([1, 2, 1/2, 2], inf));
Pbindef(\t, \octave, 4);
Pbindef(\t).play
Pbindef(\t).stop

~reverbSynth.set(\sz, 0.5);
s.queryAllNodes;
m = Synth(\micin, [\out, ~bus[\jpverbBus]], target:~mainGrp);
m.set(\out, ~out);
m.set(\out, ~bus[\jpverbBus]);
m.free;


a = Synth(\synthbass, [\pitch, 36, \detune, (4/24)], target:~mainGrp);
a.set(\out, ~bus[\reverbBus]);
~jpverb.set(\out, ~bus[\delayBus]);
a.set(\out, ~out);
~reverbSynth.set(\gate, 1);
a.set(\gate, 1);
a.set(\gate, 0);
~limiter.set(\limit, 0.4);
~reverbSynth.set(\gate, 0);

s.freeAll;



//6. Stuff to do after server is booted
~bpm = 120;
p = Pseq([Pfunc({Synth(\metronome).play; 60/~bpm})], inf).play;
s.freeAll;
*/
