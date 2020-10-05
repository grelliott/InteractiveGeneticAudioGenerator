(
"".postln;
"Restarting server".postln;
s.quit;

~basePath = "/home/pi/audiogene/audio/";

// Server config
o = ServerOptions.new;

// RaspberryPi 4 with HifiBerry DAC+ Pro
o.numOutputBusChannels = 2;
o.numInputBusChannels = 0;
o.sampleRate = 48000;
//o.hardwareBufferSize = 16; //2.pow(12);  // TODO determine correct size for this
o.blockSize = 64;

o.memSize = 2.pow(20);  // Or 2.pow(20)
o.bindAddress = "0.0.0.0";
o.maxLogins = 4;

s = Server.local;
s.options = o;

//s.dumpOSC();

/*
 * Set up configurable parameters
 */
~gain = 6.0;  // in dB
~maxGain = 6.0;
~lpCutoff = 18000;
~hpCutoff = 80;
~limiterAmp = 1.0;
~limiterDur = 0.02;
~tempo = 120;
~tempoMin = 60;
~tempoMax = 180;
~beatsBetweenRequests = 64;

~t = TempoClock.default;

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
    ~bus[\gainBus].set(~gain.dbamp);

	~bus.add(\lpCutoffBus -> Bus.control(s));
	~bus.add(\hpCutoffBus -> Bus.control(s));
	~bus.add(\limiterAmpBus -> Bus.control(s));
	~bus.add(\limiterDurBus -> Bus.control(s));

    // Set up busses for controlling behaviour of audio and synths
    ~bus.add(\rhythmBus -> Bus.control(s));
    ~bus.add(\energyBus -> Bus.control(s));
    ~bus.add(\vibeBus -> Bus.control(s));
    ~bus.add(\themeBus -> Bus.control(s));
};


// Set up a few key groups
// These allow for ordering of signals,
// and for sending messages to multiple related synths at once
~makeNodes = {
    ~mainGroup = Group.head;
    ~padsGroup = Group.new(~mainGroup, \addToHead);
    // add groups for each instrument group
    ~masterFxGroup = Group.tail;
};

// Set up the functions that will control the behaviour of the system
~addRoutines = {
    ~setGain = { |db|
        if( db > ~maxGain, {
            db.post;
            "db is greater than the max ".post;
            ~maxGain.post;
            "db".postln;
            db = ~maxGain;
            nil;
        },
        {
            "Setting gain to ".post;
            db.post;
            "db".postln;
            ~gain = db;
            ~bus[\gainBus].set(~gain.dbamp);
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
        ~t.schedAbs(~t.nextBar, {
            "Setting scale to ".post;
            ~scales.clipAt(scale).name.postln;
            ~scale = ~scales.clipAt(scale);
        });
    };

    // rythymBus carries a signal that indicates
    // how much rythym should be in the piece (arhythmic to 4/4 to polyrhythm...?)
    // Expect value from 0-6
    ~setRythm = { |rhythm|
        "Setting rhythm to ".post;
        rhythm.postln;
        // perhaps the bus can be used to select the pdefs/pseqs
        ~bus[\rhythmBus].set(rhythm);
    };

    // Energy bus may control the kind of energy the piece has
    // Perhaps going from a calm relaxed energy
    // to an upbeat energetic feeling
    // Lower energy may be low tempo, maybe less instruments, more open arrangments, less structured rythm
    // higher energy may be higher tempo, more instruments, more structured rythm
    ~setEnergy = { |energy|
        "Setting energy to ".post;
        energy.postln;
        // set the tempo based on the value of energy
        // Assume it falls between 0-255
        // and tempo falls between tempoMin and tempoMax
        ~setTempo.value(energy.linlin(0, 255, ~tempoMin, ~tempoMax));
        // input to set rhythm
        // for now only use energy to set rhythm
        ~setRhythm.value(energy.linlin(0, 255, 0, 6).round);
        ~bus[\energyBus].set(energy);
    };

    // vibeBus (name tbd) could toggle things like
    // scale, or sample selection
    // Perhaps moving from a dark moody vibe to a upbeat happy vibe
    // A dark moody vibe may use a range of scales such as Aeolian, harmonic minor, melodic minor, locrian
    // upbeat may use scales such as major, mixolydian, lydian, phrygian, harmonic major, dorian (may be on border)
    // Dark may use drawn out simple chords
    // upbeat may use snappier chords, maybe a bit more complex or at least arranged to feel happy
    ~setVibe = { |vibe|
        "Setting vibe to ".post;
        vibe.postln;
        // for now set scale only from vibe
        //TODO add a function to take inputs from multiple sources to determine scale
        ~setScale.value(vibe.round);
        // a dark moody vibe may use minimal drones, thick reverb, maybe some compression?
        // a dark moody vibe may shift octaves of certain instruments down
        // a dark moody vibe may increase the duration of notes
        // moodier vibe may use more stretched out samples
        ~bus[\vibeBus].set(vibe);
    };

    // Theme bus may change from a natural organic theme to a digital techy theme
    // organic theme may contain field-recordings played out very close to reality
    //   perhaps nature-based instruments (sticks, wooden pipes, etc)
    // digital theme may contain synths, perhaps with heavy effects
    // in-between maybe something closer to instruments?
    ~setTheme = { |theme|
        "Setting theme to ".post;
        theme.postln;
        // select instruments
        // organic to digital
        //TODO determine which instruments/groups should be active
        //0, 1, 2, 3, 4, 5, 6
        // let's just start with adding one and go from there
        //TODO pick instrument at random from grouping
        // pick degree/dur randomly based on vibe/energy bus values

        //infinitSound = getInfiniteSound("nature");
        // startInfiniteSound(\thunderstorm, \out, reverb)
        ~startInfiniteSound.value(\thunderstorm, [\out, ~masterout, \amp, 0.85]);
        //stopInfiniteSounds
        //stopInifiniteSound(\thunderstorm)
        ~loadInstrument.value(\padsPbind, \voice, ~padsGroup, \amp, 1, \degree, Pseq([1,2,3,4,5,6,7], inf), \dur, Prand([4, 2, 1/4], inf));
        //unloadInstrument
        ~padsGroup.set(\gate, 1);
        ~bus[\themeBus].set(theme);
    };
};

~startInfiniteSound = { |name...args|
    var synthKey = name.asString++"Synth";
    if (~runningSynths.includesKey(synthKey), { "Already have synth ".post; synthKey.postln; },  {
        var synth;
        "Adding ".post; synthKey.postln;
        synth = Synth.new(name, *args);
        ~runningSynths.add(synthKey -> synth);
    });
};

~replaceInfiniteSound = {|endSynthName, startSynthName...args|
    var endSynthKey, startSynthKey;
    endSynthKey = endSynthName++"Synth";
    startSynthKey = startSynthName++"Synth";

    // ramp down endSynth
    if (~runningSynths.includesKey(endSynthKey).neg, Nil, {
        var endSynth = ~runningSynths.at(endSynthName++"Synth");
        endSynth.set(\amp, Line.kr(endSynth.get(\amp), 0, 4)); //TODO get time for 1 bar
    });

    // ramp up start startSynth

    ~startSynth.value(startSynthName, args);
};

~stopInfiniteSound = { |name|
    var synthKey = name.asString++"Synth";
    if (~runningSynths.includesKey(synthKey).neg, { "Missing synth ".post; synthKey.postln; },  {
        "Stoping and removing synth ".post; synthKey.postln;
        ~runningSynths[synthKey].run(false);
        ~runningSynths[synthKey].free;
        ~runningSynths.removeAt(synthKey);
    });
};

~loadInstrument = { |pbind, name, group ... synthArgs|
    // perhaps eventually unload non-playing synthdefs? Not sure if I'll need to manage memory like that
    // would use the directory to find and load synth. For now, just use the name-as-symbol to create a synth
    // actually set up a pbindef with a gate
    // or, rather...perhaps just swaps the instrument of an existing pbindef
    if (Pbindef(pbind).get(\instrument) == name, nil, {
        Pbindef(pbind, \instrument, name, \out, ~masterout, \group, group, * synthArgs).play;
    });
};

// Set up the OSC messages we listen for
~setupOSC = {

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

    OSCdef.new(\connectedOSC, { |msg, time, addr, port|
        Synth(\playSample2, [\out, ~masterout, \sample, ~getBuffer.value("control", "connected")]);
        ~t.schedAbs(4, {
            addr.sendMsg("/request", "s");
            ~beatsBetweenRequests;
        });
        ~mainGroup.set(\gate, 1);
    },
    '/connected',
    );

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
        //TODO mayyyybe just addr?
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

// Set up a subscription to broadcast certain OSC messages to
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
// Used for testing only
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
            subfolder.entries.do { |file|
               ~b[subfolder.folderName.asSymbol].put(file.fileNameWithoutExtension.asSymbol, Buffer.read(s, file.fullPath));
            };
        };
    }.value;
    ~getBuffer = { |folder, file|
        ~b.at(folder.asSymbol).at(file.asSymbol);
    };
};


