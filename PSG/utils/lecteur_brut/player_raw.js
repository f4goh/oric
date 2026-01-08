/* -------------------------------------------------------------------------
   VARIABLES GLOBALES
---------------------------------------------------------------------------*/
var psgdump = null;      // Contiendra les données brutes (14 octets * frameCount)
var frameCount = 0;
var frameRate = 50;      // Fixe
var clockRate = 1750000; // Fixe
var frame = 0;
var sampleRate = 44100;
var isPlaying = false;

var ayumi = new Ayumi;
var audioContext;
var audioNode;

/* -------------------------------------------------------------------------
   Sélection du fichier
---------------------------------------------------------------------------*/
function selectFile() {
    document.getElementById("fileInput").click();
}

/* -------------------------------------------------------------------------
   Chargement du fichier brut (14 octets par frame)
---------------------------------------------------------------------------*/
function loadRaw(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();

    reader.onload = function(e) {
        psgdump = new Uint8Array(e.target.result);

        // Vérification que la taille est multiple de 14
        if (psgdump.length % 14 !== 0) {
            alert("Erreur : le fichier ne contient pas un multiple de 14 octets.");
            return;
        }

        frameCount = psgdump.length / 14;
        frame = 0;

        initAudio();

        document.getElementById("debug").textContent =
            "Fichier chargé.\nFrames : " + frameCount + "\nPrêt à lire.";
    };

    reader.readAsArrayBuffer(file);
}

/* -------------------------------------------------------------------------
   Initialisation audio + AY
---------------------------------------------------------------------------*/
function initAudio() {
    ayumi.configure(true, clockRate, sampleRate);
    ayumi.setPan(0, 0.1, 0);
    ayumi.setPan(1, 0.5, 0);
    ayumi.setPan(2, 0.9, 0);

    audioContext = new (window.AudioContext || window.webkitAudioContext)();
    audioNode = audioContext.createScriptProcessor(4096, 0, 2);
    audioNode.onaudioprocess = fillBuffer;
}

/* -------------------------------------------------------------------------
   Mise à jour AY
---------------------------------------------------------------------------*/
function updateState(renderer, r) {
    renderer.setTone(0, (r[1] << 8) | r[0]);
    renderer.setTone(1, (r[3] << 8) | r[2]);
    renderer.setTone(2, (r[5] << 8) | r[4]);
    renderer.setNoise(r[6]);
    renderer.setMixer(0, r[7] & 1, (r[7] >> 3) & 1, r[8] >> 4);
    renderer.setMixer(1, (r[7] >> 1) & 1, (r[7] >> 4) & 1, r[9] >> 4);
    renderer.setMixer(2, (r[7] >> 2) & 1, (r[7] >> 5) & 1, r[10] >> 4);
    renderer.setVolume(0, r[8] & 0xf);
    renderer.setVolume(1, r[9] & 0xf);
    renderer.setVolume(2, r[10] & 0xf);
    renderer.setEnvelope((r[12] << 8) | r[11]);
    if (r[13] != 255) renderer.setEnvelopeShape(r[13]);
}

/* -------------------------------------------------------------------------
   Debugger
---------------------------------------------------------------------------*/
function updateDebug(raw, decoded) {
    let txt = "=== DEBUG AY ===\n\n";

    txt += "--- Registres bruts (hex) ---\n";
    for (let i = 0; i < 14; i++) {
        txt += "R" + i.toString().padStart(2, "0") + " = " +
               raw[i].toString(16).padStart(2, "0").toUpperCase() + "\n";
    }

    txt += "\n--- Registres décodés ---\n";
    txt += "Tone A = 0x" + decoded.toneA.toString(16).padStart(4, "0").toUpperCase() + "\n";
    txt += "Tone B = 0x" + decoded.toneB.toString(16).padStart(4, "0").toUpperCase() + "\n";
    txt += "Tone C = 0x" + decoded.toneC.toString(16).padStart(4, "0").toUpperCase() + "\n";
    txt += "Noise  = 0x" + decoded.noise.toString(16).padStart(2, "0").toUpperCase() + "\n";
    txt += "Vol A  = 0x" + decoded.volA.toString(16).toUpperCase() + "\n";
    txt += "Vol B  = 0x" + decoded.volB.toString(16).toUpperCase() + "\n";
    txt += "Vol C  = 0x" + decoded.volC.toString(16).toUpperCase() + "\n";
    txt += "Env    = 0x" + decoded.env.toString(16).padStart(4, "0").toUpperCase() + "\n";
    txt += "Shape  = 0x" + decoded.shape.toString(16).toUpperCase() + "\n";

    document.getElementById("debug").textContent = txt;
}

/* -------------------------------------------------------------------------
   Génération audio
---------------------------------------------------------------------------*/
var isrCounter = 0;

function fillBuffer(e) {
    if (!isPlaying || !psgdump) return;

    var isrStep = frameRate / sampleRate;
    var left = e.outputBuffer.getChannelData(0);
    var right = e.outputBuffer.getChannelData(1);

    for (var i = 0; i < left.length; i++) {

        isrCounter += isrStep;
        if (isrCounter >= 1) {
            isrCounter--;

            // Lecture des 14 registres de la frame courante
            let raw = psgdump.slice(frame * 14, frame * 14 + 14);

            // Décodage pour debugger
            let decoded = {
                toneA: (raw[1] << 8) | raw[0],
                toneB: (raw[3] << 8) | raw[2],
                toneC: (raw[5] << 8) | raw[4],
                noise: raw[6],
                volA: raw[8] & 0xF,
                volB: raw[9] & 0xF,
                volC: raw[10] & 0xF,
                env: (raw[12] << 8) | raw[11],
                shape: raw[13]
            };

            updateState(ayumi, raw);
            updateDebug(raw, decoded);

            frame++;
            if (frame >= frameCount) frame = 0;
        }

        ayumi.process();
        ayumi.removeDC();
        left[i] = ayumi.left;
        right[i] = ayumi.right;
    }
}

/* -------------------------------------------------------------------------
   Lecture / Pause
---------------------------------------------------------------------------*/
function startAudio() {
    if (!audioContext) return;
    audioContext.resume();
    audioNode.connect(audioContext.destination);
    isPlaying = true;
}

function pauseAudio() {
    if (!audioContext) return;
    audioNode.disconnect();
    isPlaying = false;
}
