const osc = require("osc");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");

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
    if (port.path.match(/COM[0-9]/)) {
      found = true;
      console.log("Opening port " + port.path);
      serialport = new SerialPort({ path: port.path, baudRate: 9600 });
      parser = new ReadlineParser()
      serialport.pipe(parser)
      parser.on('data', arduinoIn)
    }
  })
  if (!found) {
    console.error("No valid port found");
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
  if (!drums.DRUMS_ON || !hits.length) {
    return;
  }

  let msg = {
    address: "/drum_hit",
    args: []
  }

  for (let i = 0; i < hits.length; i++) {
    let arg = {
      type: "i",
      value: hits[i]
    }

    msg.args.push(arg);
  }

  console.log("sending hits for voices: " + hits);
  
  udpPort.send(msg);
}

let metro = setInterval(drumbeat, 500);