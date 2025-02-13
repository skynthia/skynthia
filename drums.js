const drumconfig = require("./drumconfig.json");

let DRUMS_ON = true;

let voices = [];

let density_of_hits;
let density_of_voices;
let dynamism;
let root_voice;
let next_voice_probs;
let change_pattern = false;
let change_voices = false;
let change_all_voices = false;
let turn_drums_on = false;
let turn_drums_off = false;

function arduinoIn(value) {
  value = value.split(" ");
  let num_val = value[2].charCodeAt(0); // get the int
  switch (value[1]) {
    case "HARD":
      hardStartStop(num_val);
      break;
    case "ONOF":
      setDrumsOn(num_val);
      break;
    case "DENH":
      setDensityOfHits(num_val);
      break;
    case "DENV":
      setDensityOfVoices(num_val);
      break;
    case "DYNM":
      setDynamism(num_val);
      break;
    case "ROOT":
      setRootVoice(num_val);
      break;
  }
}

// beat is 0-15
function getHits(beat, measure) {
  if (change_pattern && beat === 0 && measure === 0) {
    if (turn_drums_off) {
      DRUMS_ON = false;
      turn_drums_off = false;
    }
    else if (turn_drums_on) {
      DRUMS_ON = true;
      turn_drums_on = false; // currently DOES generate new pattern.
    }
    else {
      // make a new pattern only at the end of a measure
      generatePattern();
    }
  }

  if (!DRUMS_ON) {
    return;
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

function generatePattern() {
  if (change_voices) {
    if (change_all_voices) {
      voices = [];
    }
    generateVoices();
    change_voices = false;
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
  let probs = JSON.parse(JSON.stringify(drum.probs)); // duplicate array
  drum.hits = new Array(16).fill(false);
  
  // randomly modify number of hits based on dynamism
  let modifier = Math.round(density_of_hits * (Math.random() * (dynamism / 2)));
  modifier = Math.random() > 0.5 ? modifier : -1 * modifier; // randomly + or -
  let num_hits = density_of_hits + modifier;
  num_hits = num_hits < 1 ? 1 : num_hits; // never less than 1
  num_hits = num_hits > drum.max ? drum.max : num_hits; // never more than 16

  // TODO: Make this more efficient??
  for (let i = 0; i < num_hits; i++) {
    let found_hit = false;
    let tries = 0;
    while (!found_hit && tries < 8) {
      for (let j = 0; j < probs.length; j++) {
        if (drum.equal_prob) {
          let note = Math.floor(Math.random() * 16);
          if (!drum.hits[note]) {
            drum.hits[note] = true;
            found_hit = true;
            break;
          }
        }
        else {
          let rand = Math.random();
          if (rand < probs[j].prob || tries === 7) {
            let idx = Math.floor(Math.random() * probs[j].notes.length);
            let hit = probs[j].notes[idx];
            drum.hits[hit] = true;
            probs[j].notes.splice(idx, 1); // remove the hit we just generated
            // If we're out of hits for this probability, remove the probability
            if (!probs[j].notes.length) {
              probs.splice(j, 1);
            }
            found_hit = true;
            break;
          }
        }
      }
      tries++;
    }
  }
  console.log("hits for " + voice + ": " + drum.hits.join(" "));
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
            let idx = Math.floor(Math.random() * next_voice_probs[j].voices.length);
            let next_voice = next_voice_probs[j].voices[idx];
            voices.push(next_voice);
            next_voice_probs[j].voices.splice(idx, 1);
            // remove the whole category when we've run out
            if (!next_voice_probs[j].voices.length) {
              next_voice_probs.splice(j, 1);
            }
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
      let idx = Math.floor(Math.random() * (voices.length - 1)) + 1;
      
      console.log("Removing voice: " + drumconfig[voices[idx]].name);
      voices.splice(idx, 1);
    }
  }
}

function hardStartStop(value) {
  DRUMS_ON = value;
}

function setDrumsOn(value) {
  if (value === 0) {
    turn_drums_off = true;
  }
  else if (value === 1) {
    turn_drums_off = true;
  }
  change_pattern = true;
}

function setDensityOfHits(value) {
  density_of_hits = value;
  change_pattern = true;
}

function setDensityOfVoices(value) {
  if (value < 1 || value > 8) {
    console.error("INVALID SET VOICES COMMAND");
  }
  
  density_of_voices = value;
  change_voices = true;
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
  next_voice_probs = JSON.parse(JSON.stringify(temp));
  change_pattern = true;
  change_all_voices = true;
}
/*
setRootVoice(0);
setDensityOfHits(12);
// todo: don't call generateHits in this function
setDensityOfVoices(1);
setDynamism(0.5);
generatePattern(); // testing

setTimeout(() => {
  setDensityOfVoices(3);
}, 4000);

setTimeout(() => {
  setDensityOfHits(3);
}, 12000);

setTimeout(() => {
  setDensityOfVoices(2);
}, 13000);

setTimeout(() => {
  setDensityOfVoices(6);
}, 26000);

setTimeout(() => {
  setDensityOfHits(6);
}, 42000);

setTimeout(() => {
  setDensityOfVoices(3);
}, 63000);

setTimeout(() => {
  setDrumsOn(0);
}, 80000);

setTimeout(() => {
  setDrumsOn(1);
}, 84000);

setTimeout(() => {
  hardStartStop(false);
}, 95000);*/

module.exports = {arduinoIn, getHits};