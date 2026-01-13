/* * Oric GFX Editor - Web Port
 * Logic based on C++ original by F4GOH
 */

const ORIC_WIDTH = 240;
const ORIC_HEIGHT = 200;
const BYTES_PER_LINE = 40; 

// Colors map
const COLORS = [
    [0, 0, 0],       // 0: Noir
    [255, 0, 0],     // 1: Rouge
    [0, 255, 0],     // 2: Vert
    [255, 255, 0],   // 3: Jaune
    [0, 0, 255],     // 4: Bleu
    [255, 0, 255],   // 5: Magenta
    [0, 255, 255],   // 6: Cyan
    [255, 255, 255]  // 7: Blanc
];

const GRAY = [80, 80, 80];
const GRAY_LIGHT = [150, 150, 150];

const PAINT_SCREEN = { PIXELS: 0, ATTRIBUTES: 1, PIXATTRIB: 2 };
const ATTR_FLAG = { INK: 0, PAPER: 1, NOIP: 2 };
const INVERTED_MAP = [7, 6, 5, 4, 3, 2, 1, 0];

// --- Core Logic Class ---
class OricGfx {
    constructor() {
        this.ram = new Uint8Array(ORIC_HEIGHT * BYTES_PER_LINE);
        this.pixels = new Int32Array(ORIC_HEIGHT * ORIC_WIDTH);
        this.ptr = 0;
        this.clearAll();
    }

    clearAll() {
        this.ram.fill(0x40);
        this.pixels.fill(0);
    }

    getRamIndex(x, y) { return y * BYTES_PER_LINE + x; }
    getPixelIndex(x, y) { return y * ORIC_WIDTH + x; }

    mapSixPaper(line, paper, invert) {
        let nCoul = invert ? INVERTED_MAP[paper] : paper;
        let baseIdx = this.getPixelIndex(0, line) + this.ptr;
        for (let n = 0; n < 6; ++n) {
            this.pixels[baseIdx + n] = nCoul;
        }
        this.ptr += 6;
    }

    mapSixPixel(line, paper, ink, invert, pixel) {
        let nCoulPaper = invert ? INVERTED_MAP[paper] : paper;
        let nCoulInk = invert ? INVERTED_MAP[ink] : ink;
        let mask = 0x20;
        let baseIdx = this.getPixelIndex(0, line) + this.ptr;
        
        for (let n = 0; n < 6; ++n) {
            this.pixels[baseIdx + n] = (pixel & mask) ? nCoulInk : nCoulPaper;
            mask >>= 1;
        }
        this.ptr += 6;
    }

    convertLine(line) {
        let ink = 7; 
        let paper = 0;
        this.ptr = 0;

        for (let col = 0; col < BYTES_PER_LINE; ++col) {
            let mot = this.ram[this.getRamIndex(col, line)];
            let invert = (mot & 0x80) !== 0;

            if ((mot & 0x60) === 0) {
                let attribut = mot & 0x1F;
                if ((attribut & 0x10) === 0) {
                    ink = attribut & 0x07;
                    this.mapSixPaper(line, paper, invert);
                } else {
                    paper = attribut & 0x07;
                    this.mapSixPaper(line, paper, invert);
                }
            } else {
                this.mapSixPixel(line, paper, ink, invert, mot & 0x3F);
            }
        }
    }

    setPixel(x, y) {
        let octet = Math.floor(x / 6);
        let bitPos = 5 - (x % 6);
        let msk = 1 << bitPos;
        let idx = this.getRamIndex(octet, y);
        
        if (this.ram[idx] & 0x40) {
            this.ram[idx] |= msk;
        }
        this.convertLine(y);
    }

    clearPixel(x, y) {
        let octet = Math.floor(x / 6);
        let bitPos = 5 - (x % 6);
        let msk = ~(1 << bitPos) & 0xFF;
        let idx = this.getRamIndex(octet, y);

        if (this.ram[idx] & 0x40) {
            this.ram[idx] &= msk;
        }
        this.convertLine(y);
    }

    setAttribut(x, y, coul, attFlag, attInvert) {
        let octet = Math.floor(x / 6);
        let idx = this.getRamIndex(octet, y);
        
        if (attInvert) {
            this.ram[idx] |= 0x80;
        } else {
            this.ram[idx] &= 0x7F;
        }

        if (attFlag !== ATTR_FLAG.NOIP) {
            if (attFlag === ATTR_FLAG.PAPER) {
                coul += 16;
            }
            this.ram[idx] &= 0x80; 
            this.ram[idx] |= coul;
        }
        this.convertLine(y);
    }

