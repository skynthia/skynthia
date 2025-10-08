
"Booting SC...".postln;
s.boot;

s.doWhenBooted({
  {
    (0..3).do {
      |i|
      // Need this to let me stagger the timing of initial code execution
      switch (i,
        0, {
          o = ServerOptions.new;
          o.memSize = 32768;

          f = { |msg, time, addr|
            //msg.postln;
            if (msg[0] == '/drum_hit') {
              //msg.postln;
              for(1, msg.size - 1) {
                | i |
                Synth(\drum, [\fx, ~drum_fx, \drum, ~drums[msg[i]]]);
              }
            };
            if (msg[0] == '/drum_status' && msg[1] == 1) {
              ~drum_fx = 3 + (msg[2] * 2);
              ~drum_fx.postln;
            };
            if (msg[0] == '/sample') {
              //msg.postln;
              Synth(\samp, [\sample, ~samples[msg[1]][msg[2]]]);
            };
          };
          thisProcess.addOSCRecvFunc(f);
          "Setup OSC receiver".postln;

          b = NetAddr.new("127.0.0.1", 57121);
        },

        1, {
          // ahh samples
          ~ahh1 = Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/ahh_female.wav");
          ~ahh2 = Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/ahh_chorus.wav");

          // drum samples
          ~drums = [
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/00_kick.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/01_longkick.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/02_lowstab.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/03_highstab.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/04_snare.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/05_openhat.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/06_lowoontz.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/07_highoontz.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/08_ding.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/09_clop.wav"),
            Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/drums/10_gong.wav")
          ];
          ~drum_fx = 3;

          ~samples = [
            [ // track 0
              Buffer.read(s, thisProcess.nowExecutingPath.dirname ++ "/samples/samples/00_backing.mp3")
            ]
          ];

          "Imported samples".postln;
        },

        2, {
          /* THIS IS AWESOME
          SynthDef("test2", {
            | out=0, amp=1 |
            var micinput = SoundIn.ar(0) * 10;
            var carrier =  PlayBuf.ar(2, ~ahh1);
            var sig1 = amp*Vocoder.ar(
              carrier, // synth (carrier)
              micinput, // mic (modulator)
              num:128, low:100, high:7500, q:0.03);
            var sig2 = HPF.ar(HPF.ar(micinput, 5000), 5000);
            Out.ar([0, 1],sig1+sig2);
          }).add;

          Synth("test2", [\amp, 3 /*careful!*/, \out, 0]); */

          SynthDef(\drum, {
            | fx, drum |
            var pb1;
            pb1 = PlayBuf.ar(2, drum, doneAction: Done.freeSelf)*0.5;

            Out.ar(fx, pb1);
          }).add;

          SynthDef(\fx1, {
            Out.ar(3, Decimator.ar(In.ar(5), 4000, 4));
          }).add;

          SynthDef(\fx2, {
            Out.ar(3, Disintegrator.ar(Decimator.ar(In.ar(7), 1500, 2), 0.9, 0.7));
          }).add;

          SynthDef(\drums_out, {
            Out.ar(0, Pan2.ar(In.ar(3), 0.1));
          }).add;

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

          SynthDef(\samp, {
            | sample |
            Out.ar(0, Limiter.ar(PlayBuf.ar(2, sample, doneAction: Done.freeSelf)));
          }).add;

          "Setup SynthDefs".postln;
        },

        3, {
          {
          (0..2).do {
            |i|
            switch (i,
              0, { ~drums_out = Synth(\drums_out); },
              1, { ~fx1 = Synth(\fx1); },
              2, { ~fx2 = Synth(\fx2); });
            0.125.wait;
          }
          }.fork();
          "Finished setting up SuperCollider".postln;
          b.sendMsg("/sc_fin", 1);
        }
      );
      // Wait a second between each step to give it time to process
      1.wait;
    }
  }.fork();
});

/*
Synth(\drum, [\fx, 3, \drum, ~drums[0]]);
Synth(\samp, [\sample, ~samples[0][0]]);

~fx1.free;
~fx1 = Synth(\fx1);

~fx2.free;
~fx2 = Synth(\fx2);

~drums_out.free;
~drums_out = Synth(\drums_out);
Synth(\drum, [\fx, 3, \drum, ~drums[0]]);

Synth(\samp, [\sample, ~samples[0]]);


a = SinOsc.ar(440, 0, 0.1);
{a}.play;
a.free;
{ SinOsc.ar(200, 0, 0.5) }.play;


thisProcess.removeOSCRecvFunc(f);
// s.quit;











a = Synth(\ahh, [\out, 0, \b1, ~ahh1, \b2, ~ahh2]);
a.set(\gate, 1);
a.set(\gate, 0);
a.set(\noise, 1);
a.free;

~tones = [ 0, 2, 3, 5, 7, 8, 10, 12 ];

(
{
  (0..4).do {
    |i|
    a.set(\pitch, ~tones[i * 2]);
    1.wait;
  }
}.fork();

)


*/




