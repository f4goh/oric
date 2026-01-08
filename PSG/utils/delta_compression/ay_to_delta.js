function log(txt) {
    const div = document.getElementById("log");
    div.textContent += txt + "\n";
}

function selectRaw() {
    document.getElementById("fileInput").click();
}

function loadRaw(event) {
    const file = event.target.files[0];
    if (!file) return;

    document.getElementById("log").textContent = "Chargement du fichier brut AY...\n";

    const reader = new FileReader();
    reader.onload = function(e) {
        const data = new Uint8Array(e.target.result);

        if (data.length % 14 !== 0) {
            log("ERREUR : la taille du fichier n'est pas un multiple de 14 octets.");
            return;
        }

        const frameCount = data.length / 14;
        log("Taille fichier : " + data.length + " octets");
        log("Nombre de frames : " + frameCount);

        const frames = [];
        for (let i = 0; i < frameCount; i++) {
            frames.push([...data.slice(i * 14, i * 14 + 14)]);
        }

        log("Découpage en frames : OK");

        const compressed = compressDelta(frames);

        log("Compression Delta terminée.");
        log("Taille compressée : " + compressed.length + " octets");

        downloadBinary(compressed, "comp.bin");
        log("Fichier comp.bin généré.");
    };

    reader.readAsArrayBuffer(file);
}

function compressDelta(frames) {
    const output = [];
    let prev = frames[0];

    output.push(...prev);

    for (let i = 1; i < frames.length; i++) {
        const frame = frames[i];
        let bitmask_low = 0;
        let bitmask_high = 0;
        const values = [];

        for (let r = 0; r < 14; r++) {
            if (frame[r] !== prev[r]) {
                if (r < 8) bitmask_low |= (1 << r);
                else bitmask_high |= (1 << (r - 8));
                values.push(frame[r]);
            }
        }

        output.push(bitmask_low, bitmask_high, ...values);
        prev = frame;
    }

    return new Uint8Array(output);
}

function downloadBinary(data, filename) {
    const blob = new Blob([data], { type: "application/octet-stream" });
    const url = URL.createObjectURL(blob);

    const a = document.createElement("a");
    a.href = url;
    a.download = filename;
    a.click();

    URL.revokeObjectURL(url);
}
