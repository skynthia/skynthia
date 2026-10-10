const os = require('node:os');
const osc = require("osc");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");
const sampler = require("./sampler");
const melody = require("./melody");
const util = require("./util");
const trackconfig = require("./trackconfig.json");

let clock = 0;
let track = -1;
let swung = false;
let long_beat = true;
let samples_to_date = 0;

const udpPort = new osc.UDPPort({
  // My port
  localAddress: "127.0.0.1",
  localPort: 667,

  // Ableton Live's port
  remoteAddress: "127.0.0.1",
  remotePort: 666,
  metadata: true
});

udpPort.open();

let serialport;
let sp_connected = false;
let parser;

SerialPort.list().then(function(ports){
  ports.forEach(port => {
    // correct format for serial ports on Windows
    if (port.path.match(
        os.platform() === "linux" ? 
          /\/dev\/ttyACM[0-9]+/ :
          /COM[0-9]+/
      ) && !sp_connected) {
      sp_connected = true;
      util.log("Opening port " + port.path);
      serialport = new SerialPort({ path: port.path, baudRate: 9600 });
      parser = new ReadlineParser();
      serialport.pipe(parser);
      parser.on('data', arduinoIn);
    }
  })
  if (!sp_connected) {
    util.error("No valid port found");
    //process.exit(1);
  }
});

function arduinoIn(value) {
  switch (value[0]) {
    case "D":
      util.log("Received message from Doig: " + value);
      if (value[1] === "T") {
        setTempo(value);
      }
      else {
        drums.arduinoIn(value);
      }
      break;
    case "P":
      util.log("Received message from Samplerella: " + value);
      sampler.arduinoIn(value);
      value = value.substr(0, 3);
      switch (value) {
        case "PDP":
          drums.hardStartStop(false);
          break;
        case "PDU":
          drums.hardStartStop(true);
          clock = 0;
          break;
        case "PNT":
          nextTrack();
          break;
      }
      break;
    default:
      util.log("Received message: " + value);
      break;
      //util.error("No matching handler for Arduino message " + value)
  }
}

function nextTrack() {
  track++;
  if (track < trackconfig.length) {
    let nt = trackconfig[track];
    util.log("Playing track " + nt.title);
    swung = nt.swung;
    sendOneInt("/tempo", nt.tempo);
    if (swung) {
      tempo = (60 / nt.tempo) * 666.67;
      long_beat = true;
    }
    else {
      tempo = (60 / nt.tempo) * 500;
    }

    if (track > 0) {
      samples_to_date += nt.num_samples;
    }
  }
  else {
    util.log("Track " + track + " not found; manual config only");
  }

  sendTrack(track);
}

function setTempo(value) {
  let tempo = Number(value.substr(2));
  if (isNaN(tempo)) {
    error("Tempo is NAN");
    return;
  }
  else if (tempo < 50 || tempo > 5000) {
    error("tempo too low/high, I don't believe this");
    return;
  }
  tempo = Math.round(tempo / 4);
  clearInterval(metro);
  metro = setInterval(beat, tempo);
}

function beat() {
  drumbeat();
  melodybeat();
  samplebeat();
  if (swung) {
    setTimeout(beat, long_beat ? tempo / 2 : tempo);
    long_beat = !long_beat;
  }
  else {
    setTimeout(beat, tempo);
  }
}

function drumbeat() {
  let effects = drums.getEffects();
  if (effects !== -1) {
    sendDrumEffects(effects);
  }

  // If we're starting over, restart the clock
  if (drums.getDrumsOn() === 2) {
    clock = 0;
  }

  // If we're off, just return
  if (!drums.getDrumsOn()) {
    return;
  }

  // Otherwise calculate hits
  let hits = drums.getHits(clock % 16, clock % 64);
  clock++;
  // if drums are off or no hits
  if (!hits || !hits.length) {
    return;
  }

  // add imperfections -- randomize by up to 4ms
  for (let i = 0; i < hits.length; i++) {
    setTimeout(() => { sendDrumHit(hits[i]) }, Math.random() * 4);
  }
}

function melodybeat() {
  let note = melody.getNote();
  if (note !== -1) {
    sendNote(note);
  }
}

function samplebeat() {
  let sample = sampler.getSample();
  if (sample !== -1 && sample < trackconfig[track].num_samples) {
    sendSample(sample);
  }

  if (clock % 64 == 0) {
    let clip = sampler.getClip();
    if (clip !== -1) {
      sendClip(clip);
    }
  }

  let effects = sampler.getEffects();
  if (effects !== -1) {
    sendVocalEffects(effects);
  }
}

function sendTrack(track) {
  sendOneInt("/track", track);
}

function sendDrumHit(hit) {
  sendOneInt("/drum_hit", hit + 60);
}

function sendDrumEffects(effects) {
  sendOneInt("/drum_effects", effects);
}
function sendVocalEffects(effects) {
  sendOneInt("/vocal_effects", effects);
}

function sendNote(note) {
  sendOneInt("/note", note);
}

function sendSample(sample) {
  if (sample > 60) {
    log("Attempted to send sample with value > 60");
    return;
  }

  sendOneInt("/sample", samples_to_date + sample);
}

function sendClip(clip) {
  sendOneInt("/clip", clip);
}

function sendOneInt(path, val) {
  let msg = {
    address: path,
    args: [
      {
        type: "i",
        value: val
      }
    ]
  }
  udpPort.send(msg);
}

udpPort.on("message", function (oscMsg) {
  console.log(oscMsg.address + ": " + oscMsg.args[0].value);
  if (sp_connected && oscMsg.args[0].value === 1) {
    serialport.write("connected\n");
  }
});

nextTrack();
beat();

/*
arduinoIn('DVD')
arduinoIn('DHG')
setTimeout(() => {
  arduinoIn('PDP')
}, 3000);
setTimeout(() => {
  arduinoIn('PDU')
}, 6000);*/
//arduinoIn('PCA')
//setInterval(() => {arduinoIn('PSA')}, 20000);

/*
//sendSample(0);
setTimeout(() => {
  arduinoIn('DFB')
  setTimeout(() => { arduinoIn('DFC') }, 5000);
  setTimeout(() => { arduinoIn('DFD') }, 10000);
  setTimeout(() => { arduinoIn('DFA') }, 15000);
  arduinoIn('DVD');
  arduinoIn('DHF'); // for testing
}, 15000);
//arduinoIn('DVC');

//setTimeout(() => { arduinoIn('DDA') }, 20000);
//setTimeout(() => { arduinoIn('DHJ') }, 1000);*/
