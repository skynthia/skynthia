s.boot;



~b1 = Buffer.read(s, "C:/Users/Autumn/Documents/skynthia/samples/ahh_female.wav");
~b2 = Buffer.read(s, "C:/Users/Autumn/Documents/skynthia/samples/ahh_chorus.wav");

(
SynthDef(\pb, {| out = 0, b1 = 0, b2 = 0, pitch = 0, gate = 0, noise = 1, noise2 = 0.5 |
	var sig, pb1, pb2, ln, chaos;
	ln = LFNoise2.ar(0.1, 0.5, 0.5);
	pb1 = PlayBuf.ar(2, b1,
		rate: BufRateScale.kr(b1) * pow(2, pitch/12),
		loop: 1,
		doneAction: Done.freeSelf);
	pb2 = PlayBuf.ar(2, b2,
		rate: BufRateScale.kr(b2) * pow(2, pitch/12),
		loop: 1,
		doneAction: Done.freeSelf);
	sig = Mix.new([ln*pb1, (1-ln)*pb2]);
	sig = GVerb.ar(sig, 100, 5);
	sig = sig * EnvGen.kr(Env.asr(3.0, 1.0, 5.0), gate);
	sig = Disintegrator.ar(sig, noise2, noise);
	sig = Compander.ar(sig, sig,
        thresh: 0.2,
        slopeBelow: 1,
        slopeAbove: 0.1,
        clampTime:  0.01,
        relaxTime:  0.01
    );
	Out.ar(out, sig*0.5);
}).add;
)
a = Synth(\pb, [\out, 0, \b1, ~b1, \b2, ~ b2]);
a.set(\gate, 1);
a.set(\gate, 0);
a.set(\pitch, 12);
a.set(\noise, -0.2);
a.free;


b = [27, 26, 27];
c = 0;
w = 0.2;
(
r = Routine {
	inf.do({
		a.set(\pitch, b[c % b.size]);
		c = c + 1;
		c.postln;
		w = 0.6 - w;
		w.wait;
	});
}.play;
)
r.stop;


s.quit;





























