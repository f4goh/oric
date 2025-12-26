import {
    OricGfx,
    ORIC_WIDTH,
    ORIC_HEIGHT,
    INK,
    PAPER,
    NOIP,
    INVERTED
} from "./oricGfx.js";

// ---------------------------
// Constantes & couleurs
// ---------------------------

const LOGICAL_W = ORIC_WIDTH;
const LOGICAL_H = ORIC_HEIGHT;
const ZOOM = 3;

const SCREEN_W = LOGICAL_W * ZOOM;
const SCREEN_H = LOGICAL_H * ZOOM;

const colors = [
    [0, 0, 0],
    [255, 0, 0],
    [0, 255, 0],
    [255, 255, 0],
    [0, 0, 255],
    [255, 0, 255],
    [0, 255, 255],
    [255, 255, 255]
];

const GRAY = [80, 80, 80];
const GRAY_LIGHT = [150, 150, 150];

// ---------------------------
// Canvas & context
// ---------------------------

const canvas = document.getElementById("oricCanvas");
canvas.width = SCREEN_W;
canvas.height = SCREEN_H;

const ctxMain = canvas.getContext("2d");
ctxMain.imageSmoothingEnabled = false;

// Offscreen canvas pixels
const pixelsCanvas = document.createElement("canvas");
pixelsCanvas.width = LOGICAL_W;
pixelsCanvas.height = LOGICAL_H;
const ctxPixels = pixelsCanvas.getContext("2d");

// Offscreen canvas attributs
const attribCanvas = document.createElement("canvas");
attribCanvas.width = LOGICAL_W;
attribCanvas.height = LOGICAL_H;
const ctxAttrib = attribCanvas.getContext("2d");

// ---------------------------
// OricGfx
// ---------------------------

const gfx = new OricGfx();

// ---------------------------
// Fonctions utilitaires
// ---------------------------

function setPixel(ctx, x, y, rgb) {
    ctx.fillStyle = `rgb(${rgb[0]},${rgb[1]},${rgb[2]})`;
    ctx.fillRect(x, y, 1, 1);
}

function drawVerticalGrid(ctx, spacing = 6) {
    ctx.clearRect(0, 0, LOGICAL_W, LOGICAL_H);
    ctx.strokeStyle = `rgb(${GRAY[0]},${GRAY[1]},${GRAY[2]})`;
    ctx.beginPath();
    for (let x = 0; x < LOGICAL_W; x += spacing) {
        ctx.moveTo(x + 0.5, 0);
        ctx.lineTo(x + 0.5, LOGICAL_H);
    }
    ctx.stroke();
}

function draw_5px_horizontal(ctx, x, y, colorIndex, attFlag, attInvert) {
    x = Math.floor(x / 6) * 6 + 1;
    const color = colors[colorIndex];

    ctx.strokeStyle = `rgb(${color[0]},${color[1]},${color[2]})`;
    ctx.beginPath();

    if (attFlag === INK) {
        ctx.moveTo(x + 1, y + 0.5);
        ctx.lineTo(x + 3, y + 0.5);
    } else if (attFlag === PAPER) {
        ctx.moveTo(x, y + 0.5);
        ctx.lineTo(x + 4, y + 0.5);
    }
    ctx.stroke();

    if (attInvert) {
        ctx.strokeStyle = `rgb(${GRAY_LIGHT[0]},${GRAY_LIGHT[1]},${GRAY_LIGHT[2]})`;
        ctx.beginPath();
        ctx.moveTo(x, y + 0.5);
        ctx.lineTo(x + 1, y + 0.5);
        ctx.stroke();
    }
}

// ---------------------------
// Mise à jour complète
// ---------------------------

function updatePixels() {
    ctxPixels.clearRect(0, 0, LOGICAL_W, LOGICAL_H);

    for (let y = 0; y < LOGICAL_H; y++) {
        const line = gfx.getLinePx(y);
        for (let x = 0; x < LOGICAL_W; x++) {
            setPixel(ctxPixels, x, y, colors[line[x]]);
        }
    }
}

function updateAttrib() {
    drawVerticalGrid(ctxAttrib);

    for (let y = 0; y < LOGICAL_H; y++) {
        const line = gfx.getLineAtt(y);
        for (let col = 0; col < LOGICAL_W / 6; col++) {
            const mot = line[col];
            const attInvert = !!(mot & 0x80);
            let attFlag = NOIP;

            if ((mot & 0x60) === 0) {
                attFlag = (mot & 0x10) ? PAPER : INK;
            }

            draw_5px_horizontal(
                ctxAttrib,
                col * 6,
                y,
                mot & 7,
                attFlag,
                attInvert
            );
        }
    }
}

function updateAll() {
    updatePixels();
    updateAttrib();
}

// ---------------------------
// États
// ---------------------------

