const drumconfig = require("./drumconfig.json");

let DRUMS_ON = true;

let density_of_hits = 0;
let density_of_voices = 0;
let dynamism = 0.5;
let voices = [];
let next_voice_probs;
let change_pattern = false;
let change_all_voices = false;

function arduinoIn(value) {
  value = value.split(" ");
  switch (value[1]) {
    case "0":
      setDensityOfHits(Number(value[2]));
      break;
    case "1":
      setDensityOfVoices(Number(value[2]));
      break;
    case "2":
      setDynamism(Number(value[2]));
      break;
    case "3":
      setRootVoice(Number(value[2]));
      break;
  }
}

function generatePattern(new_voices) {
  if (change_all_voices) {
    voices = [];
    generateVoices();
    change_all_voices = false;
  }

  for (let i = 0; i < voices.length; i++) {
    let voice = voices[i];
    generateHits(voice);
  }
  change_pattern = false;
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

  // TODO: Make this more efficient??
  for (let i = 0; i < num_hits; i++) {
    let found_hit = false;
    while (!found_hit) {
      for (let j = probs.length - 1; j >= 0; j--) {
        if (drum.equal_prob) {
          let note = Math.floor(Math.random() * 16);
          if (!drum.hits[note]) {
            drum.hits[note] = true;
            found_hit = true;
          }
        }
        else {
          let rand = Math.random();
          if (probs[j].length && rand < j/10) {
            let idx = Math.floor(Math.random() * probs[j].length);
            let hit = probs[j][idx];
            drum.hits[hit] = true;
            probs[j].splice(idx, 1); // remove the hit we just generated
            found_hit = true;
          }
        }
        break;
        
      }
    }
  }
  console.log("hits for " + voice + ": " + drum.hits.join(" "));
}

// beat is 0-15
function getHits(beat, measure) {
  if (change_pattern && beat === 0 && measure === 0) {
    // make a new pattern only at the end of a measure
    generatePattern();
  }
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

function generateVoices() {
  if (voices.length ===  0) {
    voices.push(root_voice);
  }

  let diff = density_of_voices - voices.length;
  // Adding voices
  if (diff > 0) {
    for (let i = 0; i < diff; i++) {
      let added_voice = false;
      while(!added_voice) {
        for (let j = 0; j < next_voice_probs.length; j++) {
          let rand = Math.random();
          if (rand < next_voice_probs[j].prob) {
            let next_voice = next_voice_probs[j].voice;
            voices.push(next_voice);
            next_voice_probs.splice(j, 1);
            generateHits(next_voice);
            added_voice = true;

            console.log("Adding voice: " + drumconfig[next_voice].name);
            break;
          }
        }
      }
    }
  }
  // subtracting voices
  else if (diff < 0) {
    // random index between 1 and last index of array
    for (let i = 0; i < Math.abs(diff); i++) {
      let idx = Math.floor(Math.random() * voices.length - 1) + 1;
      
      console.log("Removing voice: " + drumconfig[voices[idx]].name);
      voices.splice(idx, 1);
    }
  }
}

function setDensityOfHits(value) {
  change_pattern = true;
}

function setDensityOfVoices(value) {
  if (value < 1 || value > 8) {
    console.error("INVALID SET VOICES COMMAND");
  }
  
  density_of_voices = value;
  generateVoices();
  change_pattern = true;
}

function setDynamism(value) {
  dynamism = value;
  change_pattern = true;
}

function setRootVoice(value) {
  root_voice = value;
  // Make a copy of the voice probs
  let temp = drumconfig[root_voice].voice_probs; // just a separate var for visibility's sake
  next_voice_probs = temp.slice(0);
  change_pattern = true;
  change_all_voices = true;
}

setRootVoice(0);
setDensityOfVoices(1);
setDensityOfHits(6);
generatePattern(); // testing

setTimeout(() => {
  setDensityOfVoices(3);
}, 4000);

module.exports = {arduinoIn, getHits, DRUMS_ON};