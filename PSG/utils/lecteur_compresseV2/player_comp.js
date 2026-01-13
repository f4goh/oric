/* -------------------------------------------------------------------------
   VARIABLES GLOBALES
---------------------------------------------------------------------------*/
var psgdump = null;      // Contiendra les données décompressées
var frameCount = 0;
var frameRate = 50;      // 50Hz (Standard Europe/Atari ST/ZX)
var clockRate = 1750000; // Fréquence horloge (1.75 MHz)
var frame = 0;
var sampleRate = 44100;
var isPlaying = false;

var ayumi = new Ayumi;
var audioContext;
var audioNode;

// --- Visualisation ---
var analyser = null;
var bufferLength = 0;
var dataArrayTime = null;
var dataArrayFreq = null;
var animationId = null;
var canvasCtxFreq = null;
var canvasCtxTime = null;
var canvasFreq = null;
var canvasTime = null;

/* -------------------------------------------------------------------------
   Sélection du fichier
---------------------------------------------------------------------------*/
function selectFile() {
    document.getElementById("fileInput").click();
}

/* -------------------------------------------------------------------------
   Décompression du format Delta+Bitmask
---------------------------------------------------------------------------*/
function decompressDeltaBitmask(data) {
    const FRAME_SIZE = 14;
    let pos = 0;
    let frames = [];

    // Frame 0 brute
    if (data.length < FRAME_SIZE) {
        throw new Error("Fichier compressé trop petit");
    }

    let regs = [];
    for (let i = 0; i < FRAME_SIZE; i++) {
        regs[i] = data[pos++];
    }
    frames.push(regs.slice());

    // Frames suivantes
    while (pos < data.length) {
        if (pos + 2 > data.length) break;

        let bitmask_low = data[pos++];
        let bitmask_high = data[pos++];

        for (let i = 0; i < FRAME_SIZE; i++) {
            let bit;
            if (i < 8) {
                bit = (bitmask_low >> i) & 1;
            } else {
                bit = (bitmask_high >> (i - 8)) & 1;
            }
            if (bit === 1) {
                if (pos >= data.length) {
                    throw new Error("Fichier compressé tronqué");
                }
                regs[i] = data[pos++];
            }
        }
        frames.push(regs.slice());
    }

    // Aplatir en Uint8Array
    let flat = new Uint8Array(frames.length * FRAME_SIZE);
    let k = 0;
    for (let f = 0; f < frames.length; f++) {
        for (let i = 0; i < FRAME_SIZE; i++) {
            flat[k++] = frames[f][i];
        }
    }

    return { flat, frameCount: frames.length };
}

/* -------------------------------------------------------------------------
   Chargement du fichier compressé
---------------------------------------------------------------------------*/
function loadCompressed(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();

    reader.onload = function(e) {
        const compressed = new Uint8Array(e.target.result);

        try {
            const result = decompressDeltaBitmask(compressed);
            psgdump = result.flat;
            frameCount = result.frameCount;
            frame = 0;

            initAudio();

            document.getElementById("debug").textContent =
                "Fichier chargé avec succès.\n" +
                "Frames : " + frameCount + "\n" +
                "Durée approx : " + (frameCount / frameRate).toFixed(2) + " sec\n" +
                "Prêt à lire.";
        } catch (err) {
            alert("Erreur de décompression : " + err.message);
        }
    };

    reader.readAsArrayBuffer(file);
}

/* -------------------------------------------------------------------------
   Initialisation audio + Visualiseurs
---------------------------------------------------------------------------*/
function initAudio() {
    // Config Ayumi
    ayumi.configure(true, clockRate, sampleRate);
    ayumi.setPan(0, 0.1, 0); // Canal A (gauche)
    ayumi.setPan(1, 0.5, 0); // Canal B (centre)
    ayumi.setPan(2, 0.9, 0); // Canal C (droite)

    // Init Context Audio
    if (!audioContext) {
        audioContext = new (window.AudioContext || window.webkitAudioContext)();
    }
    
    // --- Création du graphe audio pour visualisation ---
    if (!analyser) {
        analyser = audioContext.createAnalyser();
        analyser.fftSize = 2048; // Résolution (plus haut = plus précis en fréq, moins en temps)
        analyser.smoothingTimeConstant = 0.85; // Lissage visuel
        
        bufferLength = analyser.frequencyBinCount;
        dataArrayTime = new Uint8Array(bufferLength);
        dataArrayFreq = new Uint8Array(bufferLength);
        
        setupVisualizers();
    }

    // ScriptProcessor (Note: AudioWorklet serait plus moderne, mais SP est plus simple ici)
    // Buffer size 4096 = latence d'env 90ms, acceptable pour playback simple
    if (!audioNode) {
        audioNode = audioContext.createScriptProcessor(4096, 0, 2);
        audioNode.onaudioprocess = fillBuffer;
        
        // Connexion PERMANENTE : Script -> Analyser
        // L'analyser sera connecté à la sortie (destination) uniquement lors du play
        audioNode.connect(analyser);
    }
}

/* -------------------------------------------------------------------------
   Configuration des Canvas
---------------------------------------------------------------------------*/
function setupVisualizers() {
    canvasFreq = document.getElementById("spectrumCanvas");
    canvasCtxFreq = canvasFreq.getContext("2d");

    canvasTime = document.getElementById("scopeCanvas");
    canvasCtxTime = canvasTime.getContext("2d");
}