~loadSynths = {
    ~runningSynths = Dictionary.new();
    ~loadFiles.value(~basePath++"synths/", [
        "kick.scd",
		"apad_mh.scd",
        "synthbass.scd",
        "ambientpad.scd",
        "voice.scd",
        "thunderstorm.scd"
    ]);
};

~makeSynths = {
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
};

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
// Will need to figure out how to create some melody/harmony between the playing instruments
//
~makeSequences = {
    // Make a bunch of Pbind-style objects that can be used to describe how an instrument should be played
    // should describe the notes being played, the order of the notes, and the duration of the notes (relative to global tempo and scale)
    ~sequences = Dictionary.new;
    // totally random
    ~sequences[\random] = Dictionary.new;
    // A bit of form, but mostly random. May come back to repeat a phrase
    ~sequences[\chaotic] = Dictionary.new;
    // inspired by jazz
    ~sequences[\improvised] = Dictionary.new;
//    ~sequences[\improvised][\thelick] = Pseq(#[62,64,65,67,64,Rest(1),60,62], inf);
    // polyrhythimc composition
    ~sequences[\poly] = Dictionary.new;
    // nearly on-beat, but with some swing, or off-beat phrases
    ~sequences[\synchopated] = Dictionary.new;
    // straight-up 4/4
    ~sequences[\beats] = Dictionary.new;

    // Maybe break up sequences into timing and notes
    // need to test out instruments with different sequences to create them
};

~makePbinds = {
    Pbindef.new(\padsPbind, \instrument, \default).clear;
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

ServerTree.add(~loadBuffers);
ServerTree.add(~loadSynths);
ServerTree.add(~loadFx);
ServerTree.add(~makeSynths);

ServerBoot.add(~makeSequences);
ServerBoot.add(~makePbinds);
//ServerBoot.add(~makeEvents);

ServerQuit.add(~cleanup);

s.waitForBoot({
    s.sync;
    "Server booted".postln;

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
	~setGain.value(~gain);
	~setLimiter.value(~limiterAmp, ~limiterDur);
	~setCutoff.value(~hpCutoff, ~lpCutoff);

    Synth(\playSample2, [\out, ~masterout, \sample, ~getBuffer.value("control", "startup")]);
	"Ready".postln;
});

)
