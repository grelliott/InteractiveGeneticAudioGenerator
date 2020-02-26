//1. Server config
(
s.options.numOutputBusChannels_(1);
s.options.numInputBusChannels_(0);
s.options.sampleRate_(48000);
s.options.memSize_(2.pow(12));  // Or 2.pow(20)
s.options.maxLogins = 2;
s.newBusAllocators;
ServerBoot.removeAll;
ServerTree.removeAll;
ServerQuit.removeAll;

// Set tuning and pitch. These probably shouldn't change
// Pythagorean / 432?

//2. Global variables
~out = 0;  // lowest index of main out
~path = PathName(thisProcess.nowExecutingPath).parentPath++"samples/";

//3. Piece specific functions
~makeBuffers = {
	b = Dictionary.new;
	PathName(~path).entries.do{
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

~makeBusses = {
	~bus = Dictionary.new;
	~bus.add(\reverbBus -> Bus.audio(s, 2));
};

// Set up a few key groups
~makeNodes = {
	s.bind({
		~mainGrp = Group.new;
		~reverbGrp = Group.after(~mainGrp);
		~reverbSynth = Synth.new(\reverb, [
			\amp, 1,
			\predelay, 0.1,
			\revtime, 1.8,
			\lpf, 4500,
			\mix, 0.35,
			\in, ~bus[\reverbBus],
			\out, ~out,
		],
		~reverbGrp
		);
	});
};

~makeEvents = {
	e = Dictionary.new;

	e.add(\event1 -> {
		Pbind(
			\instrument, \metronome,
			\rel, 0.1,
			\dur, 1/2,
			\stretch, TempoClock.tempo,
			\gate, 1,
			\group, ~mainGrp,
			\out, ~out,
		).play(quant: 15) ;
	});
	e.add(\event2 -> {"Event 1".postln;});
	e.add(\event3 -> {"Event 1".postln;});

	// Use quant to time entry/exit of sequences to bars/beats of tempo
};

~setupOSC = {
	OSCdef.new(\notifyOSC, {
		arg msg, time, addr, port;
		[msg, addr].postln;
		if (msg[1] == 1) {
			e[\event1].value;
			~reverbGrp.set(\gate, 1);
			~mainGroup.set(\gate, 1);
			addr.sendMsg("/done", "awake");
		}
	}, \notify
	);

	OSCdef.new(\tempoOSC, {
		arg msg, time, addr, port;
		msg.postln;
		// Set tempo
		TempoClock.tempo = msg[1];
		e[\event1].value;
		//~reverbGrp.set(\gate, 1);
		~mainGroup.set(\gate, 1);
	},
	'/gene/tempo'
	);

	OSCdef.new(\scaleOSC, {
		arg msg, time, addr, port;
		msg.postln;
		// Set scale
		//TempoClock.tempo = msg[1];
	},
	'/gene/scale'
	);
};

~cleanup = {
	s.newBusAllocators;
	ServerBoot.removeAll;
	ServerTree.removeAll;
	ServerQuit.removeAll;
};

//4.  Register functions with ServerBoot/Quit/Tree
ServerBoot.add(~makeBuffers);
ServerBoot.add(~makeBusses);
ServerQuit.add(~cleanup);

//5. Boot server
s.waitForBoot({
	// wait for previous server actions to complete
	s.sync;
	// add synth defs
	SynthDef(\metronome, {
		arg out=~out, freq=440, amp=1, atk=0.01, dec=1, sus=0.1, rel=0.1, gate=1;
		var sig, env;
		sig = SinOsc.ar(freq, 0);
		env = EnvGen.kr(Env.adsr(atk, dec, sus, rel), gate, doneAction:2);
		sig = sig * env * amp;
		Out.ar(out, sig);
	}).add;

	// add delay
	// add limiter
	SynthDef(\reverb, {
		arg in=~bus[\reverbBus], predelay=0.1, revtime=1.8,
		lpf=4500, mix=0.15, amp=1, out=~out;
		var wet, dry, temp, sig;
	 	dry = In.ar(in, 2);
	 	temp = In.ar(in, 2);
	 	wet = 0;
	 	temp = DelayN.ar(temp, 0.2, predelay);
	 	16.do{
	 		temp = AllpassN.ar(temp, 0.05, {Rand(0.001, 0.05)}!2, revtime);
	 		temp = LPF.ar(temp, lpf);
	 		wet = wet + temp;
	 	};
	 	sig = XFade2(dry, wet, mix*2-1, amp);
	 	Out.ar(out, sig);
	}).add;

	s.sync;
	ServerTree.add(~makeNodes);
	ServerTree.add(~makeEvents);
	ServerTree.add(~setupOSC);
	s.freeAll;
	s.sync;

	"done".postln;
    "Listening on ".post;
    NetAddr.localAddr.post;
    ":".post;
    NetAddr.langPort.postln
});
)
//
// (
// ~mainGrp.set(\gate, 1);
// ~reverbGrp.set(\gate, 1);
// e[\event1].value;
// )