/* -------------------------------------------------------------------------
   Boucle de rendu graphique (RequestAnimationFrame)
---------------------------------------------------------------------------*/
function drawVisualizers() {
    if (!isPlaying) return;

    animationId = requestAnimationFrame(drawVisualizers);

    // 1. Récupération des données temps réel
    analyser.getByteTimeDomainData(dataArrayTime); // Forme d'onde
    analyser.getByteFrequencyData(dataArrayFreq);  // Spectre

    // --- DESSIN SPECTRE (Barres) ---
    var width = canvasFreq.width;
    var height = canvasFreq.height;
    
    // Effacement avec transparence pour effet de traînée (optionnel, ici noir pur)
    canvasCtxFreq.fillStyle = '#000';
    canvasCtxFreq.fillRect(0, 0, width, height);

    var barWidth = (width / bufferLength) * 2.5;
    var barHeight;
    var x = 0;

    for(var i = 0; i < bufferLength; i++) {
        barHeight = dataArrayFreq[i] / 255 * height;

        // Couleur dynamique basé sur l'intensité
        // Violet/Bleu en bas, Cyan/Blanc en haut
        var r = barHeight + (25 * (i/bufferLength));
        var g = 250 * (i/bufferLength);
        var b = 255;

        canvasCtxFreq.fillStyle = 'rgb(' + r + ',' + g + ',' + b + ')';
        canvasCtxFreq.fillRect(x, height - barHeight, barWidth, barHeight);

        x += barWidth + 1;
        if(x > width) break;
    }

    // --- DESSIN OSCILLOSCOPE (Ligne) ---
    width = canvasTime.width;
    height = canvasTime.height;

    canvasCtxTime.fillStyle = '#000';
    canvasCtxTime.fillRect(0, 0, width, height);

    canvasCtxTime.lineWidth = 2;
    canvasCtxTime.strokeStyle = '#0f0'; // Vert oscilloscope classique
    canvasCtxTime.beginPath();

    var sliceWidth = width * 1.0 / bufferLength;
    x = 0;

    for(var i = 0; i < bufferLength; i++) {
        var v = dataArrayTime[i] / 128.0;
        var y = v * height / 2;

        if(i === 0) {
            canvasCtxTime.moveTo(x, y);
        } else {
            canvasCtxTime.lineTo(x, y);
        }

        x += sliceWidth;
    }

    canvasCtxTime.lineTo(canvasTime.width, canvasTime.height / 2);
    canvasCtxTime.stroke();
}

/* -------------------------------------------------------------------------
   Mise à jour AY (Registres)
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
   Debugger Texte
---------------------------------------------------------------------------*/
function updateDebug(raw, decoded) {
    let txt = "=== DEBUG AY (Frame " + frame + "/" + frameCount + ") ===\n\n";

    txt += "--- Registres Raw ---\n";
    // Affichage compact
    for (let i = 0; i < 14; i++) {
        txt += raw[i].toString(16).padStart(2, "0").toUpperCase() + " ";
    }
    
    txt += "\n\n--- Valeurs Décodées ---\n";
    txt += "Tone A: " + decoded.toneA + " | Vol: " + decoded.volA + "\n";
    txt += "Tone B: " + decoded.toneB + " | Vol: " + decoded.volB + "\n";
    txt += "Tone C: " + decoded.toneC + " | Vol: " + decoded.volC + "\n";
    txt += "Noise : " + decoded.noise + "\n";
    txt += "Env   : " + decoded.env + " (Shape: " + decoded.shape + ")";

    document.getElementById("debug").textContent = txt;
}

/* -------------------------------------------------------------------------
   Génération audio (Callback)
---------------------------------------------------------------------------*/
var isrCounter = 0;

function fillBuffer(e) {
    if (!isPlaying || !psgdump) {
        // Silence si pas de lecture
        e.outputBuffer.getChannelData(0).fill(0);
        e.outputBuffer.getChannelData(1).fill(0);
        return;
    }

    var isrStep = frameRate / sampleRate;
    var left = e.outputBuffer.getChannelData(0);
    var right = e.outputBuffer.getChannelData(1);

    for (var i = 0; i < left.length; i++) {

        isrCounter += isrStep;
        if (isrCounter >= 1) {
            isrCounter--;

            // Lecture des registres
            let raw = psgdump.slice(frame * 14, frame * 14 + 14);

            // Décodage pour le debug
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
            
            // Pour ne pas spammer le DOM, on met à jour le debug toutes les 5 frames (approx)
            if(frame % 5 === 0) updateDebug(raw, decoded);

            frame++;
            if (frame >= frameCount) frame = 0; // Boucle
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
    
    // Réveil du contexte (navigateurs modernes)
    if(audioContext.state === 'suspended') {
        audioContext.resume();
    }
    
    // Connexion finale : Analyser -> Haut-parleurs
    analyser.connect(audioContext.destination);
    
    isPlaying = true;
    
    // Lancement de la boucle visuelle
    drawVisualizers();
}

function pauseAudio() {
    if (!audioContext) return;
    
    // Déconnexion de la sortie (coupe le son, mais le graphe reste en mémoire)
    analyser.disconnect(audioContext.destination);
    
    isPlaying = false;
    
    // Arrêt de la boucle visuelle
    if (animationId) {
        cancelAnimationFrame(animationId);
    }
}