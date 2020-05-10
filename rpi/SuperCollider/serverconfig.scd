(
s.quit;

// 1. Server config
o = ServerOptions.new;

// RaspberryPi
/*
o.device = "audiogene";
o.numOutputBusChannels = 2;
o.numInputBusChannels = 0;
o.sampleRate = 48000;
//o.hardwareBufferSize = 16; //2.pow(12);  // TODO determine correct size for this
o.blockSize = 64;
o.memSize = 2.pow(18);  // Or 2.pow(20)
o.maxLogins = 2;
~basePath = "/home/pi/src/audiogene/rpi/SuperCollider/";
*/

///*
// Laptop
/*
o.device = "ASIO";
o.numOutputBusChannels = 12;
o.numInputBusChannels = 6;
o.sampleRate = 48000;
o.hardwareBufferSize = 32;
o.blockSize = 32;
*/
// /*
o.outDevice = "CABLE Input";
// */
o.memSize = 2.pow(16);  // Or 2.pow(20)
o.maxLogins = 2;
~basePath = "Z:/src/audiogene/rpi/SuperCollider/";
//*/

s = Server.local;
s.options = o;

ServerBoot.removeAll;
ServerTree.removeAll;
ServerQuit.removeAll;


~oscPort = 51730;

/*
 * TODO look at setting up an OSC connection with an app/webpage
 * expose the environment controls.
 * probably use python for most of it, or javascript...?
 * looks like javascript on nodejs would work
 *
 * Set up parameters for volume, EQ, limiter
 */
~gain = 0.0;
~lpCutoff = 22000;
~lpGain = -45.0;
~hpCutoff = 35;
~hpGain = -45.0;


~masterout = Bus.new(\audio, 0, 2, s);
// ~micin = Bus.new(\audio, 0, 1, s); // Mic In channel

~t = TempoClock.default;


~loadFiles = { |base, files|
    files.do({ |file, i|
        (base++file).load;
    });
};


~makeBusses = {
    s.newBusAllocators;
    ~bus = Dictionary.new;
    ~bus.add(\out -> Bus.audio(s, 2));
    ~out = ~bus[\out];

	~bus.add(\limiterBus -> Bus.audio(s, 2));
	~bus.add(\cutoffBus -> Bus.audio(s, 2));
    ~bus.add(\masterFxBus -> Bus.audio(s, 2));

	// Add control busses that can carry signals to any synth that needs
	// to know the value for a parameter

	// rythymBus carries a signal that indicates
	// how much rythym should be in the piece (none to 4/4 to broken...?)
	~bus.add(\rhythmBus -> Bus.control(s));
	// vibeBus (name tbd) could toggle things like
	// scale, or sample selection
	~bus.add(\vibeBus -> Bus.control(s));
};


// Set up a few key groups
~makeNodes = {
	~mainGroup = Group.head;
	~padsGroup = Group.new(~mainGroup, \addToHead);
	// add groups for each instrument group
	~fxGroup = Group.after(~mainGroup);
	~masterFxGroup = Group.tail;
};


~addRoutines = {
	~bpm = 120;
    ~setTempo = { |bpm|
        // Schedule tempo change to next bar
        ~t.schedAbs(~t.nextBar, {
            "Setting BPM to ".post;
            bpm.postln;
			~t.tempo = bpm / 60;
        });
    };

    ~adjustTempo = { |dir|
        ~setBpm.value(~bpm + (5 * dir.sign));
    };

    ~setScale = { |scale|
        "Setting scale to ".post;
        scale.postln;
    };

	~setVibe = { |vibe|
		~bus[\vibeBus].set(vibe);
	};
};


~setupOSC = {
    ~osc = NetAddr("localhost", ~oscPort);
    /*OSCdef.new(\notifyOSC, {
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
    );*/

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

	OSCdef.new(\vibeOSC, { |msg, time, addr, port|
        ~setVibe.value(msg[1]);
    },
    '/gene/vibe'
    );
};


/*
 * Load up all the samples in the buffers path
 * TODO see about organizing them by style?
 * TODO have a keyed lookup to get the buffer number?
 * eg getBuffer("pads/foo")
 * or getBuffer("field/recording1")
 */
~loadBuffers = {
    var bufferPath;
    bufferPath = ~basePath++"buffers/";
    ~b = Dictionary.new;
    PathName(bufferPath).entries.do { | subfolder |
		subfolder.folderName.postln;
        ~b.add(
            subfolder.folderName.asSymbol ->
            Array.fill(subfolder.entries.size, { | i |
				subfolder.entries[i].fullPath.postln;
				Buffer.read(s, subfolder.entries[i].fullPath);
				//bufnum;
			})
        );
    };
};


~loadSynths = {
    ~loadFiles.value(~basePath++"synths/", [
		"kick.scd",
		"synthbass.scd",
		"ambientpad.scd",
		"voice.scd",
	]);
};


~makeSynths = {
	//TODO add routing for MIDI to test synths
	// send keys as note inputs for the synth

    SynthDef(\sine_a, {
		arg out=~out,
		freq=440,
		amp=1,
		atk=0.01, dec=1, sus=0.1, rel=0.1,
		gate=1;
		var sig, env;
		sig = SinOsc.ar(freq, 0);
		env = EnvGen.kr(Env.adsr(atk, rel), gate, doneAction:2);
		sig = sig * env * amp;
		Out.ar(out, sig);
	}).add;

	/*SynthDef(\metronome, {
		arg out=~out,
		freq=440,
		amp=1,
		atk=0.01, rel=0.1,
		gate=1;

		var sig, env;
		sig = SinOsc.ar(freq, 0);
		env = EnvGen.kr(Env.perc(atk, rel), gate: gate, doneAction:2);
		sig = sig * env * amp;
		Out.ar(out, sig);
	}).add;*/

	// play 1-channel sample
	SynthDef(\playSample1, {
		arg out,
		sample,
		loop = 0;

		Out.ar(out, PlayBuf.ar(1, sample, loop: loop, doneAction: Done.freeSelf));
	}).add;

	// play 2-channel samples
	SynthDef(\playSample2, {
		arg out,
		sample,
		loop = 0;

		Out.ar(out, PlayBuf.ar(2, sample, loop: loop, doneAction: Done.freeSelf));
	}).add;

	// set up some ambient pads (squeeks, groans, whooshes)

/*
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

	Pbindef(\mypad,
		\instrument, \squeekyPad,
		\out, ~masterout,

		\amp, 1,
		\degree, Pseq([0, 1, 2, 3, 4, 5, 6, 7], 1),
		\scale, Pfunc({Scale.major}, inf),
		\dur, 0.25,
		\squeek, 0.1,
		// \chatter, 0.5,
		\group, ~mainGroup
	);
	Pbindef(\mypad).play
	Pbindef(\mypad, \db, -40)
	Pbindef(\mypad, \scale, Scale.minor);
	Pbindef(\mypad).stop
	Pbindef(\mypad, \gate, 1)
	~mainGroup.set(\db, 1.0)
	~mainGroup.freeAll
*/

	/*SynthDef(\groan, {
		arg out,
		gate=0,
		amp=1.0,

	});*/

	// readme https://theseanco.github.io/howto_co34pt_liveCode/2-1-Recommended-Addons/

	// add some clicking sounds (wooden, insecty, metallic)
	// use samples for these

	// add some voices (with echo and delay)
	// maybe like those voices from Annihilation
	// or choral-sounding (uplifting)
};


/*
 * Look at VST plugins for some fx
 * for sound FX:
 * need a very good reverb, with a few parameters
 * need a good timestretch and pitchbend
 *
 * for audio FX:
 * need a limiter
 * need an EQ
 * need a bandpass filter
 *
 */
~loadFx = {
	~loadFiles.value(~basePath++"fx/", [
		//"reverb.scd",
		"schroederReverb.scd",
		"delay.scd",
		"limiter.scd",
		"cutoff.scd",
		"paulstretch.scd",
		"volume.scd",
	]);
};


~makeFx = {
	// bitcrush
	// StageLimiter quark (BatLib)

	// Tape stretch

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
ServerTree.add(~makeNodes);
ServerTree.add(~makeBusses);
ServerTree.add(~addRoutines);
ServerTree.add(~setupOSC);

ServerBoot.add(~loadBuffers);
ServerBoot.add(~loadSynths);
ServerBoot.add(~makeSynths);
ServerBoot.add(~loadFx);
ServerBoot.add(~makeFx);

//ServerBoot.add(~makeSequences);
//ServerBoot.add(~makeEvents);

ServerQuit.add(~cleanup);

//5. Boot server
s.waitForBoot({
	"Server booted".postln;

    // wait for previous server actions to complete
    s.sync;

	// Take a look at NDef for perhaps swapping synths in and out

/*
    // Mic In
    SynthDef(\micin, {
        arg in=~micin, out=~out, amp=0.5;
        var sig;
        sig = SoundIn.ar(in);
        Out.ar(out, sig*amp);
    }).add;
*/

	/*
	 * Utilities
	 */
	// Metronome / Click
	// Sine


	/*
	 * Maybe layer up different samples? Skip the synths for now?
	 * Maybe key samples and play as notes?
	 */

	/*
	 * Instruments
	 */

	// add some sample playback (through tape stretch, saturation, delay)
	// TODO change to Pbindef
	 /*({
	 	var envBuf, envSignal, buffer;
	 	buffer = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav");
	 	// The grain envelope
	 	envBuf = Buffer.alloc(s, s.sampleRate, 1);
	 	envSignal = Signal.newClear(s.sampleRate).waveFill({|x| (1 - x.pow(2)).pow(1.25)}, -1.0, 1.0);
	 	envBuf.loadCollection(envSignal);
	 	s.sync();
	 	// Runs indefinitely
	 	Synth(\paulstretchMono, [\bufnum, buffer.bufnum, \envBufnum, envBuf.bufnum]);
	 	}.fork;
	 );*/

	//~mainGroup.set(\gate, 1);

	// mix
	// http://www.dewdrop-world.net/sc3/tutorials/index.php?id=1
	// Add levels mixer
	// maybe use some modulation to automate the amplitude
	// should it be set up before?
	// mixer.addChannel(\name, [\out, \dest], \channels, 2)
	// should add a new
	// mixer(\name).set(\amp, -4db) // should take something that can modulate
	// mixer(\name).set(\out, \newdest)


	/*
	* FX
	*/
/*
	~reverbSynth = Synth.new(\reverb, [
		\amp, 1,
		\predelay, 0.1,
		\revtime, 1.8,
		\lpf, 4500,
		\mix, 0.35,
		\in, ~bus[\reverbBus],
	   \out, ~out
	], ~fxGroup);
*/
/*
	~delaySynth = Synth(\delay, [
		\maxdelaytime, 0.8,
		\delaytime, 0.2,
		\decaytime, 1.4,
		\atk, 0.01,
		\sustain, 1.0,
		\rel, 0.01,
		\in, ~bus[\delayBus],
		\out, ~out
	], ~fxGroup);
*/

	/*
	* Master FX
	*/
	// Cutoff EQ
	~cutoffEq = Synth(\cutoff, [
		\lpCutoff, ~lpCutoff,
		\lpGain, ~lpGain,
		\hpCutoff, ~hpCutoff,
		\hpGain, ~hpGain,
		\in, ~out,
		\out, ~bus[\limiterBus]
	], ~masterFxGroup);

	// EQ

	//TODO verify limiter works as expected
	// It should prevent any clipping, but should not
	// distort any sounds under the limit
	// Limiter
	//~gainBus = Bus.audio(s, 2);
	~limiter = Synth.after(~cutoffEq, \limiter, [
		\amp, 0.0,
		\dur, 0.02,
		\in, ~bus[\limiterBus],
		\out, ~masterout
	]);

	//TODO need volume
	~gainSynth = Synth.after(~limiter, \volumeCtl, [
		\amp, ~gain.dbamp,
		\out, ~masterout
	]);
	// s.sync;
	//~osc.sendMsg('/done', "awake");

	// send event to start playing
	"done".postln;
});
)  // Done server load


~cutoffEq.set(\hpCutoff, 500);

// Look at Pgroup for getting a collection of Pbind objects to play...?
// Also look at Pfx/Pfxb!! And Pbus
// check out https://depts.washington.edu/dxscdoc/Help/Tutorials/A-Practical-Guide/PG_06f_Server_Control.html
(
var syn, fxr;
syn = Pbindef(\voices,
	\instrument, \voice,
	\midinote, Prand(#[24,28,29,48], inf),
	\pan, Pprob([-0.5, 0.5]),
	\atk, Pprob([4,14]),
	\rel, Pprob([5,25]),
	\amp, 1.0,
	\group, ~padsGroup
);
~g = Pfxb(syn, \schroederReverb,
	\predelay, 0.1,
	// \revtime, 1.8,
	// \lpf, 4500,
	\mix, 0.15,
	\group, ~fxGroup
).play;
// ~g = Ppar([fxr, syn]).play;
// Pbindef(\voices).play(~t);
)
~g.new(\midinote, Pseq(#[18, 21, 22, 25, 27], inf));
~g.stop
Pbindef(\voices).stop

(
var reverb, reverbBus, envBuf, envSignal, synth;

reverbBus = Bus.audio(s, 2);
reverb = Synth.new(\schroederReverb, [
	// \amp, 1,
	\predelay, 0.1,
	// \revtime, 1.8,
	// \lpf, 4500,
	\mix, 0.35,
	\in, reverbBus,
	\out, ~out
], ~fxGroup);

envBuf = Buffer.alloc(s, s.sampleRate, 1);
envSignal = Signal.newClear(s.sampleRate).waveFill({|x| (1 - x.pow(2)).pow(1.25)}, -1.0, 1.0);
envBuf.loadCollection(envSignal);
~synth = Synth.new(\paulstretchMono, [
	\out, reverbBus,
	\bufnum, ~b[\sounds][0],
	\envBufnum, envBuf,
	\stretch, 100
], ~padsGroup);

//synth.run(false);
)
~synth.set(\stretch, 1);
~synth.run(false);


(
Pbindef(\rainSample,
	\instrument, \playSample2,
	\out, ~bus[\delayBus],
	\sample, ~b[\sounds][0],
	\amp, 0.8,
	\group, ~mainGroup
);

)
Pbindef(\rainSample, \amp, 0.4).stop

~delaySynth.set(\decaytime, 0.5)
~mainGroup.freeAll

~padsGroup.freeAll
~fxGroup.freeAll


/*

// Example
(
{
    var envBuf, envSignal, buffer;
    buffer = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav");
    // The grain envelope
    envBuf = Buffer.alloc(s, s.sampleRate, 1);
    envSignal = Signal.newClear(s.sampleRate).waveFill({|x| (1 - x.pow(2)).pow(1.25)}, -1.0, 1.0);
    envBuf.loadCollection(envSignal);
    s.sync();
    // Runs indefinitely
    Synth(\paulstretchMono, [\bufnum, buffer.bufnum, \envBufnum, envBuf.bufnum]);
}.fork;
)

~setTempo.value(70);
Pbindef(\mykick).stop;

// (
/* Voices

http://www.kimri.org/blog/?p=64

 */
// Ndef(\rev,{
// 	Out.ar(0,Limiter.ar(FreeVerb.ar(LPF.ar(In.ar([0,1]),10000),mix:0.33),0.7));
// 	};
// );

// Pbindef(\voices).play;
// Tdef(\voices).play;
// Tdef(\voices).stop;
// )
// s.quit;

// Pbindef(\mykick).stop
// ~mainGroup.stop

*/

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
