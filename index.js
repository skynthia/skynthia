const osc = require("osc");
const fs = require("fs");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");

const outputLog = fs.createWriteStream("./outputLog.txt");
const errorsLog = fs.createWriteStream("./errorsLog.txt");
const logger = new console.Console(outputLog, errorsLog);

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
    if (port.path.match(/COM[0-9]+/)) {
      found = true;
      console.log("Opening port " + port.path);
      logger.log("Opening port " + port.path);
      serialport = new SerialPort({ path: port.path, baudRate: 9600 });
      parser = new ReadlineParser()
      serialport.pipe(parser)
      parser.on('data', arduinoIn)
    }
  })
  if (!found) {
    console.error("No valid port found");
    logger.error("No valid port found");
  }
});

function arduinoIn(value) {
  switch (value[0]) {
    case "d":
      drums.arduinoIn(value);
      break;
    default:
      console.error("No matching handler for Arduino message " + value)
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
  console.log("Sending " + a);
  udpPort.send(msg);
}

function drumbeat() {
  let hits = drums.getHits(clock % 16, clock % 64);
  clock++;
  // if drums are off or no hits
  if (!hits || !hits.length) {
    return;
  }

  // add imperfections -- randomize by up to 10ms
  for (let i = 0; i < hits.length; i++) {
    setTimeout(() => { sendDrumHit(hits[i]) }, Math.random() * 10);
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

let metro = setInterval(drumbeat, 150); // TODO: allow to configure tempo