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
  localPort: 57121,

  // SuperCollider's port
  remoteAddress: "127.0.0.1",
  remotePort: 57120,
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
  util.log("Received message: " + value);
  switch (value[0]) {
    case "D":
      drums.arduinoIn(value);
      if (value[1] === 'T') {
        setTempo(value);
      }
      break;
    default:
      util.error("No matching handler for Arduino message " + value)
  }
}

function setTempo(value) {
  let tempo = Number(value.substr(2));
  if (isNaN(tempo)) {
    error("Tempo is NAN");
    return;
  }
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
  let status = drums.getStatus();
  if (status !== -1) {
    sendDrumStatus(status);
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
        value: hit
      }
    ]
  }
  udpPort.send(msg);
}

function sendDrumStatus(status) {
  let msg = {
    address: "/drum_status",
    args: [
      {
        type: "i",
        value: status.type
      },
      {
        type: "i",
        value: status.val
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
  let msg = {
    address: "/sample",
    args: [
      {
        type: "i",
        value: sample.track
      },
      {
        type: "i",
        value: sample.val
      }
    ]
  }
  udpPort.send(msg);
}

udpPort.on("message", function (oscMsg) {
  console.log(oscMsg.address + ": " + oscMsg.args[0].value);
  if (sp_connected && oscMsg.args[0].value === 1) {
    serialport.write("SC1");
  }
});

let metro = setInterval(beat, 150);

/*arduinoIn('DFB')
arduinoIn('DVD');
arduinoIn('DHJ'); // for testing*/
//arduinoIn('DVC');

//setTimeout(() => { arduinoIn('DDA') }, 20000);
//setTimeout(() => { arduinoIn('DHJ') }, 1000);
