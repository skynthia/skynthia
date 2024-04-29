let hits = new Array(16).fill(false);
let voices = new Array(8).fill(0);

let density_of_hits = 0;
let density_of_voices = 0;
let dynamism = 0;

function newPattern() {
  generateHits();
}

function generateHits() {
  hits[0] = true;
  let num_hits = Math.floor(16/density_of_hits);
  for (let i = 0; i < num_hits; i++) {

  }
}

function setHits(value) {
  density_of_hits = value;
}

function setVoices(value) {
  density_of_voices = value;
}

function setDynamism(value) {
  dynamism = value;
}

module.exports = { newPattern, setHits, setVoices, setDynamism };