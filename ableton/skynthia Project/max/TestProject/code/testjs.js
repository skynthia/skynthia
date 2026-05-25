function test() {
    post("hi");
    outlet(0, arguments[1]);
}

let last_note;

function msg_int() {
    post(arguments[0]);
    let note = 60 + arguments[0];
    last_note = note;
    outlet_array(0, [note, 127]);
    let t = new Task(noteOff, this);
    t.schedule(250);
}

function noteOff() {
    //post("hi" + note);
    let test = [last_note, 0];
    //outlet(0, note + " 0");
    outlet_array(0, test);
}