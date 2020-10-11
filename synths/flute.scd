(
SynthDef(\flute, {
	arg: scl = 0.2, freq = 440, ipress = 0.9, ibreath = 0.09, ifeedbk1, 0.4, ifeedback2 = 0.4, dur = 1, gate = 1, amp = 2;
	var: kenv1, kenv2, kvibr, sr, cr, block;
	var poly, signalOut, ifqc;
	var aflow, asum1, asum2, afqc, atemp1, ax, apoly, asum3, avalue, atemp2, aflute1;

});
)