    resetAttribut(x, y) {
        let octet = Math.floor(x / 6);
        let idx = this.getRamIndex(octet, y);
        this.ram[idx] = 0x40;
        this.convertLine(y);
    }

    getTapHeader() {
        return new Uint8Array([
            0x16, 0x16, 0x16, 0x24, 0x00, 0xFF, 0x00, 0xC7, 0x05, 0x11, 0x05, 0x01, 0x00,
            0x48, 0x49, 0x52, 0x4C, 0x4F, 0x41, 0x44, 0x00, 0x07, 0x05, 0x0A, 0x00, 0xA2,
            0x00, 0x0F, 0x05, 0x14, 0x00, 0xB6, 0x22, 0x22, 0x00, 0x00, 0x00, 0x55,
            0x16, 0x16, 0x16, 0x24, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x3F, 0xA0, 0x00,
            0x00, 0x00
        ]);
    }

    exportRectHex(x, y, w, h) {
        let output = "";
        for (let yy = y; yy < y + h; ++yy) {
            output += "\t.byt ";
            let lineBytes = [];
            for (let xx = x; xx < x + w; ++xx) {
                let v = this.ram[this.getRamIndex(xx, yy)];
                lineBytes.push("$" + v.toString(16).toUpperCase().padStart(2, '0'));
            }
            output += lineBytes.join(",") + "\n";
        }
        return output;
    }
}

// --- Main Application UI Class ---
class MainWindow {
    constructor() {
        this.gfx = new OricGfx();
        this.canvas = document.getElementById('oricCanvas');
        this.ctx = this.canvas.getContext('2d', { willReadFrequently: true, alpha: false });
        
        // Buffers
        this.bufferPixels = new ImageData(ORIC_WIDTH, ORIC_HEIGHT);
        this.bufferAttrib = new ImageData(ORIC_WIDTH, ORIC_HEIGHT);
        
        // Final buffer used for rendering to avoid sync issues
        this.renderBuffer = new ImageData(ORIC_WIDTH, ORIC_HEIGHT);

        // State
        this.showAttrib = PAINT_SCREEN.PIXELS;
        this.selectAttrib = PAINT_SCREEN.PIXELS;
        this.attFlag = ATTR_FLAG.INK;
        this.codeFlag = 0;
        this.attInvert = false;
        
        // Render loop management to prevent stuttering
        this.dirty = true; // Force initial draw

        this.initUI();
        this.initBuffers();
        
        // Start Render Loop
        this.startRenderLoop();
    }

    // New optimized render loop
    startRenderLoop() {
        const loop = () => {
            if (this.dirty) {
                this.refreshScreen();
                this.dirty = false;
            }
            requestAnimationFrame(loop);
        };
        requestAnimationFrame(loop);
    }

