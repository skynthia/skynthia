const osc = require("osc");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");
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
let parser;

SerialPort.list().then(function(ports){
  let found = false;
  ports.forEach(port => {
    // correct format for serial ports on Windows
    if (port.path.match(/COM[0-9]+/) && !found) {
      found = true;
      util.log("Opening port " + port.path);
      serialport = new SerialPort({ path: port.path, baudRate: 9600 });
      parser = new ReadlineParser();
      serialport.pipe(parser);
      parser.on('data', arduinoIn);
    }
  })
  if (!found) {
    util.error("No valid port found");
    //process.exit(1);
  }
});

function arduinoIn(value) {
  util.log("Received message: " + value);
  switch (value[0]) {
    case "D":
      drums.arduinoIn(value);
      break;
    default:
      util.error("No matching handler for Arduino message " + value)
  }
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

function drumbeat() {
  // If we're starting over, restart the clock
  if (drums.getDrumsOn() === 2) {
    clock = 0;
  }

  // If we're off, just return
  if (!drums.getDrumsOn()) {
    return;
  }

  // Otherwise calculate hits
  if (clock % 64 === 0) {
    let status = drums.getStatus();
    if (status !== -1) {
      sendDrumStatus(status);
    }
  }
  
  let hits = drums.getHits(clock % 16, clock % 64);
  clock++;
  // if drums are off or no hits
  if (!hits || !hits.length) {
    return;
  }

  // add imperfections -- randomize by up to 8ms
  for (let i = 0; i < hits.length; i++) {
    setTimeout(() => { sendDrumHit(hits[i]) }, Math.random() * 8);
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

let metro = setInterval(drumbeat, 150); // TODO: allow to configure tempo

arduinoIn('DFB')
arduinoIn('DVD');
arduinoIn('DHJ'); // for testing

setTimeout(() => { arduinoIn('DFA') }, 10000);
