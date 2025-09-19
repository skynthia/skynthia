const drumconfig = require("./drumconfig.json");
const vibeconfig = require("./vibeconfig.json");
const util = require("./util");

let DRUMS_ON = true;

let voices = [];

let density_of_hits     = 0;
let density_of_voices   = 1;
let dynamism            = 0;
let vibe                = vibeconfig[0];
let root_voice          = vibe.root;
let next_voice_probs    = vibe.probs;
let change_pattern      = false;
let change_voices       = false;
let change_all_voices   = false;
let turn_drums_on       = false;
let turn_drums_off      = false;

let status = -1; // 0-7: status effects

let measures_since_change = 0;
let change_target = 1;

function arduinoIn(value) {
  let num_val = value.charCodeAt(2) - 65; // get the int
  switch (value[1]) {
    case 'H':
      setDensityOfHits(num_val);
      break;
    case 'V':
      setDensityOfVoices(num_val + 1);
      break;
    case 'D':
      setDynamism(num_val);
      break;
    case 'B':
      setVibe(num_val);
      break;
    case 'F':
      setFX(num_val);
      break;
  }
}

// beat is 0-15
function getHits(beat, measure) {
  if (beat === 0 && measure === 0) {
    // chance of changing up drum pattern every four measures
    if (measures_since_change % (4 * change_target) === 0) {
      let r = Math.random();
      util.log(r);
      if (r < change_target / 8) {
        change_pattern = true;
        measures_since_change = -1; // TODO: this is stupid, better solution??
        change_target = 0.5;
      }
      
      change_target *= 2;
    }
    measures_since_change++;

    if (change_pattern) {
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
  num_hits = num_hits > drum.max ? drum.max : num_hits; // never more than max for this voice

  // TODO: Make this more efficient??
  for (let i = 0; i < num_hits; i++) {
    let found_hit = false;
    let tries = 0;
    while (!found_hit && tries < drum.max) {
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
          if (rand < probs[j].prob || tries === drum.max - 1) {
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
  util.log("hits for " + voice + ": " + drum.hits.join(" "));
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

            util.log("Adding voice: " + drumconfig[next_voice].name);
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
      
      util.log("Removing voice: " + drumconfig[voices[idx]].name);
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
  let last_doh = density_of_hits;
  density_of_hits = value;
  if (last_doh === 0 && density_of_hits != 0) {
    change_voices = true;
    generatePattern();
  }
  else {
    change_pattern = true;
  }
}

function setDensityOfVoices(value) {
  if (value < 1 || value > 8) {
    util.error("Invalid set density of voices command");
  }
  
  density_of_voices = value;
  change_voices = true;
  change_pattern = true;
}

function setDynamism(value) {
  dynamism = value;
  change_pattern = true;
}

function setVibe(value) {
  vibe = vibeconfig[value];
  root_voice = vibe.root_voice;
  next_voice_probs = vibe.probs;

  change_pattern = true;
  change_voices = true;
  change_all_voices = true;
}

function setFX(value) {
    status = {
        type: 1,
        val: value
    };
}

// TODO: only do this at the end of a bar/measure?
function getStatus() {
  let temp = status;
  status = -1;
  return temp;
}

module.exports = {arduinoIn, getHits, getStatus};