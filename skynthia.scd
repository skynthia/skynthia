s.boot;

(
// ahh samples
~ahh1 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/ahh_female.wav");
~ahh2 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/ahh_chorus.wav");

// drum samples
~d00 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/00_kick.wav");
~d01 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/01_longkick.wav");
~d02 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/02_lowstab.wav");
~d03 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/03_highstab.wav");
~d04 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/04_snare.wav");
~d05 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/05_openhat.wav");
~d06 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/06_lowoontz.wav");
~d07 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/07_highoontz.wav");
~d08 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/08_ding.wav");
~d09 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/09_clop.wav");
~d10 = Buffer.read(s, thisProcess.nowExecutingPath.dirname +/+ "samples/drums/10_gong.wav");
~drums = [~d00, ~d01, ~d02, ~d03, ~d04, ~d05, ~d06, ~d07, ~d08, ~d09, ~d10];

~drum_chan = 3;
)

(
f = { |msg, time, addr|
	if (msg[0] == '/drum_hit') {
    //msg.postln;
    for(1, msg.size - 1) {
      | i |
      Synth(\drum, [\out, ~drum_chan, \drum, ~drums[msg[i]]]);
    }
	};
  if (msg[0] == '/drum_status') {
    ~drum_chan = 3 + msg[2];
    ~drum_chan.postln;
  };
};
thisProcess.addOSCRecvFunc(f);
);
thisProcess.removeOSCRecvFunc(f);

(
SynthDef(\ahh, {| out = 0, b1 = 0, b2 = 0, pitch = 0, gate = 0, noise = 1, noise2 = 0.5 |
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

SynthDef(\drum, {
  | out, drum |
  var pb1;
  pb1 = PlayBuf.ar(2, drum, doneAction: Done.freeSelf);
  Out.ar(out, pb1);
}).add;

SynthDef(\fx1, {
  var in, fx;
  in = In.ar(5);
  fx = Decimator.ar(in, 4000, 3);
  Out.ar(3, fx);
}).add;

SynthDef(\drum_out, {
  Out.ar(0, Pan2.ar(In.ar(3) * 0.2));
}).add;

)

~fx1 = Synth(\fx1);
~fx1.free;
~drum_out = Synth(\drum_out);
~drum_out.free;


Synth(\drum, [\out, 3, \drum, ~drums[2]]);

a = Synth(\ahh, [\out, 0, \b1, ~ahh1, \b2, ~ahh2]);
a.set(\gate, 1);
a.set(\gate, 0);
a.set(\pitch, 12);
a.set(\noise, -0.2);
a.free;

a = SinOsc.ar(440, 0, 0.1);
{a}.play;
a.free;
{ SinOsc.ar(200, 0, 0.5) }.play;


s.quit;

























