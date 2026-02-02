const os = require('node:os');
const osc = require("osc");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");
const melody = require("./melody");
const util = require("./util");

let clock = 0;

const udpPort = new osc.UDPPort({
  // My port
  localAddress: "127.0.0.1",
  localPort: 667,

  // SuperCollider's port
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
      drums.arduinoIn(value);
      if (value[1] === 'T') {
        setTempo(value);
      }
      break;
    case "P":
      break;
    default:
      util.log("Received message: " + value);
      break;
      //util.error("No matching handler for Arduino message " + value)
  }
}

function setTempo(value) {
  let tempo = Number(value.substr(2));
  if (isNaN(tempo)) {
    error("Tempo is NAN");
    return;
  }
  else if (tempo < 50) {
    error("tempo too low, I don't believe this");
    return;
  }
  tempo = Math.round(tempo / 4);
  clearInterval(metro);
  metro = setInterval(beat, tempo);
}

function sendToSC(a) {
  let r = Math.random();
  let msg = {
    address: "/test",
    args: [{
      "type": "i",
      "value": a
    }]
  }
  util.log("Sending " + a);
  udpPort.send(msg);
}


function beat() {
  drumbeat();
  melodybeat();
  samplebeat();
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
  let sample = drums.getSample();
  if (sample !== -1) {
    sendSample(sample);
  }
}

function sendDrumHit(hit) {
  let msg = {
    address: "/drum_hit",
    args: [
      {
        type: "i",
        value: hit + 60
      }
    ]
  }
  udpPort.send(msg);
}

function sendDrumEffects(effects) {
  let msg = {
    address: "/drum_effects",
    args: [
      {
        type: "i",
        value: effects
      }
    ]
  }
  udpPort.send(msg);

}

function sendNote(note) {
  let msg = {
    address: "/note",
    args: [
      {
        type: "i",
        value: note
      }
    ]
  }
  udpPort.send(msg)
}

function sendSample(sample) {
  if (sample > 60) {
    log("Attempted to send sample with value > 60");
    return;
  }

  let msg = {
    address: "/sample",
    args: [
      /*{
        type: "i",
        value: sample.track
      },*/
      {
        type: "i",
        value: sample
      }
    ]
  }
  udpPort.send(msg);
}

udpPort.on("message", function (oscMsg) {
  console.log(oscMsg.address + ": " + oscMsg.args[0].value);
  if (sp_connected && oscMsg.args[0].value === 1) {
    serialport.write("SC1\n");
  }
});

let metro = setInterval(beat, 180);

/*setTimeout(() => { 
  serialport.write("SC1\n", function(err) {
  if (err) {
    return console.log('Error on write: ', err.message)
  }
  console.log('message written')
})

}, 2000);

arduinoIn('DBD')
arduinoIn('DVF')
arduinoIn('DHG')

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
