const drumconfig = require("./drumconfig.json");

let DRUMS_ON = true;

let density_of_hits = 6;
let density_of_voices = 1;
let dynamism = 0.5;
let root_voice = 0;
let voices = [0];

function arduinoIn(value) {

}

function newPattern() {
  for (let i = 0; i < voices.length; i++) {
    let voice = voices[i];
    generateHits(voice);
  }
}

function generateHits(voice) {
  let drum = drumconfig[voice];
  let probs = drum.probs.slice(); // duplicate array
  drum.hits = new Array(16).fill(false);
  
  // randomly modify number of hits based on dynamism
  let modifier = Math.round(density_of_hits * (Math.random() * dynamism));
  modifier = Math.random() > 0.5 ? modifier : -1 * modifier; // randomly + or -
  let num_hits = density_of_hits + modifier;
  num_hits = num_hits < 1 ? 1 : num_hits; // never less than 1
  num_hits = num_hits > drum.max ? drum.max : num_hits; // never more than 16

  for (let i = 0; i < num_hits; i++) {
    for (let j = probs.length - 1; j >= 0; j--) {
      let rand = Math.random();
      if (probs[j].length && rand < j/10) {
        let idx = Math.floor(Math.random() * probs[j].length);
        let hit = probs[j][idx];
        drum.hits[hit] = true;
        probs[j].splice(idx, 1); // remove the hit we just generated
        break;
      }
    }
  }
  //console.log(drum.hits);
}

// beat is 0-15
function getHits(beat) {
  let hits = [];
  for (let i = 0; i < voices.length; i++) {
    let voice = voices[i];
    let drum = drumconfig[voice];
    if (drum.hits[beat]) {
      hits.push(voice);
    }
  }
  return hits;
}

function setHits(value) {
  // receive as 0-15; want 1-16
  density_of_hits = value + 1;
}

function setVoices(value) {
  // receive as 0-7; want 1-8
  value++;
  
  density_of_voices = value;
  voices.push(root_voice);
}

function setDynamism(value) {
  dynamism = value;
}

function setRootVoice(value) {
  root_voice = value;
}

newPattern(); // testing

module.exports = {arduinoIn, getHits, DRUMS_ON};