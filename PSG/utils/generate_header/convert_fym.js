// -------------------------------------------------------------
// Sélection du fichier
// -------------------------------------------------------------
function selectFile() {
    document.getElementById("fileInput").click();
}

// -------------------------------------------------------------
// Lecture du fichier FYM
// -------------------------------------------------------------
function loadFYM(event) {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();

    reader.onload = function(e) {
        const data = new Uint8Array(e.target.result);
        processFYM(data, file.name);
    };

    reader.readAsArrayBuffer(file);
}

// -------------------------------------------------------------
// Lecture string FYM (latin-1)
// -------------------------------------------------------------
function getString(psgdump, ptrObj) {
    let bytes = [];
    let c;
    while ((c = psgdump[ptrObj.ptr++]) !== 0) bytes.push(c);
    return new TextDecoder("latin1").decode(new Uint8Array(bytes));
}

// -------------------------------------------------------------
// Lecture int32 little-endian
// -------------------------------------------------------------
function getInt(psgdump, ptrObj) {
    let r = 0;
    for (let i = 0; i < 4; i++)
        r |= psgdump[ptrObj.ptr++] << (8 * i);
    return r;
}

// -------------------------------------------------------------
// Traitement du FYM
// -------------------------------------------------------------
function processFYM(fymData, filename) {

    // Décompression FYM → données brutes
    const psgdump = pako.inflate(fymData);

    let ptrObj = { ptr: 0 };

    // Lecture de l’en-tête FYM
    const offset     = getInt(psgdump, ptrObj);
    const frameCount = getInt(psgdump, ptrObj);
    const loopFrame  = getInt(psgdump, ptrObj);
    const clockRate  = getInt(psgdump, ptrObj);
    const frameRate  = getInt(psgdump, ptrObj);
    const trackName  = getString(psgdump, ptrObj);
    const authorName = getString(psgdump, ptrObj);

    // ---------------------------------------------------------
    // Extraction des données AY (14 * frameCount octets)
    // ---------------------------------------------------------
    let ayData = [];
    for (let r = 0; r < 14; r++) {
        for (let f = 0; f < frameCount; f++) {
            ayData.push(psgdump[offset + r * frameCount + f]);
        }
    }

    // ---------------------------------------------------------
    // Génération du fichier .h
    // ---------------------------------------------------------
    let out = "";
    out += "// Fichier généré automatiquement depuis " + filename + "\n\n";
    out += "#ifndef _FYM_DATA_H_\n#define _FYM_DATA_H_\n\n";

    out += "static const unsigned int fym_offset     = " + offset + ";\n";
    out += "static const unsigned int fym_frameCount = " + frameCount + ";\n";
    out += "static const unsigned int fym_loopFrame  = " + loopFrame + ";\n";
    out += "static const unsigned int fym_clockRate  = " + clockRate + ";\n";
    out += "static const unsigned int fym_frameRate  = " + frameRate + ";\n\n";

    out += "static const char fym_trackName[]  = \"" + trackName + "\";\n";
    out += "static const char fym_authorName[] = \"" + authorName + "\";\n\n";

    out += "static const unsigned char fym_data[] = {\n";

    // ---------------------------------------------------------
    // Formatage des données AY : lignes de 14 octets hex
    // ---------------------------------------------------------
    for (let f = 0; f < frameCount; f++) {
        out += "    ";
        for (let r = 0; r < 14; r++) {
            const val = ayData[r * frameCount + f];
            out += "0x" + val.toString(16).padStart(2, "0").toUpperCase();
            if (!(f === frameCount - 1 && r === 13)) out += ", ";
        }
        out += "\n";
    }

    out += "};\n\n#endif\n";

    // ---------------------------------------------------------
    // Téléchargement du fichier .h
    // ---------------------------------------------------------
    const blob = new Blob([out], { type: "text/plain" });
    const url = URL.createObjectURL(blob);

    const a = document.createElement("a");
    a.href = url;
    a.download = filename.replace(".fym", ".h");
    a.click();

    document.getElementById("log").textContent =
        "Conversion terminée ! Fichier généré : " + a.download;
}
