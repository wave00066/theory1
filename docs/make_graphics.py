#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""THEORY-1's README pictures, from the host simulator's own screens:

    python3 base/tools/build.py app --host
    python3 docs/make_graphics.py            # docs/img/*.png (English) and *-it.png (Italian)

Needs Pillow. The screens are made fresh by app/build/host/theory1_host (app/tests/screens.txt and flow.txt)."""
import os, subprocess, tempfile
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageFilter

ROOT = Path(__file__).resolve().parent.parent
HOST = ROOT / "app/build/host/theory1_host"
OUT = ROOT / "docs/img"
FONTS = ROOT / "base/assets/fonts"
BG0, BG1 = (12, 17, 28), (26, 40, 66)
CYAN, ORANGE, GREEN, GRAY, WHITE = (70, 170, 255), (255, 150, 40), (80, 220, 120), (150, 160, 175), (240, 244, 250)
PURPLE = (190, 110, 255)
LANGS = ["Italiano", "English", "Español", "Deutsch", "Français"]


def font(size, weight="Bold"):
    return ImageFont.truetype(str(FONTS / f"BarlowSemiCondensed-{weight}.ttf"), size)


def screens(lang, script):
    """the host's screens for a script, in a language: {name: Image}"""
    d = Path(tempfile.mkdtemp())
    s = d / "s.txt"
    s.write_text(f"lang {lang}\n" + (ROOT / script).read_text())
    subprocess.run([str(HOST), str(s), str(d)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return {p.stem: Image.open(p).convert("RGB") for p in d.glob("*.png")}


def gradient(w, h):
    im = Image.new("RGB", (w, h))
    px = im.load()
    for y in range(h):
        for x in range(w):
            t = (x / w * 0.6 + y / h * 0.4)
            px[x, y] = tuple(int(a + (b - a) * t) for a, b in zip(BG0, BG1))
    return im


def rounded(im, r):
    m = Image.new("L", im.size, 0)
    ImageDraw.Draw(m).rounded_rectangle((0, 0, im.size[0] - 1, im.size[1] - 1), r, fill=255)
    out = Image.new("RGBA", im.size)
    out.paste(im, (0, 0), m)
    return out


def shadow_paste(base, im, xy, r=18, blur=18, off=8):
    """im (rounded) on base with a soft shadow"""
    sh = Image.new("RGBA", (im.size[0] + 4 * blur, im.size[1] + 4 * blur), (0, 0, 0, 0))
    ImageDraw.Draw(sh).rounded_rectangle((2 * blur, 2 * blur, 2 * blur + im.size[0], 2 * blur + im.size[1]), r,
                                         fill=(0, 0, 0, 150))
    sh = sh.filter(ImageFilter.GaussianBlur(blur))
    base.alpha_composite(sh, (xy[0] - 2 * blur + off // 2, xy[1] - 2 * blur + off))
    base.alpha_composite(rounded(im, r).convert("RGBA"), xy)


def device(screen, scale=1.5):
    """a stylised FM-1: a dark body, the screen, a strip of keys"""
    sw = int(240 * scale)
    pad = int(22 * scale)
    w, h = sw + 2 * pad, sw + int(110 * scale)
    body = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(body)
    d.rounded_rectangle((0, 0, w - 1, h - 1), int(26 * scale), fill=(34, 36, 42), outline=(70, 74, 84), width=2)
    for i in range(4):                                          # four knobs
        cx = pad + int((i + 0.5) * sw / 4)
        d.ellipse((cx - 9 * scale, pad // 2 - 2, cx + 9 * scale, pad // 2 + 16 * scale), fill=(60, 63, 72))
    body.paste(screen.resize((sw, sw), Image.NEAREST), (pad, int(pad * 1.4)))
    ky = int(pad * 1.4) + sw + int(18 * scale)
    kw = sw / 16
    lit = {0: ORANGE, 2: CYAN, 4: CYAN, 5: CYAN, 7: CYAN, 9: CYAN, 11: CYAN}
    for i in range(16):                                         # white keys F..G, C major lit
        pc = [5, 7, 9, 11, 0, 2, 4, 5, 7, 9, 11, 0, 2, 4, 5, 7][i]
        c = lit.get(pc, (200, 202, 210))
        d.rounded_rectangle((pad + i * kw + 1, ky, pad + (i + 1) * kw - 2, ky + int(46 * scale)), 3, fill=c)
    for i in (0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14):
        x = pad + (i + 1) * kw - kw * 0.32
        d.rounded_rectangle((x, ky, x + kw * 0.64, ky + int(28 * scale)), 2, fill=(20, 21, 25))
    return body


def text(d, xy, s, f, fill, anchor="la"):
    d.text(xy, s, font=f, fill=fill, anchor=anchor)


def banner(scr, title_sub, chips, out):
    W, H = 1280, 420
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    for k in range(5):                                          # a faint staff
        y = 250 + k * 14
        d.line((60, y, 700, y), fill=(255, 255, 255, 18), width=1)
    text(d, (60, 70), "THEORY-1", font(118), WHITE)
    text(d, (64, 196), title_sub, font(38, "Medium"), CYAN)
    x = 64
    for c, col in chips:
        w = d.textlength(c, font=font(26)) + 32
        d.rounded_rectangle((x, 330, x + w, 372), 21, fill=col + (255,))
        text(d, (x + w / 2, 351), c, font(26), (12, 17, 28), "mm")
        x += w + 12
    dev = device(scr, 1.2)
    shadow_paste(im, dev, (W - dev.size[0] - 70, (H - dev.size[1]) // 2 + 6), r=30)
    im.convert("RGB").save(out, optimize=True)


def card_row(items, out, W=1280):
    """items: (title, colour, screen, caption): three cards side by side"""
    H = 640
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    cw = (W - 4 * 40) // 3
    for i, (title, col, scr, cap) in enumerate(items):
        x = 40 + i * (cw + 40)
        d.rounded_rectangle((x, 30, x + cw, H - 30), 24, fill=(20, 27, 42, 255), outline=(52, 64, 88), width=2)
        d.rounded_rectangle((x, 30, x + cw, 40), 4, fill=col + (255,))
        text(d, (x + cw / 2, 78), title, font(40), col, "mm")
        s = scr.resize((330, 330), Image.NEAREST)
        shadow_paste(im, s.convert("RGBA"), (x + (cw - 330) // 2, 118), r=14, blur=10)
        yy = 478
        for line in cap:
            text(d, (x + cw / 2, yy), line, font(25, "Medium"), WHITE, "mm")
            yy += 34
    im.convert("RGB").save(out, optimize=True)


def flow(shots, labels, out, W=1280):
    H = 470
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    n = len(shots)
    sw = 260
    gap = (W - n * sw) // (n + 1)
    for i, (s, (lab, col)) in enumerate(zip(shots, labels)):
        x = gap + i * (sw + gap)
        d.ellipse((x + sw / 2 - 22, 22, x + sw / 2 + 22, 66), fill=col + (255,))
        text(d, (x + sw / 2, 44), str(i + 1), font(30), (12, 17, 28), "mm")
        shadow_paste(im, s.resize((sw, sw), Image.NEAREST).convert("RGBA"), (x, 86), r=12, blur=10)
        yy = 380
        for line in lab:
            text(d, (x + sw / 2, yy), line, font(24, "Medium"), WHITE, "mm")
            yy += 32
        if i < n - 1:                                           # an arrow to the next
            ax = x + sw + 14
            d.line((ax, 216, ax + gap - 28, 216), fill=GRAY + (255,), width=4)
            d.polygon([(ax + gap - 28, 206), (ax + gap - 14, 216), (ax + gap - 28, 226)], fill=GRAY + (255,))
    im.convert("RGB").save(out, optimize=True)


def languages(shots, out):
    W, H = 1300, 380
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    for i, s in enumerate(shots):
        x = 20 + i * 256
        shadow_paste(im, s.convert("RGBA"), (x + 4, 30), r=12, blur=8)
        text(d, (x + 124, 300), LANGS[i], font(30), WHITE, "mm")
        text(d, (x + 124, 336), ["Re (D) maggiore", "Re (D) major", "Re (D) mayor", "Re (D) Dur", "Ré (D) majeur"][i],
             font(22, "Medium"), CYAN, "mm")
    im.convert("RGB").save(out, optimize=True)


def clefs(shots, names, title, out):
    """middle C in the seven clefs: the staff part of each lesson screen, four and three to a row"""
    W, H = 1280, 470
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    text(d, (W / 2, 36), title, font(36), WHITE, "mm")
    for i, s in enumerate(shots):
        row, col = i // 4, i % 4
        n = 4 if row == 0 else 3
        crop = s.crop((0, 74, 240, 186))
        x0 = (W - n * 290) // 2
        x, y = x0 + col * 290 + 25, 74 + row * 200
        shadow_paste(im, crop.convert("RGBA"), (x, y), r=10, blur=8)
        text(d, (x + 120, y + 136), names[i], font(28), CYAN, "mm")
    im.convert("RGB").save(out, optimize=True)


def gallery(items, out):
    """items: (screen, caption) eight of them, 4 x 2"""
    W, H = 1280, 760
    im = gradient(W, H).convert("RGBA")
    d = ImageDraw.Draw(im)
    for i, (s, cap) in enumerate(items):
        c, r = i % 4, i // 4
        x, y = 40 + c * 305, 30 + r * 370
        shadow_paste(im, s.resize((270, 270), Image.NEAREST).convert("RGBA"), (x + 6, y), r=12, blur=8)
        text(d, (x + 141, y + 300), cap, font(25, "Medium"), WHITE, "mm")
    im.convert("RGB").save(out, optimize=True)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    sc = {l: screens(l, "app/tests/screens.txt") for l in range(5)}
    fl = {l: screens(l, "app/tests/flow.txt") for l in (0, 1)}
    cl = {l: screens(l, "app/tests/clefs.txt") for l in (0, 1)}
    sy = {l: screens(l, "app/tests/synth_shots.txt") for l in (0, 1)}
    for lang, sfx in ((1, ""), (0, "-it")):
        s, f = sc[lang], fl[lang]
        en = lang == 1
        banner(s["01-menu"], "Music theory on the M-VAVE FM-1" if en else "Teoria musicale sull'M-VAVE FM-1",
               [("LESSONS" if en else "LEZIONI", CYAN), ("QUIZ", ORANGE), ("LAB" if en else "LABORATORIO", GREEN),
                ("SYNTH" if en else "SINTESI", PURPLE), ("5 " + ("LANGUAGES" if en else "LINGUE"), (200, 200, 210))],
               OUT / f"banner{sfx}.png")
        card_row([("LESSONS" if en else "LEZIONI", CYAN, s["10-armonia"],
                   ["Scales, modes, chords, harmony,", "keys: from any root"] if en else
                   ["Scale, modi, accordi, armonia,", "tonalità: da qualsiasi tonica"]),
                  ("QUIZ", ORANGE, s["16-quiz-giusto"],
                   ["Answer on the keys or by ear,", "3 levels, scores and records"] if en else
                   ["Rispondi sui tasti o a orecchio,", "3 livelli, punti e record"]),
                  ("LAB" if en else "LABORATORIO", GREEN, s["20-lab-rivolto"],
                   ["Play: it names the note,", "the interval, the chord"] if en else
                   ["Suona: ti dice la nota,", "l'intervallo, l'accordo"])], OUT / f"features{sfx}.png")
        flow([f["q1"], f["q2"], f["q3"], f["q4"]],
             [(["The question", "and the keys to use"] if en else ["La domanda", "e i tasti da usare"], CYAN),
              (["A wrong note:", "named, in red"] if en else ["Una nota sbagliata:", "detta, in rosso"], (240, 80, 70)),
              (["The right notes", "fill in, in green"] if en else ["Le note giuste", "si riempiono in verde"], GREEN),
              (["Right: points,", "and the answer heard"] if en else ["Giusto: punti", "e la risposta suonata"],
               ORANGE)], OUT / f"quiz-flow{sfx}.png")
        caps = (["The staff", "Bass clef", "Intervals", "Modes", "Chords", "Keys", "Staff-reading quiz", "Ear: notes"]
                if en else ["Pentagramma", "Chiave di basso", "Intervalli", "Modi", "Accordi", "Tonalità",
                            "Quiz di lettura", "Orecchio: note"])
        gallery(list(zip([s[k] for k in ("03b-pentagramma", "03d-chiave-basso", "04-intervallo", "08-modo", "09-accordo",
                                          "12-tonalita", "18c-quiz-lettura-giusto", "18d-quiz-orecchio-note")], caps)),
                OUT / f"gallery{sfx}.png")
        names = (["Treble", "Bass", "Soprano", "Mezzo", "Alto", "Tenor", "Baritone"] if en else
                 ["Violino", "Basso", "Soprano", "Mezzosoprano", "Contralto", "Tenore", "Baritono"])
        clefs([cl[lang][f"c{i}"] for i in range(7)], names,
              "Middle C in the seven clefs" if en else "Il Do centrale nelle sette chiavi", OUT / f"clefs{sfx}.png")
        y = sy[lang]
        gallery(list(zip([y[k] for k in ("s1-list", "s2-sound", "s3-osc", "s4-filter", "s5-adsr", "s6-lfo", "s7-fm",
                                          "s9-bass")],
                         ["The chapters", "Sound", "The oscillator", "The filter", "ADSR envelope", "LFO",
                          "FM synthesis", "Let's build a bass"] if en else
                         ["I capitoli", "Il suono", "L'oscillatore", "Il filtro", "Inviluppo ADSR", "LFO",
                          "Sintesi FM", "Costruiamo un basso"])), OUT / f"synth{sfx}.png")
    languages([sc[l]["05-scala"] for l in range(5)], OUT / "languages.png")


if __name__ == "__main__":
    main()
