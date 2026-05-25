s.boot;

(
SynthDef(\audio_out, {
  Out.ar(0, In.ar(3));
}).add;

SynthDef(\basic_sig, {
  | out_chan = 3 |
  var sig = Impulse.ar(1);// SinOsc.ar(freq: 440, mul: 0.1);
  Out.ar(out_chan, sig);
}).add;

SynthDef(\fx1, {
  var sig, fx;
  sig = In.ar(4);
  fx = FreeVerb.ar(sig);
  Out.ar(3, fx);
}).add;
)

a = Synth(\audio_out);
b = Synth(\basic_sig);
c = Synth(\fx1);
b.set(\out_chan, 4);
b.free;
