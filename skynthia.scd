
(
a = {
	var sig;
	sig = SinOsc.ar([220, 880], mul: 0.05);
	Out.ar(0, sig);
}.play;
)
a.free;


~b1 = Buffer.read(s, "C:/Users/AUTUM/Documents/Skynthia/ahh_samples/processed/ahh_female.wav");
~b2 = Buffer.read(s, "C:/Users/AUTUM/Documents/Skynthia/ahh_samples/processed/ahh_chorus.wav");
(
SynthDef(\pb, {| out = 0, b1 = 0, b2 = 0, pitch = 0, trig = 0 |
	var sig, pb1, pb2, ln;
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
  Out.ar(out, sig*0.5);
}).add;
)
a = Synth(\pb, [\out, 0, \b1, ~b1, \b2, ~b2]);
a.set(\pitch, 0);
a.free;

(
{ SinOsc.ar(
        LFNoise1.ar(4, 400, 450),
        0, 0.2
    )
}.play;
)

pow(2, 0);

{ var freq = 1000; [LFNoise0.ar(freq), Impulse.ar(freq)] }.plot
(BufFrames.kr(~b1)/2).postln

Project