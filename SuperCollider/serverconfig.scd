(
"".postln;
"Restarting server".postln;
s.quit;


~basePath = "/home/pi/audiogene/SuperCollider/";
// ~oscAddress = "192.168.1.29";
// ~oscPort = 57120;

// 1. Server config
o = ServerOptions.new;
//
// // RaspberryPi 4
o.numOutputBusChannels = 2;
o.numInputBusChannels = 0;
o.sampleRate = 48000;
// // //o.hardwareBufferSize = 16; //2.pow(12);  // TODO determine correct size for this
o.blockSize = 64;
o.memSize = 2.pow(20);  // Or 2.pow(20)
o.maxLogins = 2;
o.bindAddress = "0.0.0.0";
//

s = Server.local;
s.options = o;
s.dumpOSC;


/*
 * TODO look at setting up an OSC connection with an app/webpage
 * expose the environment controls.
 * probably use python for most of it
  */


 /*
  * Set up parameters for volume, EQ, limiter
  */
~gain = 0.0;  // in dB
~lpCutoff = 8000;
~hpCutoff = 60;
~limiterAmp = 0.8;
~limiterDur = 0.02;
~tempo = 120;
~tempoMin = 60;
~tempoMax = 180;


// Set up scales from 0 - 12
// 0 being most moody, 12 being most joyous
// Then to set scale, just pick the scale from the array at the index
    // A dark moody vibe may use a range of scales such as Aeolian, harmonic minor, melodic minor, locrian
    // upbeat may use scales such as major, mixolydian, lydian, phrygian, harmonic major, dorian (may be on border)
~scales = [
    Scale.locrian,
    Scale.melodicMinor,
    Scale.harmonicMinor,
    Scale.aeolian,
    Scale.dorian,
    Scale.harmonicMajor,
    Scale.phrygian,
    Scale.lydian,
    Scale.mixolydian,
    Scale.major
];
~scale = Scale.major;



/*
 * Helper functions
 */
~loadFiles = { |base, files|
    files.do({ |file, i|
        (base++file).load;
    });
};

/*
 * Server setup routines
 */
~makeBusses = {
    s.newBusAllocators;


    ~bus = Dictionary.new;

    // Set up audio busses

    // This is the audio bus to the output device
    ~masterout = Bus.new(\audio, 0, 2, s);

    // This is where to send any signals destined for
    // output. There will be master FX chained between it and masterout
    ~bus.add(\out -> Bus.audio(s, 2));
    ~out = ~bus[\out];


    // Set up control busses

    ~bus.add(\gainBus -> Bus.control(s));
    ~gainBus = ~bus[\gainBus];
    ~gainBus.set(~gain.dbamp);

	~bus.add(\lpCutoffBus -> Bus.control(s));
	~bus.add(\hpCutoffBus -> Bus.control(s));
	~bus.add(\limiterAmpBus -> Bus.control(s));
	~bus.add(\limiterDurBus -> Bus.control(s));

    // rythymBus carries a signal that indicates
    // how much rythym should be in the piece (none to 4/4 to random...?)
    ~bus.add(\rhythmBus -> Bus.control(s));

    // Energy bus may control the kind of energy the piece has
    // Perhaps going from a calm relaxed energy
    // to an upbeat energetic feeling
    // May not need tempo OSC for this
    // Lower energy may be low tempo, maybe less instruments, more open arrangments, less structured rythm
    // higher energy may be higher tempo, more instruments, more structured rythm

   ~bus.add(\energyBus -> Bus.control(s));

    // vibeBus (name tbd) could toggle things like
    // scale, or sample selection
    // Perhaps moving from a dark moody vibe to a upbeat happy vibe
    // A dark moody vibe may use a range of scales such as Aeolian, harmonic minor, melodic minor, locrian
    // upbeat may use scales such as major, mixolydian, lydian, phrygian, harmonic major, dorian (may be on border)
    // Dark may use drawn out simple chords
    // upbeat may use snappier chords, maybe a bit more complex or at least arranged to feel happy

    ~bus.add(\vibeBus -> Bus.control(s));

    // Theme bus may change from a natural organic theme to a digital techy theme
    // organic theme may contain field-recordings played out very close to reality
    //   perhaps nature-based instruments (sticks, wooden pipes, etc)
    // digital theme may contain synths, perhaps with heavy effects

    ~bus.add(\themeBus -> Bus.control(s));
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
    ~t = TempoClock.default;
    ~setTempo = { |bpm|
        ~tempo = bpm;
        // Schedule tempo change to next bar
        ~t.schedAbs(~t.nextBar, {
            "Setting BPM to ".post;
            ~tempo.postln;
            ~t.tempo = ~tempo / 60.0;
        });
    };

    ~setScale = { |scale|
        "Setting scale to ".post;
        ~scales.clipAt(scale).name.postln;
        ~scale = ~scales.clipAt(scale);
    };

    ~setGain = { |db|
        if( db > 6, {
            db.post;
            "db is greater than the max 6db".postln;
            nil;
        },
        {
            "Setting gain to ".post;
            db.post;
            "db".postln;
            ~gain = db;
            ~gainBus.set(~gain.dbamp);
			~notifySubscribers.value("/update/gain", ~gain);
        });
    };

	~setCutoff = { |hpCutoff, lpCutoff|
		~hpCutoff = hpCutoff;
		~lpCutoff = lpCutoff;
		~bus[\hpCutoffBus].set(~hpCutoff);
		~bus[\lpCutoffBus].set(~lpCutoff);
	};

	~setLimiter = { |amp, dur|
		~limiterAmp = amp;
		~limiterDur = dur;
		~bus[\limiterAmpBus].set(~limiterAmp);
		~bus[\limiterDurBus].set(~limiterDur);
	};

    ~setEnergy = { |energy|
        "Setting energy to ".post;
        energy.postln;
        // set the tempo based on the value of energy
        // Assume it falls between 0-255
        // and tempo falls between tempoMin and tempoMax
        ~setTempo.value(energy.linlin(0, 255, ~tempoMin, ~tempoMax));
        ~bus[\energyBus].set(energy);
    };

    ~setVibe = { |vibe|
        "Setting vibe to ".post;
        vibe.postln;
        ~bus[\vibeBus].set(vibe);
    };

    ~setTheme = { |theme|
        "Setting theme to ".post;
        theme.postln;
        ~bus[\themeBus].set(theme);
    };
};


~setupOSC = {
//	~osc = NetAddr.new(~oscAddress, ~oscPort);

/*
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
*/

    OSCdef.new(\tempoOSC, { |msg, time, addr, port|
        ~setTempo.value(msg[1]);
    },
    '/gene/tempo'
    );

    OSCdef.new(\scaleOSC, { |msg, time, addr, port|
        ~setScale.value(msg[1]);
    },
    '/gene/scale'
    );

    OSCdef.new(\energyOSC, { |msg, time, addr, port|
        ~setEnergy.value(msg[1]);
    },
    '/gene/energy'
    );

    OSCdef.new(\vibeOSC, { |msg, time, addr, port|
        ~setVibe.value(msg[1]);
    },
    '/gene/vibe'
    );

    OSCdef.new(\themeOSC, { |msg, time, addr, port|
        ~setTheme.value(msg[1]);
    },
    '/gene/theme'
    );

	OSCdef.new(\gainOSC, { |msg, time, addr, port|
		~setGain.value(msg[1]);
	},
	'/ctrl/gain'
	);

	OSCdef.new(\subscriberOSC, { |msg, time, addr, port|
		var receiverAddr = NetAddr.new(addr.ip, msg[1]);
		~addSubscriber.value(receiverAddr);
	},
	'/subscribe'
	);

	OSCdef.new(\unsubscribeOSC, { |msg, time, addr, port|
		~removeSubscriber.value(addr);
	},
	'/unsubscribe'
	);
};

// Need a data struct of NetAddr's
// addSubscription inserts the value into the struct
// removeSubscription removes the value from the struct
// notifySubscribers takes an OSCMessage and sends it to each member of the subscription
~setupSubscription = {
	~subscribers = Set.new();

	~addSubscriber = { |addr|
		"Adding new subscriber ".post;
		addr.postln;
		~subscribers.add(addr);
	};

	~removeSubscriber = { |addr|
		"Removing subscriber ".post;
		addr.postln;
		~subscribers.remove(addr);
	};

	~notifySubscribers = { |msg, val|
		"Notifying subscribers ".post;
		msg.post;
		" ".post;
		val.postln;
		~subscribers.do({ |item, i|
			"Sending message to ".post;
			item.postln;
			item.sendMsg(msg, val);
		});
	};
};

// Set up MIDI input
~setupMIDI = {
    // Initialize client
    MIDIClient.init;
    //MIDIClient.init(1, 0);
    MIDIIn.connectAll;

    ~midiDebug = MIDIFunc.noteOn({ |veloc, num, chan, src|
        "MIDI On: ".post;
        num.postln;
    });
    ~prepareMIDI = { |synthdef|
        var notes;
        notes = Array.newClear(128);

        ~midiOn = MIDIFunc.noteOn({ |veloc, num, chan, src|
                notes[num] = Synth(synthdef.asSymbol, [\out, ~masterout, \freq, num.midicps, \amp, veloc * 0.00315]);
        });
        ~midiOff = MIDIFunc.noteOff({ |veloc, num, chan, src|
            notes[num].release;
        });
    };
    ServerQuit.add({ ~midiDebug.free; ~midiOn.free; ~midiOff.free; MIDIIn.disconnectAll; });
};

/*
 * Load up all the samples in the buffers path
 */
~loadBuffers = {
    {
        var bufferPath = ~basePath++"buffers/";
        ~b = Dictionary.new;
        PathName(bufferPath).entries.do { | subfolder |
            ~b.put(subfolder.folderName.asSymbol, Dictionary.new);
            subfolder.folderName.postln;
            subfolder.entries.do { |file|
               ~b[subfolder.folderName.asSymbol].put(file.fileNameWithoutExtension.asSymbol, Buffer.read(s, file));
            };
        };
    };
    ~getBuffer = { |folder, file|
        ~b.at(folder.asSymbol).at(file.asSymbol);
    };
};


~loadSynths = {
    ~loadFiles.value(~basePath++"synths/", [
        "kick.scd",
		"apad_mh.scd",
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
*/

/*
    SynthDef(\groan, {
        arg out,
        gate=0,
        amp=1.0,

    });
*/

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
        "reverb.scd",
        "schroederReverb.scd",
        "delay.scd",
		"paulstretch.scd",
        "cutoff.scd",
        "limiter.scd",
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
    // Maybe break up sequences into timing and notes
    // need to test out instruments with different sequences to create them
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
ServerTree.add(~setupSubscription);
ServerTree.add(~setupOSC);
// ServerTree.add(~setupMIDI);

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
    s.sync;
    "Server booted".postln;


    /*
     * Maybe layer up different samples? Skip the synths for now?
     * Maybe key samples and play as notes?
     */

    // add some sample playback (through tape stretch, saturation, delay)


    /*
    * FX
    */
    // TODO maybe I can create a single reverb and run sends from synths to it and back?


    /*
    * Master FX
    */
    // Hard cutoff EQ for low/hi ends
    ~cutoffEq = Synth.head(~masterFxGroup, \cutoff, [
		\lpCutoff, ~bus[\lpCutoffBus].asMap,
		\hpCutoff, ~bus[\hpCutoffBus].asMap,
        \out, ~masterout
    ]);

    // EQ

    // Limiter to protect against any clipping
    ~limiter = Synth.after(~cutoffEq, \limiter, [
		\amp, ~bus[\limiterAmpBus].asMap,
		\dur, ~bus[\limiterDurBus].asMap,
        \out, ~masterout
    ]);

    // Final Master gain
    ~gainSynth = Synth.after(~limiter, \volumeCtl, [
		\amp, ~bus[\gainBus].asMap,
		\out, ~masterout
    ]);
});

s.doWhenBooted({
	~setTempo.value(~tempo);

	~setGain.value(~gain);
	~setCutoff.value(~hpCutoff, ~lpCutoff);
	~setLimiter.value(~limiterAmp, ~limiterDur);
	// ~osc.sendMsg('/done', "awake");
	"done".postln;
	//~mainGroup.set(\gate, 1);
});
)