let showAttrib = false;
let attFlag = NOIP;
let codeFlag = 0;
let attInvert = false;

let leftDown = false;
let rightDown = false;

// ---------------------------
// Gestion souris
// ---------------------------

canvas.addEventListener("contextmenu", e => e.preventDefault());

canvas.addEventListener("mousedown", e => {
    const rect = canvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / ZOOM);
    const y = Math.floor((e.clientY - rect.top) / ZOOM);

    if (e.button === 0) leftDown = true;
    if (e.button === 2) rightDown = true;

    paint(x, y);
});

canvas.addEventListener("mouseup", e => {
    if (e.button === 0) leftDown = false;
    if (e.button === 2) rightDown = false;
});

canvas.addEventListener("mousemove", e => {
    if (!leftDown && !rightDown) return;

    const rect = canvas.getBoundingClientRect();
    const x = Math.floor((e.clientX - rect.left) / ZOOM);
    const y = Math.floor((e.clientY - rect.top) / ZOOM);

    paint(x, y);
});

// ---------------------------
// Fonction paint()
// ---------------------------

function paint(x, y) {
    if (x < 0 || x >= LOGICAL_W || y < 0 || y >= LOGICAL_H) return;

    if (!showAttrib) {
        if (leftDown) gfx.setPixel(x, y);
        if (rightDown) gfx.clearPixel(x, y);

        const pixelsLine = gfx.getLinePx(y);
        for (let px = 0; px < LOGICAL_W; px++) {
            setPixel(ctxPixels, px, y, colors[pixelsLine[px]]);
        }
    }
    else {
        if (leftDown) gfx.setAttribut(x, y, codeFlag, attFlag, attInvert);
        if (rightDown) gfx.resetAttribut(x, y);

        updateAttrib();

        const pixelsLine = gfx.getLinePx(y);
        for (let px = 0; px < LOGICAL_W; px++) {
            setPixel(ctxPixels, px, y, colors[pixelsLine[px]]);
        }
    }
}

// ---------------------------
// UI
// ---------------------------

const btnToggleAttrib = document.getElementById("btnToggleAttrib");
const btnInk = document.getElementById("btnInk");
const btnPaper = document.getElementById("btnPaper");
const btnNoip = document.getElementById("btnNoip");
const btnInvert = document.getElementById("btnInvert");
const btnSaveTap = document.getElementById("btnSaveTap");
const inputLoadBin = document.getElementById("inputLoadBin");
const colorButtonsContainer = document.getElementById("colorButtons");

// 🔥 Ajout du checkbox header
const chkHeader = document.getElementById("header");

// 🔥 Mise à jour dynamique du bouton
chkHeader.addEventListener("change", () => {
    btnSaveTap.textContent = chkHeader.checked ? "Sauver TAP" : "Sauver BIN";
});

btnToggleAttrib.addEventListener("click", () => {
    showAttrib = !showAttrib;
    btnToggleAttrib.textContent = showAttrib ? "Afficher pixels" : "Afficher attributs";
});

btnInk.onclick = () => attFlag = INK;
btnPaper.onclick = () => attFlag = PAPER;
btnNoip.onclick = () => attFlag = NOIP;

btnInvert.onclick = () => {
    attInvert = !attInvert;
    btnInvert.textContent = `Invert: ${attInvert ? "ON" : "OFF"}`;
};

// 🔥 Sauvegarde TAP/BIN dynamique
btnSaveTap.onclick = () => {
    const addHeader = chkHeader.checked;
    const blob = gfx.saveTap(addHeader);

    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;

    a.download = addHeader ? "screen.tap" : "screen.bin";

    a.click();
    URL.revokeObjectURL(url);
};

inputLoadBin.onchange = e => {
    const file = e.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = evt => {
        gfx.loadBin(evt.target.result);
        updateAll();
    };
    reader.readAsArrayBuffer(file);
};

// Boutons couleurs 0–7
for (let i = 0; i < 8; i++) {
    const btn = document.createElement("button");
    btn.className = "color-btn";
    btn.style.background = `rgb(${colors[i][0]},${colors[i][1]},${colors[i][2]})`;
    btn.onclick = () => {
        codeFlag = i;
        [...colorButtonsContainer.children].forEach((b, idx) =>
            b.classList.toggle("active", idx === i)
        );
    };
    colorButtonsContainer.appendChild(btn);
}

colorButtonsContainer.children[0].classList.add("active");

// ---------------------------
// Rendu
// ---------------------------

function render() {
    ctxMain.clearRect(0, 0, SCREEN_W, SCREEN_H);
    ctxMain.drawImage(showAttrib ? attribCanvas : pixelsCanvas, 0, 0, SCREEN_W, SCREEN_H);
    requestAnimationFrame(render);
}

updateAll();
render();
