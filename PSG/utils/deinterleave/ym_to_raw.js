function log(msg) {
    const div = document.getElementById("log");
    div.textContent += msg + "\n";
}

function selectYM() {
    document.getElementById("fileInput").click();
}

function loadYM(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function(e) {
        const data = new Uint8Array(e.target.result);
        try {
            processYM(data, file.name);
        } catch (err) {
            log("Erreur : " + err.message);
        }
    };
    reader.readAsArrayBuffer(file);
}

function readNullTerminatedString(data, offset) {
    let bytes = [];
    let i = offset;
    while (i < data.length && data[i] !== 0) {
        bytes.push(data[i]);
        i++;
    }
    return { text: new TextDecoder("ascii").decode(new Uint8Array(bytes)), nextOffset: i + 1 };
}

function processYM(data, filename) {
    document.getElementById("log").textContent = "";
    log("Fichier chargé : " + filename);

    if (data.length < 4) throw new Error("Fichier trop petit pour être un YM");

    const sig = String.fromCharCode(data[0], data[1], data[2], data[3]);
    log("Signature : " + sig);

    let oldFormat = false;
    let regCount = 16;
    let nFrames = 0;
    let interleaved = 1;
    let songDataStart = 0;

    if (sig === "YM5!") {
        log("Format YM5 détecté");
        const dv = new DataView(data.buffer);
        nFrames = dv.getInt32(0x0C, false); // big-endian
        log("Nombre de frames : " + nFrames);

        interleaved = data[0x13];
        log("Interleaved flag : " + interleaved);

        let offset = 0x22;
        let t1 = readNullTerminatedString(data, offset); // title
        offset = t1.nextOffset;
        let t2 = readNullTerminatedString(data, offset); // author
        offset = t2.nextOffset;
        let t3 = readNullTerminatedString(data, offset); // comments
        offset = t3.nextOffset;

        songDataStart = offset;
        log("Début des données song_data à l’offset : 0x" + songDataStart.toString(16));
    } else if (sig === "YM3!" || sig === "YM3b") {
        log("Format ancien YM3/YM3b détecté");
        oldFormat = true;
        regCount = 14;
        const fileSize = data.length;
        nFrames = Math.floor((fileSize - 4) / 14);
        log("Taille fichier : " + fileSize + " bytes");
        log("Nombre de frames estimé : " + nFrames);
        interleaved = 1;
        songDataStart = 4;
    } else {
        throw new Error("Format YM non supporté : " + sig);
    }

    let songData = [];
    for (let i = songDataStart; i < data.length; i++) {
        songData.push(data[i]);
    }

    if (!oldFormat) {
        if (songData.length >= 4) {
            songData.splice(songData.length - 4, 4);
        }
    }

    const songDataLen = songData.length;
    log("Longueur song_data : " + songDataLen + " bytes");
    log("Song length : " + nFrames + " frames");

    let registers = {};
    for (let i = 0; i < regCount; i++) {
        registers[i] = [];
    }

    if (interleaved === 1) {
        log("Données entrelacées → désentrelacement");
        for (let i = 0; i < regCount; i++) {
            const start = i * nFrames;
            const end = start + nFrames;
            registers[i] = songData.slice(start, end);
        }
    } else {
        log("Données non entrelacées → lecture séquentielle");
        let idx = 0;
        for (let f = 0; f < nFrames; f++) {
            for (let r = 0; r < regCount; r++) {
                if (idx >= songData.length) break;
                registers[r].push(songData[idx++]);
            }
        }
    }

    let allFrames = [];
    for (let f = 0; f < nFrames; f++) {
        for (let r = 0; r < 14; r++) {
            const val = registers[r][f] !== undefined ? registers[r][f] : 0;
            allFrames.push(val);
        }
    }

    log("Total frames exportées : " + nFrames);
    log("Total octets bruts (14 regs/frame) : " + allFrames.length);

    const out = new Uint8Array(allFrames);
    const blob = new Blob([out], { type: "application/octet-stream" });
    const url = URL.createObjectURL(blob);

    const a = document.createElement("a");
    a.href = url;
    a.download = filename.replace(/\.ym$/i, "") + "_raw14.bin";
    a.click();

    log("Fichier brut téléchargé : " + a.download);
}
