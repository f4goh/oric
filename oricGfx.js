// ---------------------------------------------
//  ORIC GRAPHICS ENGINE - Browser Version
// ---------------------------------------------

export const ORIC_WIDTH = 240;
export const ORIC_HEIGHT = 200;

export const INK = 0;
export const PAPER = 1;
export const NOIP = 2;
export const INVERTED = true;

const inverted = [7, 6, 5, 4, 3, 2, 1, 0];

export class OricGfx {
    constructor() {
        this.ram = Array.from({ length: ORIC_HEIGHT }, () =>
            new Uint8Array(ORIC_WIDTH / 6).fill(0x40)
        );

        this.pixels = Array.from({ length: ORIC_HEIGHT }, () =>
            new Uint8Array(ORIC_WIDTH).fill(0)
        );

        this.ptr = 0;
    }

    affiche() {
        for (let y = 0; y < 5; y++) {
            const line = this.ram[y];
            let hexLine = "";
            for (let n = 0; n < line.length / 2; n++) {
                hexLine += line[n].toString(16).padStart(2, "0").toUpperCase() + " ";
            }
            console.log(`L ${String(y).padStart(2, "0")}: ${hexLine}`);
        }
    }

    getLinePx(line) {
        return this.pixels[line];
    }

    getLineAtt(line) {
        return this.ram[line];
    }

    mapSixPaper(line, paper, invert) {
        const ncoul = invert ? inverted[paper] : paper;
        for (let n = 0; n < 6; n++) {
            this.pixels[line][this.ptr++] = ncoul;
        }
    }

    mapSixPixel(line, paper, ink, invert, pixel) {
        const ncoulPaper = invert ? inverted[paper] : paper;
        const ncoulInk = invert ? inverted[ink] : ink;

        let mask = 0x20;
        for (let n = 0; n < 6; n++) {
            this.pixels[line][this.ptr++] =
                (pixel & mask) ? ncoulInk : ncoulPaper;
            mask >>= 1;
        }
    }

    convertLine(line) {
        let ink = 7;
        let paper = 0;

        this.ptr = 0;

        for (let mot of this.ram[line]) {
            const invert = mot & 0x80;

            if ((mot & 0x60) === 0) {
                const attribut = mot & 0x1F;

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
        const octet = Math.floor(x / 6);
        const mask = 1 << (5 - (x % 6));

        const val = this.ram[y][octet];
        if ((val & 0x40) === 0x40) {
            this.ram[y][octet] |= mask;
        }
        this.convertLine(y);
    }

    clearPixel(x, y) {
        const octet = Math.floor(x / 6);
        const mask = ~(1 << (5 - (x % 6))) & 0xFF;

        const val = this.ram[y][octet];
        if ((val & 0x40) === 0x40) {
            this.ram[y][octet] &= mask;
        }
        this.convertLine(y);
    }

    setAttribut(x, y, coul, attFlag, attInvert) {
        const octet = Math.floor(x / 6);

        if (attInvert === INVERTED) {
            this.ram[y][octet] |= 0x80;
        } else {
            this.ram[y][octet] &= 0x7F;
        }

        if (attFlag !== NOIP) {
            this.ram[y][octet] &= 0x80;

            if (attFlag === PAPER) {
                this.ram[y][octet] |= (coul + 16);
            } else if (attFlag === INK) {
                this.ram[y][octet] |= coul;
            }
        }

        this.convertLine(y);
    }

    resetAttribut(x, y) {
        const octet = Math.floor(x / 6);
        this.ram[y][octet] = 0x40;
        this.convertLine(y);
    }

	// ---------------------------------------------
	//  SAVE TAP (Browser version)
	// ---------------------------------------------
	saveTap(addHeader = true) {
		const header = new Uint8Array([
			0x16, 0x16, 0x16, 0x24, 0x00, 0xFF, 0x00, 0xC7, 0x05, 0x11, 0x05, 0x01, 0x00,
			0x48, 0x49, 0x52, 0x4C, 0x4F, 0x41, 0x44, 0x00, 0x07, 0x05, 0x0A, 0x00, 0xA2,
			0x00, 0x0F, 0x05, 0x14, 0x00, 0xB6, 0x22, 0x22, 0x00, 0x00, 0x00, 0x55,
			0x16, 0x16, 0x16, 0x24, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x3F, 0xA0, 0x00,
			0x00, 0x00
		]);

		const body = new Uint8Array(ORIC_HEIGHT * (ORIC_WIDTH / 6));
		let idx = 0;

		for (let y = 0; y < ORIC_HEIGHT; y++) {
			body.set(this.ram[y], idx);
			idx += ORIC_WIDTH / 6;
		}

		// Sélection du contenu selon addHeader
		const content = addHeader ? [header, body] : [body];

		return new Blob(content, { type: "application/octet-stream" });
	}

    // ---------------------------------------------
    //  LOAD BIN (Browser version)
    // ---------------------------------------------
    loadBin(buffer) {
        const data = buffer instanceof Uint8Array ? buffer : new Uint8Array(buffer);

        if (data.length !== 8000) {
            throw new Error(`Le fichier doit faire 8000 octets, reçu ${data.length}`);
        }

        let index = 0;
        for (let y = 0; y < ORIC_HEIGHT; y++) {
            for (let x = 0; x < ORIC_WIDTH / 6; x++) {
                this.ram[y][x] = data[index++];
            }
            this.convertLine(y);
        }
    }

    dumpHex() {
        for (let y = 0; y < ORIC_HEIGHT; y++) {
            console.log([...this.ram[y]].map(v => v.toString(16).padStart(2, "0")).join(" "));
        }
    }
}
