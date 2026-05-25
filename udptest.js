const osc = require("osc");

const udpPort = new osc.UDPPort({
  // My port
  localAddress: "127.0.0.1",
  localPort: 666,

  // SuperCollider's port
  remoteAddress: "127.0.0.1",
  remotePort: 667,
  metadata: true
});

udpPort.open();

function sendTestMsg() {
  let msg = {
    address: "/test",
    args: [
      {
        type: "i",
        value: Math.floor(Math.random() * 10)
      }
    ]
  }
  udpPort.send(msg);
}

setInterval(sendTestMsg, 1000);
