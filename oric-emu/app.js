
/* =========================
   MATRICE ORIC (8x8)
========================= */

const matrix = [
    ["7", "N", "5", "V", "Right Ctrl", "1", "X", "3"],
    ["J", "T", "R", "F", "", "esc", "Q", "D"],
    ["M", "6", "B", "4", "Ctrl", "Z", "2", "C"],
    ["K", "9", ";", "-", "", "", "\\", "'"],
    ["Space", ",", ".", "Up", "Left Shift", "Left", "Down", "Right"],
    ["U", "I", "O", "P", "Fct", "Del", "]", "["],
    ["Y", "H", "G", "E", "", "A", "S", "W"],
    ["8", "L", "0", "/", "Right Shift", "Return", "", "="]
];


/* =========================
   BUILD MAP ORIC
========================= */

const oricKeys = {};

function norm(k)
{
    return k.trim().toUpperCase();
}

for (let row = 0; row < matrix.length; row++) {
    for (let col = 0; col < matrix[row].length; col++) {

        const key = matrix[row][col];

        if (key && key.trim() !== "") {
            oricKeys[norm(key)] = [col, row];
        }
    }
}


/* =========================
   NES MAPPING
========================= */

const controls = {
    1: "btn1",     // Right
    2: "btn2",     // Left
    4: "btn4",     // Down
    8: "btn8",     // Up
    16: "btn16",   // Start
    32: "btn32",   // Select
    64: "btn64",   // B
    128: "btn128"  // A
};


/* =========================
   FILL COMBOS
========================= */

function populateCombos()
{
    const selectKeys = Object.keys(oricKeys).sort();

    for (const id of Object.values(controls)) {

        const sel = document.getElementById(id);

        selectKeys.forEach(k => {

            const opt = document.createElement("option");
            opt.value = k;
            opt.textContent = k;

            sel.appendChild(opt);
        });
    }

    /* Préconfig NES (optionnel) */
    setDefault();
}


/* =========================
   DEFAULT CONFIG (ton exemple)
========================= */

function setDefault()
{
    const set = (id, key) => {
        const el = document.getElementById(id);
        if (el) el.value = key;
    };

    set("btn1",   "RIGHT");
    set("btn2",   "LEFT");
    set("btn4",   "DOWN");
    set("btn8",   "UP");

    set("btn128", "SPACE");
    set("btn64",  "CTRL");
    set("btn32",  "RETURN");
    set("btn16",  "ESC");
}


/* =========================
   GENERATE CSV
========================= */

function generateCSV()
{
    let csv = "";

    for (const [nesCode, selectId] of Object.entries(controls)) {

        const key = document.getElementById(selectId).value;

        const entry = oricKeys[norm(key)];

        if (!entry) {
            console.warn("Touche inconnue:", key);
            continue;
        }

        const [col, row] = entry;

        csv += `${nesCode},${col},${row}\n`;
    }

    let filename = document.getElementById("filename").value.trim();
    if (!filename) filename = "mapping";

    const blob = new Blob([csv], { type: "text/csv" });

    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = filename + ".csv";

    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);

    URL.revokeObjectURL(a.href);
}


/* =========================
   INIT
========================= */

document.getElementById("generateBtn")
    .addEventListener("click", generateCSV);

populateCombos();