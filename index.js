const osc = require("osc");
const { SerialPort, ReadlineParser } = require("serialport");
const drums = require("./drums");

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
      parser.on('data', sendToSC)
    }
  })
  if (!found) {
    console.error("No valid port found");
  }
});

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

//let clock = setInterval(sendToSC, 500);