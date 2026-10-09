const util = require("./util");

let sample = -1;
let clip = -1;
let effects = -1;

function arduinoIn(value) {
  let num_val = value.charCodeAt(2) - 65; // get the int
  switch (value[1]) {
    case 'S':
      setSample(num_val);
      break;
    case 'C':
      setClip(num_val);
      break;
    case 'V':
      setEffects(num_val);
      break;
  }
}

function setSample(value) {
  sample = value;
}

function getSample() {
  let temp = sample;
  sample = -1;
  return temp;
}

function setClip(value) {
  clip = value;
}

function getClip() {
  let temp = clip;
  clip = -1;
  return temp;
}

function setEffects(value) {
  effects = value;
}

function getEffects() {
  let temp = effects;
  effects = -1;
  return temp;
}

module.exports = { arduinoIn, getSample, getClip, getEffects };