    initUI() {
        document.querySelectorAll('.color-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                this.codeFlag = parseInt(e.target.dataset.code);
                this.updateInkPaperStyle();
            });
        });

        document.getElementById('btnNoIp').onclick = () => { this.attFlag = ATTR_FLAG.NOIP; this.updateInkPaperStyle(); };
        document.getElementById('btnPaper').onclick = () => { this.attFlag = ATTR_FLAG.PAPER; this.updateInkPaperStyle(); };
        document.getElementById('btnInk').onclick = () => { this.attFlag = ATTR_FLAG.INK; this.updateInkPaperStyle(); };
        
        document.getElementById('checkInvert').onchange = (e) => { this.attInvert = e.target.checked; };
        document.getElementById('btnClear').onclick = () => this.onClear();

        const comboPaint = document.getElementById('comboPaintSelect');
        const comboSelect = document.getElementById('comboSelect');
        
        comboPaint.onchange = (e) => {
            this.showAttrib = parseInt(e.target.value);
            if (this.showAttrib === 2) { 
                comboSelect.disabled = false;
                this.selectAttrib = parseInt(comboSelect.value);
            } else {
                comboSelect.disabled = true;
                comboSelect.value = this.showAttrib;
                this.selectAttrib = this.showAttrib;
            }
            this.dirty = true;
        };

        comboSelect.onchange = (e) => { this.selectAttrib = parseInt(e.target.value); };

        document.getElementById('checkHeader').onchange = (e) => {
            document.getElementById('btnSaveTap').textContent = e.target.checked ? "Save TAP" : "Save BIN";
        };

        document.getElementById('btnSaveTap').onclick = () => this.saveBinTap();
        document.getElementById('btnLoadBin').onclick = () => document.getElementById('fileInputBin').click();
        document.getElementById('fileInputBin').onchange = (e) => this.loadBin(e);

        document.getElementById('btnSavePng').onclick = () => this.savePng();
        document.getElementById('btnLoadPng').onclick = () => document.getElementById('fileInputPng').click();
        document.getElementById('fileInputPng').onchange = (e) => this.loadPng(e);

        document.getElementById('btnAbout').onclick = () => alert("Oric GFX Editor Web Port\nOriginal by F4GOH\nhttps://github.com/f4goh/oric\nhttps://ceo.oric.org/");

        document.getElementById('btnExport').onclick = () => document.getElementById('exportModal').style.display = "block";
        document.getElementById('btnExpCancel').onclick = () => document.getElementById('exportModal').style.display = "none";
        document.getElementById('btnExpOk').onclick = () => this.doExport();

        this.isDrawing = false;
        this.canvas.addEventListener('mousedown', (e) => this.handleMouse(e));
        this.canvas.addEventListener('mousemove', (e) => this.handleMouse(e));
        this.canvas.addEventListener('mouseup', () => this.isDrawing = false);
        this.canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        window.addEventListener('keydown', (e) => this.handleKey(e));

        this.updateInkPaperStyle();
    }

    initBuffers() {
        this.gfx.clearAll();
        this.drawVerticalGrid(this.bufferAttrib);
        this.updateAll();
    }

    drawVerticalGrid(imgData) {
        const data = imgData.data;
        data.fill(0); 
        for(let i=0; i<data.length; i+=4) { data[i]=0; data[i+1]=0; data[i+2]=0; data[i+3]=255; } // Black background

        for (let y = 0; y < ORIC_HEIGHT; y++) {
            for (let x = 0; x < ORIC_WIDTH; x++) {
                let idx = (y * ORIC_WIDTH + x) * 4;
                if (x % 6 === 0 || y % 8 === 0) {
                    data[idx] = 80; data[idx+1] = 80; data[idx+2] = 80;
                }
            }
        }
    }

    draw5pxHorizontal(imgData, x, y, colorIndex, attFlag, attInvert) {
        if (y < 0 || y >= ORIC_HEIGHT) return;
        let baseX = Math.floor(x / 6) * 6 + 1;
        if (baseX < 0 || baseX + 4 >= ORIC_WIDTH) return;

        let col = COLORS[colorIndex];
        let data = imgData.data;

        let drawLine = (x1, x2, r, g, b) => {
            for(let xx=x1; xx<=x2; xx++) {
                let idx = (y * ORIC_WIDTH + xx) * 4;
                data[idx] = r; data[idx+1] = g; data[idx+2] = b;
            }
        };

        if (attFlag === ATTR_FLAG.INK) {
            drawLine(baseX + 1, baseX + 3, col[0], col[1], col[2]);
        } else if (attFlag === ATTR_FLAG.PAPER) {
            drawLine(baseX, baseX + 4, col[0], col[1], col[2]);
        }

        if (attInvert) {
            drawLine(baseX, baseX + 1, GRAY_LIGHT[0], GRAY_LIGHT[1], GRAY_LIGHT[2]);
        }
    }

    colorFromIndex(idx) {
        return COLORS[idx] || [0,0,0];
    }

    updateAll() {
        const pxData = this.bufferPixels.data;
        for (let y = 0; y < ORIC_HEIGHT; ++y) {
            for (let x = 0; x < ORIC_WIDTH; ++x) {
                let colIdx = this.gfx.pixels[y * ORIC_WIDTH + x];
                let rgb = this.colorFromIndex(colIdx);
                let idx = (y * ORIC_WIDTH + x) * 4;
                pxData[idx] = rgb[0]; pxData[idx+1] = rgb[1]; pxData[idx+2] = rgb[2]; pxData[idx+3] = 255;
            }
            
            for (let col = 0; col < ORIC_WIDTH / 6; ++col) {
                let mot = this.gfx.ram[y * 40 + col];
                let inv = (mot & 0x80) !== 0;
                let flg = ATTR_FLAG.NOIP;
                if ((mot & 0x60) === 0) {
                    flg = (mot & 0x10) ? ATTR_FLAG.PAPER : ATTR_FLAG.INK;
                }
                this.draw5pxHorizontal(this.bufferAttrib, col * 6, y, mot & 0x07, flg, inv);
            }
        }
        this.dirty = true;
    }

    updateScanline(y) {
        if (y < 0 || y >= ORIC_HEIGHT) return;
        const pxData = this.bufferPixels.data;
        for (let x = 0; x < ORIC_WIDTH; ++x) {
            let colIdx = this.gfx.pixels[y * ORIC_WIDTH + x];
            let rgb = this.colorFromIndex(colIdx);
            let idx = (y * ORIC_WIDTH + x) * 4;
            pxData[idx] = rgb[0]; pxData[idx+1] = rgb[1]; pxData[idx+2] = rgb[2]; pxData[idx+3] = 255;
        }
    }

    refreshScreen() {
        // Optimized Refresh: No async operations, no flickering
        const target = this.renderBuffer.data;
        let source;

        if (this.showAttrib === PAINT_SCREEN.PIXELS) {
            source = this.bufferPixels.data;
            target.set(source);
        } 
        else if (this.showAttrib === PAINT_SCREEN.ATTRIBUTES) {
            source = this.bufferAttrib.data;
            target.set(source);
        } 
        else {
            // PIXATTRIB: CompositionMode_Plus (Addition)
            const px = this.bufferPixels.data;
            const at = this.bufferAttrib.data;
            for (let i = 0; i < target.length; i += 4) {
                // R, G, B added together (clamped to 255 automatically by Uint8ClampedArray)
                target[i] = px[i] + at[i];     // R
                target[i+1] = px[i+1] + at[i+1]; // G
                target[i+2] = px[i+2] + at[i+2]; // B
                target[i+3] = 255;             // Alpha
            }
        }

        this.ctx.putImageData(this.renderBuffer, 0, 0);
    }

    handleMouse(e) {
        const rect = this.canvas.getBoundingClientRect();
        let x = Math.floor((e.clientX - rect.left) * (ORIC_WIDTH / rect.width));
        let y = Math.floor((e.clientY - rect.top) * (ORIC_HEIGHT / rect.height));

        if (x < 0 || x >= ORIC_WIDTH || y < 0 || y >= ORIC_HEIGHT) return;

        document.getElementById('labelXY').textContent = `X: ${x}, Y: ${y}`;

        if (e.buttons === 0) return; 

        const evenMode = document.getElementById('comboEven').value;
        if (evenMode == 1 && y % 2 !== 0) return;
        if (evenMode == 2 && y % 2 === 0) return;

        const left = (e.buttons & 1) === 1;
        const right = (e.buttons & 2) === 2;

        if (this.selectAttrib === PAINT_SCREEN.PIXELS) {
            if (left) this.gfx.setPixel(x, y);
            if (right) this.gfx.clearPixel(x, y);
            this.updateScanline(y);
        } else {
            if (left) {
                this.draw5pxHorizontal(this.bufferAttrib, x, y, this.codeFlag, this.attFlag, this.attInvert);
                this.gfx.setAttribut(x, y, this.codeFlag, this.attFlag, this.attInvert);
                this.updateScanline(y);
            }
            if (right) {
                this.draw5pxHorizontal(this.bufferAttrib, x, y, 0, ATTR_FLAG.PAPER, false);
                this.gfx.resetAttribut(x, y);
                this.updateScanline(y);
            }
        }
        
        // Mark for render on next frame (Fix for stuttering)
        this.dirty = true;
    }

    handleKey(e) {
        if (e.target.tagName === 'INPUT') return;
        const k = e.key.toUpperCase();
        if (k >= '0' && k <= '7') {
            this.codeFlag = parseInt(k);
            this.updateInkPaperStyle();
        }
        else if (k === 'P') { this.attFlag = ATTR_FLAG.PAPER; this.updateInkPaperStyle(); }
        else if (k === 'I') { this.attFlag = ATTR_FLAG.INK; this.updateInkPaperStyle(); }
        else if (k === 'N') { this.attFlag = ATTR_FLAG.NOIP; this.updateInkPaperStyle(); }
        else if (k === 'V' && this.showAttrib !== 0) { document.getElementById('checkInvert').click(); }
        else if (k === 'X') { document.getElementById('comboPaintSelect').value = 0; document.getElementById('comboPaintSelect').dispatchEvent(new Event('change')); }
        else if (k === 'A') { document.getElementById('comboPaintSelect').value = 1; document.getElementById('comboPaintSelect').dispatchEvent(new Event('change')); }
        else if (k === 'W') { document.getElementById('comboPaintSelect').value = 2; document.getElementById('comboPaintSelect').dispatchEvent(new Event('change')); }
    }

    updateInkPaperStyle() {
        const btnInk = document.getElementById('btnInk');
        const btnPaper = document.getElementById('btnPaper');
        
        // --- FIX: Button Color Logic ---
        const c = COLORS[this.codeFlag];
        let displayColor;
        
        // Si noir (0), on affiche du gris 80,80,80 pour la lisibilité
        if (this.codeFlag === 0) {
            displayColor = "rgb(80,80,80)";
        } else {
            displayColor = `rgb(${c[0]},${c[1]},${c[2]})`;
        }

        const grayStr = `rgb(240,240,240)`;

        btnInk.style.backgroundColor = grayStr;
        btnPaper.style.backgroundColor = grayStr;
        // On remet la couleur du texte à noir ou blanc selon le contraste si besoin, mais ici le noir suffit
        btnInk.style.color = "black";
        btnPaper.style.color = "black";

        if (this.attFlag === ATTR_FLAG.INK) {
            btnInk.style.backgroundColor = displayColor;
            if (this.codeFlag === 0) btnInk.style.color = "white"; // Texte blanc si fond gris foncé
        }
        if (this.attFlag === ATTR_FLAG.PAPER) {
            btnPaper.style.backgroundColor = displayColor;
             if (this.codeFlag === 0) btnPaper.style.color = "white";
        }
    }

    onClear() {
        this.gfx.clearAll();
        this.initBuffers();
        this.dirty = true;
    }

    saveBinTap() {
        const header = document.getElementById('checkHeader').checked;
        let data = this.gfx.ram; 
        
        let finalBlob;
        if (header) {
            const head = this.gfx.getTapHeader();
            const combined = new Uint8Array(head.length + data.length);
            combined.set(head);
            combined.set(data, head.length);
            finalBlob = new Blob([combined], {type: "application/octet-stream"});
        } else {
            finalBlob = new Blob([data], {type: "application/octet-stream"});
        }

        const a = document.createElement('a');
        a.href = URL.createObjectURL(finalBlob);
        a.download = header ? "screen.tap" : "screen.bin";
        a.click();
    }

    loadBin(e) {
        const file = e.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = (evt) => {
            const buf = new Uint8Array(evt.target.result);
            if (buf.length !== 8000) {
                alert("File must be exactly 8000 bytes (Raw BIN).");
                return;
            }
            this.gfx.ram.set(buf);
            for(let y=0; y<ORIC_HEIGHT; y++) this.gfx.convertLine(y);
            this.updateAll();
        };
        reader.readAsArrayBuffer(file);
        e.target.value = ''; 
    }

    savePng() {
        const a = document.createElement('a');
        const tmp = document.createElement('canvas');
        tmp.width = ORIC_WIDTH; tmp.height = ORIC_HEIGHT;
        tmp.getContext('2d').putImageData(this.bufferPixels, 0, 0);
        a.href = tmp.toDataURL("image/png");
        a.download = "oric_screen.png";
        a.click();
    }

    loadPng(e) {
        const file = e.target.files[0];
        if (!file) return;
        const img = new Image();
        img.onload = () => {
            if (img.width !== ORIC_WIDTH || img.height !== ORIC_HEIGHT) {
                alert(`Image must be ${ORIC_WIDTH}x${ORIC_HEIGHT}`);
                return;
            }
            const tmp = document.createElement('canvas');
            tmp.width = ORIC_WIDTH; tmp.height = ORIC_HEIGHT;
            const ctx = tmp.getContext('2d');
            ctx.drawImage(img, 0, 0);
            const data = ctx.getImageData(0, 0, ORIC_WIDTH, ORIC_HEIGHT).data;
            
            this.onClear();

            for (let y = 0; y < ORIC_HEIGHT; y++) {
                for (let x = 0; x < ORIC_WIDTH; x++) {
                    let i = (y * ORIC_WIDTH + x) * 4;
                    let gray = 0.299*data[i] + 0.587*data[i+1] + 0.114*data[i+2];
                    if (gray >= 128) {
                        this.gfx.setPixel(x, y);
                    }
                }
                this.updateScanline(y);
            }
            this.dirty = true;
        };
        img.src = URL.createObjectURL(file);
        e.target.value = '';
    }

    doExport() {
        const x = parseInt(document.getElementById('expX').value);
        const y = parseInt(document.getElementById('expY').value);
        const w = parseInt(document.getElementById('expW').value);
        const h = parseInt(document.getElementById('expH').value);

        if (x+w > 40 || y+h > 200) {
            alert("Invalid dimensions");
            return;
        }

        const txt = this.gfx.exportRectHex(x, y, w, h);
        const blob = new Blob([txt], {type: "text/plain"});
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = "export.s";
        a.click();
        document.getElementById('exportModal').style.display = "none";
    }
}

window.onload = () => { new MainWindow(); };