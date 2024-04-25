const osc = require("osc");
const { SerialPort } = require("serialport");

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

//const serialport = new SerialPort({ path: 'COM3', baudRate: 9600 });
//console.log(SerialPort.list());

function sendToSC() {
  let r = Math.random();
  let msg = {
    address: "/test",
    args: [{
      "type": "i",
      "value": 0
    }, {
      "type": "f",
      "value": r
    }]
  }
  console.log("Sending " + r);
  udpPort.send(msg);
}

let clock = setInterval(sendToSC, 500);