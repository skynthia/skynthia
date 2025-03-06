const fs = require("fs");

const outputLog = fs.createWriteStream("./outputLog.txt");
const errorsLog = fs.createWriteStream("./errorsLog.txt");
const logger = new console.Console(outputLog, errorsLog);

function log(msg) {
  console.log(msg);
  logger.log(msg);
}

function error(msg) {
  console.error(msg);
  logger.error(msg);
}

module.exports = { log, error };