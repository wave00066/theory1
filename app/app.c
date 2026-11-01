/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* THEORY-1 teaches music theory on the FM-1: LESSONS (notes, intervals, scales, modes, chords, the chords on every
 * degree of a scale, keys and the circle of fifths), QUIZ (questions answered on the keys or by ear, three levels,
 * a score and a record) and LAB (play anything: it names the note, the interval or the chord). In Italian,
 * English, Spanish, German or French; notes named Do (C), Do, C or the German way.
 * The keys light up with what is on the screen. Notes come from the FM-1's own keys and from any MIDI keyboard
 * (USB, the TRS jack, Bluetooth). Part of the unity build (device/unity.c, host/plat_host.c), after the platform:
 * gfx.c and announce.c are in scope. The theory is theory.c, the words texts.c, the sound synth.c. */
#include "fastmath.h"
#include "synth.h"
#include "theory.c"
#include "texts.c"

/* ================================================================ settings === */
#define SET_MAGIC 0x31454854u            /* "THE1" */
enum { QT_MIXED, QT_NOTE, QT_INTERVAL, QT_SCALE, QT_MODE, QT_CHORD, QT_DEGREE, QT_EAR_INT, QT_EAR_CHORD, QT_KEYSIG,
       NQT };
typedef struct {
    uint32_t magic;
    uint8_t names;                       /* NAMES_*: how notes are named */
    uint8_t sound;                       /* SND_* */
    uint8_t speed;                       /* 0 slow, 1 medium, 2 fast: the examples */
    uint8_t ext_mute;                    /* 1: notes from an outside keyboard are not sounded here */
    uint8_t ble_off;
    uint8_t level;                       /* 0..2 */
    uint8_t qtype;                       /* QT_* */
    uint8_t ui;                          /* L_*: the language */
    uint8_t best[NQT];                   /* each quiz's record, 0..100 */
    uint8_t rsv[6];
} settings_t;
static settings_t cfg;
static uint8_t set_dirty;

static void settings_defaults(void)
{
    uint32_t i;
    uint8_t *p = (uint8_t *)&cfg;
    for (i = 0; i < sizeof cfg; i++)
        p[i] = 0;
    cfg.magic = SET_MAGIC;
    cfg.speed = 1;
}

int theory_ble_off(void) { return cfg.ble_off; }

static void settings_apply(void)
{
    th_names = cfg.names;
    th_ui = cfg.ui;
    synth_sound(cfg.sound);
}

static void settings_save(void)
{
    if (!set_dirty)
        return;
    if (plat_store_save(OBJ_SETTINGS, &cfg, sizeof cfg) == 0)
        set_dirty = 0;
}

/* ================================================================== sound === */
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    synth_render(lr, n, (float)master_q12 / 4096.0f * 8300000.0f);
}

void app_panic(void) { synth_panic(); }

/* the examples: notes at times, played from the main loop */
#define NPEV 96
static struct {
    uint32_t t;
    uint8_t note, on, vel, used;
} pev[NPEV];
static uint8_t pev_sounding[128];

static void play_stop(void)
{
    int i;
    for (i = 0; i < NPEV; i++)
        pev[i].used = 0;
    for (i = 0; i < 128; i++)
        if (pev_sounding[i]) {
            pev_sounding[i] = 0;
            synth_note_off(i);
        }
}

static void play_ev(uint32_t t, int note, int on, int vel)
{
    int i;
    for (i = 0; i < NPEV; i++)
        if (!pev[i].used) {
            pev[i].t = t;
            pev[i].note = (uint8_t)note;
            pev[i].on = (uint8_t)on;
            pev[i].vel = (uint8_t)vel;
            pev[i].used = 1;
            return;
        }
}

static void play_note(uint32_t at, int note, uint32_t dur, int vel)
{
    play_ev(at, note, 1, vel);
    play_ev(at + dur, note, 0, 0);
}

static uint32_t step_ms(void) { return cfg.speed == 0 ? 520u : cfg.speed == 1 ? 360u : 240u; }

static void play_tick(void)
{
    int i;
    uint32_t now = plat_ms();
    for (i = 0; i < NPEV; i++)
        if (pev[i].used && (int32_t)(now - pev[i].t) >= 0) {
            pev[i].used = 0;
            if (pev[i].on) {
                synth_note_on(pev[i].note, pev[i].vel);
                pev_sounding[pev[i].note] = 1;
            } else if (pev_sounding[pev[i].note]) {
                pev_sounding[pev[i].note] = 0;
                synth_note_off(pev[i].note);
            }
        }
}

/* where a root sounds: G3 .. F#4, so the examples sit in the middle of the keyboard */
static int base_midi(nt_t root)
{
    int pc = pc_of(root);
    return 60 + pc - (pc >= 7 ? 12 : 0);
}

/* a scale up and back down */
static void play_scale(const scale_t *s, nt_t root, int updown)
{
    uint32_t t = plat_ms() + 30u, st = step_ms();
    int b = base_midi(root), i;
    for (i = 0; i <= s->n; i++, t += st)
        play_note(t, b + (i < s->n ? s->semi[i] : 12), st - 30u, 90);
    for (i = s->n - 1; updown && i >= 0; i--, t += st)
        play_note(t, b + s->semi[i], i ? st - 30u : st * 2u, 80);
}

/* a chord: its notes one by one, then together */
static void play_chord(int type, nt_t root, uint32_t at, int arp)
{
    uint32_t t = at, st = step_ms();
    int b = base_midi(root), i;
    for (i = 0; arp && i < CHORDS[type].n; i++, t += st)
        play_note(t, b + CHORDS[type].semi[i], st - 30u, 85);
    for (i = 0; i < CHORDS[type].n; i++)
        play_note(t, b + CHORDS[type].semi[i], st * 3u, 80);
}

/* an interval: the two notes one after the other, then together */
static void play_interval(int b, int semis, int together)
{
    uint32_t t = plat_ms() + 30u, st = step_ms() + 120u;
    play_note(t, b, st - 40u, 90);
    play_note(t + st, b + semis, st - 40u, 90);
    if (together) {
        play_note(t + 2u * st, b, st * 2u, 80);
        play_note(t + 2u * st, b + semis, st * 2u, 80);
    }
}

/* ================================================================ screens === */
enum { SCR_MENU, SCR_TOPICS, SCR_LESSON, SCR_QSETUP, SCR_QUIZ, SCR_QEND, SCR_LAB, SCR_SETTINGS };
enum { T_BASICS, T_INTERVALS, T_SCALES, T_MODES, T_CHORDS, T_HARMONY, T_KEYS, NTOPICS };
static const uint8_t TOPIC_ITEMS[NTOPICS] = {8, NINTERVALS, NSCALES, NMODES, NCHORDS, 9, NKEYSIG};

/* the scales of the Harmony lesson: four scales and the five modes that are not one of them */
#define NHSCALES 9
static void hscale(int i, scale_t *out)
{
    static const uint8_t MODE_OF[5] = {1, 2, 3, 4, 6};
    if (i < 4)
        *out = SCALES[i];
    else
        mode_scale(MODE_OF[i - 4], out);
}

/* ===================================================================== UI === */
#define COL_ROOT RGB(255, 150, 40)
#define COL_SCALE RGB(70, 170, 255)
#define COL_OK RGB(80, 220, 120)
#define COL_BAD RGB(240, 80, 70)
#define COL_HELD RGB(255, 225, 90)
#define COL_WKEY RGB(214, 214, 222)
#define COL_BKEY RGB(40, 40, 48)
#define COL_PANEL RGB(24, 30, 42)
#define COL_SELBG RGB(30, 70, 110)
#define COL_ALT RGB(150, 200, 255)             /* the letter names beside Do Re Mi */

static struct {
    uint8_t screen, menu, topic, topic_sel;
    uint8_t item[NTOPICS], root[NTOPICS];
    uint8_t hdeg, hseventh;
    uint8_t qsel, set_sel;
    uint8_t lab_scale, lab_root;         /* lab_scale 0: none, 1..: SCALES, then modes */
    int8_t octave;
    uint32_t keys, btn;
    uint8_t held[128];                   /* notes down, from any source (a count) */
    uint8_t hist[8], nhist;
    char msg[2][40];
    uint32_t msg_until;
    uint8_t looking;                     /* Bluetooth devices: REC pressed, the scan's list up */
    uint8_t redraw;
} ui;

/* --------------------------------------------------------------- the quiz --- */
enum { K_SEQ, K_SET, K_ONE, K_CHOICE };
#define NQ 10
static struct {
    uint8_t type, kind, num, score;
    char l1[64], l2[64], ans[64];        /* the question (two lines) and the answer, written out */
    uint8_t n, pc[12];                   /* the notes asked (pitch classes, in order) */
    nt_t nt[12];                         /* and spelled */
    uint8_t pos;                         /* K_SEQ: the next note */
    uint16_t found;                      /* K_SET: the notes found */
    int8_t ref_pc;                       /* a note given (an interval's root) */
    nt_t ref;
    int8_t wrong_pc;                     /* the last wrong note, shown red */
    uint8_t errors, hints, hint_on;
    uint8_t nopt, correct, sel, gone;    /* K_CHOICE; gone: options struck out by a hint */
    char opt[4][40];
    uint8_t listen[8], nlisten, listen_kind;   /* what an ear question plays: 0 two notes, 1 a chord */
    uint8_t done;                        /* 0 asking, 1 right, 2 wrong or skipped */
    uint32_t done_at;
    uint8_t gained, record;
    char fb[64];                         /* the feedback line */
    uint16_t fb_col;
} q;
static uint32_t t1_rng = 0x12345678u;
static uint32_t rnd(uint32_t n)
{
    t1_rng ^= t1_rng << 13;
    t1_rng ^= t1_rng >> 17;
    t1_rng ^= t1_rng << 5;
    return n ? t1_rng % n : 0;
}

/* ============================================================ text helpers === */
static void cat_int(char *b, int v)
{
    char t[12];
    int n = 0;
    uint32_t u = (uint32_t)(v < 0 ? -v : v);
    do
        t[n++] = (char)('0' + u % 10u);
    while ((u /= 10u) && n < 11);
    while (*b)
        b++;
    if (v < 0)
        *b++ = '-';
    while (n)
        *b++ = t[--n];
    *b = 0;
}

static void int_str(char *b, int v)
{
    b[0] = 0;
    cat_int(b, v);
}

static int t1_bits(uint32_t m)                  /* set bits (no compiler builtin: the chip's runtime may lack one) */
{
    int n = 0;
    for (; m; m &= m - 1u)
        n++;
    return n;
}

static void t1_strcpy(char *d, const char *s)
{
    d[0] = 0;
    str_cat(d, s);
}

static int t1_streq(const char *a, const char *b)
{
    while (*a && *a == *b)
        a++, b++;
    return *a == *b;
}

/* capitals for a header, accented letters too (UTF-8 C3 A0..BE -> C3 80..9E) */
static void t1_upper(char *d, const char *s)
{
    for (; *s; s++, d++) {
        uint8_t c = (uint8_t)*s;
        if (c >= 'a' && c <= 'z')
            c = (uint8_t)(c - 32);
        else if (c >= 0xA0u && c <= 0xBEu && c != 0xB7u && s[-1] == '\xC3')   /* (never the first byte) */
            c = (uint8_t)(c - 32);
        *d = (char)c;
    }
    *d = 0;
}

/* the notes of a list, spelled with the main names: "Re Mi Fa#" */
static void notes_text(const nt_t *n, int count, char *b)
{
    int i;
    char t[8];
    b[0] = 0;
    for (i = 0; i < count; i++) {
        if (i)
            str_cat(b, " ");
        note_name(n[i], t);
        str_cat(b, t);
    }
}
/* the same in letters, when the letters are shown beside: "D E F#"; else "" */
static void notes_alt(const nt_t *n, int count, char *b)
{
    int i;
    char t[8];
    b[0] = 0;
    for (i = 0; th_names == NAMES_BOTH && i < count; i++) {
        if (i)
            str_cat(b, " ");
        note_alt(n[i], t);
        str_cat(b, t);
    }
}

/* ============================================================== drawing === */
/* Text is UTF-8. The fonts have ASCII; an accented letter is its plain letter with the mark drawn above it (an i
 * loses its dot first), and a German sharp s is drawn ss. '\x01' and '\x02' are the roman numerals' small circle
 * (diminished) and crossed circle (half-diminished). */
enum { MK_NONE, MK_GRAVE, MK_ACUTE, MK_CIRC, MK_TILDE, MK_UML, MK_RING, MK_CEDILLA };
static const char LAT_BASE[65] = "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYPsaaaaaaaceeeeiiiidnooooo/ouuuuypy";
static const uint8_t LAT_MARK[64] = {1, 2, 3, 4, 5, 6, 0, 7, 1, 2, 3, 5, 1, 2, 3, 5, 0, 4, 1, 2, 3, 4, 5, 0,
                                     0, 1, 2, 3, 5, 2, 0, 0, 1, 2, 3, 4, 5, 6, 0, 7, 1, 2, 3, 5, 1, 2, 3, 5,
                                     0, 4, 1, 2, 3, 4, 5, 0, 0, 1, 2, 3, 5, 2, 0, 5};

static uint32_t u8next(const char **s)
{
    const uint8_t *p = (const uint8_t *)*s;
    uint32_t c = p[0];
    if ((c == 0xC2u || c == 0xC3u) && (p[1] & 0xC0u) == 0x80u) {
        *s += 2;
        return (c & 0x1Fu) << 6 | (p[1] & 0x3Fu);
    }
    *s += 1;
    return c;
}

/* the first row of a glyph with ink in it */
static int32_t ink_top(const felucca_font_t *f, uint32_t ch)
{
    uint32_t gi = glyph(f, ch), bpr = (f->bw[gi] + 1u) / 2u, gy, b;
    const uint8_t *gd = f->data + f->off[gi];
    for (gy = 0; gy < f->h; gy++)
        for (b = 0; b < bpr; b++)
            if (gd[gy * bpr + b])
                return (int32_t)gy;
    return f->h;
}

/* the last row of a glyph with ink in it */
static int32_t ink_bottom(const felucca_font_t *f, uint32_t ch)
{
    uint32_t gi = glyph(f, ch), bpr = (f->bw[gi] + 1u) / 2u, b;
    int32_t gy;
    const uint8_t *gd = f->data + f->off[gi];
    for (gy = (int32_t)f->h - 1; gy >= 0; gy--)
        for (b = 0; b < bpr; b++)
            if (gd[(uint32_t)gy * bpr + b])
                return gy;
    return 0;
}

/* one glyph from row row0 down (cv_text's drawing, for the i without its dot) */
static void glyph_rows(int32_t x, int32_t y, const felucca_font_t *f, uint32_t ch, uint16_t c, uint32_t row0)
{
    uint32_t gi = glyph(f, ch), w = f->bw[gi], bpr = (w + 1u) / 2u, gx, gy;
    const uint8_t *gd = f->data + f->off[gi];
    for (gy = row0; gy < f->h; gy++)
        for (gx = 0; gx < w; gx++) {
            uint32_t a = gd[gy * bpr + gx / 2u];
            a = (gx & 1u) ? (a & 15u) : (a >> 4);
            if (a >= 6u)
                cv_pset(x - f->pad + (int32_t)gx, y + (int32_t)gy, c);
        }
}

/* a mark above the ink at row top, centred on cx; k the size (1, or 2 in the big font) */
static void t1_mark(int32_t cx, int32_t top, int m, int32_t k, uint16_t c)
{
    static const int8_t DOT[7][10] = {
        {0},
        {1, 0, 0, 1, -1, 2, 99},                      /* grave */
        {-1, 0, 0, 1, 1, 2, 99},                      /* acute */
        {-2, 0, -1, 1, 0, 2, 1, 1, 2, 0},             /* circumflex */
        {-2, 0, -1, 1, 0, 0, 1, 0, 2, 1},             /* tilde */
        {-2, 0, -1, 0, 1, 0, 2, 0, 99},               /* umlaut (and a row above: below) */
        {-1, 0, 0, 0, 1, 0, 99},                      /* ring (its sides: below) */
    };
    int i;
    int32_t b = top - 2 * k;
    for (i = 0; i < 10 && DOT[m][i] != 99; i += 2) {
        cv_rect(cx + DOT[m][i] * k, b - DOT[m][i + 1] * k, k, k, c);
        if (m == MK_UML)
            cv_rect(cx + DOT[m][i] * k, b - k, k, k, c);
    }
    if (m == MK_RING) {
        cv_rect(cx - k, b - 2 * k, 3 * k, k, c);
        cv_rect(cx - k, b - k, k, k, c);
        cv_rect(cx + k, b - k, k, k, c);
    }
}

/* draws (draw 1) or measures (draw 0) text; returns the end x, or the width */
static int32_t tx_do(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c, int ui_draw)
{
    int32_t x0 = x, k = f->h >= 22 ? 2 : 1;
    char one[2] = {0, 0};
    while (*s) {
        uint32_t cp = u8next(&s);
        if (cp == 1u || cp == 2u || cp == 0xB0u) {   /* a small circle, raised; crossed for '\x02' */
            int32_t r = f->h >= 15 ? 3 : 2, cx = x + r + 1, cy = y + r + 1, dx, dy;
            for (dy = -r; ui_draw && dy <= r; dy++)
                for (dx = -r; dx <= r; dx++) {
                    int32_t d2 = dx * dx + dy * dy;
                    if (d2 <= r * r + r && d2 >= (r - 1) * (r - 1))
                        cv_pset(cx + dx, cy + dy, c);
                }
            if (ui_draw && cp == 2u)
                cv_line(cx - r - 1, cy + r + 1, cx + r + 1, cy - r - 1, c);
            x += 2 * r + 3;
            continue;
        }
        if (cp == 0xDFu) {                            /* the sharp s: ss */
            x = ui_draw ? cv_text(x, y, f, "ss", c) : x + text_w(f, "ss");
            continue;
        }
        if (cp >= 0xC0u && cp <= 0xFFu) {             /* an accented letter */
            uint32_t i = cp - 0xC0u, gi;
            int m = LAT_MARK[i];
            one[0] = LAT_BASE[i];
            gi = glyph(f, (uint8_t)one[0]);
            if (ui_draw) {
                int32_t top;
                if ((one[0] == 'i') && m && f->last >= 'a') {
                    top = ink_top(f, 'x');
                    glyph_rows(x, y, f, 'i', c, (uint32_t)top);
                } else {
                    cv_text(x, y, f, one, c);
                    top = ink_top(f, (uint8_t)one[0]);
                }
                if (m == MK_CEDILLA) {       /* a hook under the letter */
                    int32_t cx = x - f->pad + (int32_t)f->bw[gi] / 2, by = y + ink_bottom(f, (uint8_t)one[0]);
                    cv_rect(cx, by + 1, k, k, c);
                    cv_rect(cx + k, by + 1 + k, k, k, c);
                    cv_rect(cx - k, by + 1 + 2 * k, 2 * k, k, c);
                } else if (m) {
                    int32_t kk = k;
                    if (top - 3 * kk < -1 && kk > 1)   /* no room above a capital in the big font: a small mark */
                        kk = 1;
                    if (top - 3 * kk >= -1)
                        t1_mark(x - f->pad + (int32_t)f->bw[gi] / 2, y + top, m, kk, c);
                }
            }
            x += text_w(f, one);
            continue;
        }
        if (cp >= 0x80u)                              /* other marks (inverted ? and !): left out */
            continue;
        one[0] = (char)cp;
        x = ui_draw ? cv_text(x, y, f, one, c) : x + text_w(f, one);
    }
    return ui_draw ? x : x - x0;
}
static int cut_count;
static void text_cut(const char *s);
static int32_t t1_tx(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{
    int32_t e = tx_do(x, y, f, s, c, 1);
    if (e > 240 || x < 0)
        text_cut(s);
    return e;
}
static int32_t t1_tw(const felucca_font_t *f, const char *s) { return tx_do(0, 0, f, s, 0, 0); }

static void text_c(int32_t y, const felucca_font_t *f, const char *s, uint16_t c) { t1_tx((240 - t1_tw(f, s)) / 2, y, f, s, c); }
static void text_r(int32_t xr, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{
    t1_tx(xr - t1_tw(f, s), y, f, s, c);
}

/* words wrapped into width w (draw 0: only measured); returns the y after the last line, ymax + 999 if it did not
 * all fit */
static int32_t wrap_do(int32_t x, int32_t y, int32_t w, const felucca_font_t *f, const char *s, uint16_t c,
                       int32_t lh, int32_t ymax, int ui_draw)
{
    char line[120], word[48], t[170];
    int ln = 0;
    line[0] = 0;
    while (*s) {
        int wn = 0;
        while (*s == ' ')
            s++;
        while (*s && *s != ' ' && wn < 47)
            word[wn++] = *s++;
        word[wn] = 0;
        if (!wn)
            break;
        t1_strcpy(t, line);
        if (ln)
            str_cat(t, " ");
        str_cat(t, word);
        if (ln && (t1_tw(f, t) > w || ln + wn + 1 > 118)) {
            if (y + lh > ymax + 2)
                return ymax + 999;
            if (ui_draw)
                t1_tx(x, y, f, line, c);
            y += lh;
            t1_strcpy(line, word);
            ln = wn;
        } else {
            t1_strcpy(line, t);
            ln = ln + (ln ? 1 : 0) + wn;
        }
    }
    if (ln) {
        if (y + lh > ymax + 2)
            return ymax + 999;
        if (ui_draw)
            t1_tx(x, y, f, line, c);
        y += lh;
    }
    return y;
}
/* text that did not fit: the host's tests list it (app_host_fails counts it as a failure) */
static void text_cut(const char *s)
{
#ifdef PLAT_HOST
    fprintf(stderr, "TEXT CUT (lang %d): %s\n", th_ui, s);
#endif
    (void)s;
    cut_count++;
}
static int32_t text_wrap(int32_t x, int32_t y, int32_t w, const felucca_font_t *f, const char *s, uint16_t c,
                         int32_t lh, int32_t ymax)
{
    int32_t r = wrap_do(x, y, w, f, s, c, lh, ymax, 1);
    if (r > ymax + 2)
        text_cut(s);
    return r;
}
/* a paragraph in the medium-small font if it fits, else the small one */
static void para(int32_t x, int32_t y, int32_t w, const char *s, uint16_t c, int32_t ymax)
{
    if (wrap_do(x, y, w, &FONT_S, s, c, 16, ymax, 0) <= ymax + 2)
        wrap_do(x, y, w, &FONT_S, s, c, 16, ymax, 1);
    else if (wrap_do(x, y, w, &FONT_XS, s, c, 13, ymax, 1) > ymax + 2)
        text_cut(s);
}

/* the biggest of M, B and XS that fits */
static const felucca_font_t *fit_font(const char *s, int32_t w)
{
    return t1_tw(&FONT_M, s) <= w ? &FONT_M : t1_tw(&FONT_B, s) <= w ? &FONT_B : &FONT_XS;
}

/* a box with a note in it, and its letter name under it when that is shown */
static void note_box(int32_t x, int32_t y, int32_t w, int32_t h, const char *s, const char *sub, uint16_t bg,
                     uint16_t fg, uint16_t border)
{
    const felucca_font_t *f = t1_tw(&FONT_B, s) <= w - 2 ? &FONT_B : &FONT_XS;
    cv_rect(x, y, w, h, border);
    cv_rect(x + 1, y + 1, w - 2, h - 2, bg);
    if (sub && sub[0] && h >= 30) {
        t1_tx(x + (w - t1_tw(f, s)) / 2, y + 2, f, s, fg);
        t1_tx(x + (w - t1_tw(&FONT_XS, sub)) / 2, y + h - 15, &FONT_XS, sub, COL_ALT);
    } else {
        t1_tx(x + (w - t1_tw(f, s)) / 2, y + (h - (int32_t)f->h) / 2, f, s, fg);
    }
}
static int32_t box_h(void) { return th_names == NAMES_BOTH ? 34 : 24; }

/* the header: a title, a value on the right */
static void ui_header(const char *title, const char *right, uint16_t rc)
{
    char t[64];
    if (plat_ms() < ui.msg_until) {         /* a message for a moment */
        int32_t x = t1_tx(8, 4, &FONT_B, ui.msg[0], C_WHITE);
        t1_tx(x + 8, 4, &FONT_B, ui.msg[1], C_HI);
    } else {
        int32_t x;
        t1_upper(t, title);
        x = t1_tx(8, 4, &FONT_B, t, C_WHITE);
        if (FM1B_BLE_CENTRAL && plat_ble_device() == BLE_DEV_READY)
            x = t1_tx(x + 6, 7, &FONT_T, "BT", COL_SCALE);
        if (right)
            text_r(232, t1_tw(&FONT_B, right) <= 224 - x ? 4 : 7, t1_tw(&FONT_B, right) <= 224 - x ? &FONT_B : &FONT_XS,
                   right, rc);
    }
    cv_rect(0, 24, 240, 1, C_LINE);
}

static void hint(const char *s) { text_c(188, &FONT_T, s, C_GRAY); }

/* the footer: the FM-1's 27 keys, F3 .. G5, marked by pitch class */
static struct {
    uint16_t scale, root, ok, bad, hint;
} mk;
static void ui_keyboard(void)
{
    static const uint8_t WK[16] = PLAT_WHITE_KEYS, BK[11] = PLAT_BLACK_KEYS;
    static const uint8_t AT[11] = {0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14};
    int i;
    const int32_t y = 200, kw = 15;
    for (i = 0; i < 16; i++) {
        int k = WK[i], pc = (53 + k) % 12, note = 53 + k + 12 * ui.octave;
        uint16_t c = COL_WKEY;
        if (mk.scale >> pc & 1u)
            c = COL_SCALE;
        if (mk.hint >> pc & 1u)
            c = RGB(120, 150, 220);
        if (mk.root >> pc & 1u)
            c = COL_ROOT;
        if (mk.ok >> pc & 1u)
            c = COL_OK;
        if (mk.bad >> pc & 1u)
            c = COL_BAD;
        if (ui.held[note])
            c = COL_HELD;
        cv_rect(i * kw, y, kw - 1, 40, c);
        if (pc == 0)                        /* the Cs, marked */
            cv_rect(i * kw + 5, y + 33, 4, 3, C_BLACK);
    }
    for (i = 0; i < 11; i++) {
        int k = BK[i], pc = (53 + k) % 12, note = 53 + k + 12 * ui.octave;
        uint16_t c = COL_BKEY;
        if (mk.scale >> pc & 1u)
            c = RGB(40, 110, 190);
        if (mk.hint >> pc & 1u)
            c = RGB(80, 100, 170);
        if (mk.root >> pc & 1u)
            c = RGB(220, 110, 20);
        if (mk.ok >> pc & 1u)
            c = RGB(40, 170, 80);
        if (mk.bad >> pc & 1u)
            c = RGB(200, 50, 40);
        if (ui.held[note])
            c = RGB(230, 190, 40);
        cv_rect(AT[i] * kw + 9, y, 11, 24, C_BLACK);
        cv_rect(AT[i] * kw + 10, y, 9, 23, c);
    }
}

/* the LEDs follow the footer: bright the root, the right and held notes; dim the scale and the hint */
static uint32_t btn_live;                    /* the buttons that do something here: they glow */
static void ui_lights(void)
{
    uint32_t bright = 0, dim = 0, mid = 0, k;
    for (k = 0; k < NKEYS; k++) {
        int pc = (int)((53u + k) % 12u), note = 53 + (int)k + 12 * ui.octave;
        uint32_t bit = 1u << k;
        if (ui.held[note] || ((mk.root | mk.ok) >> pc & 1u))
            bright |= bit;
        else if (mk.bad >> pc & 1u)
            mid |= bit;
        else if ((mk.scale | mk.hint) >> pc & 1u)
            dim |= bit;
    }
    plat_lights(ui.btn | (set_dirty ? 1u << B_SAVE : 0u), bright, btn_live, dim, 1);
    plat_lights_mid(0, mid);
}

/* =========================================================== the screens === */
static void draw_menu(void)
{
    static const uint16_t COL[4] = {RGB(70, 170, 255), RGB(255, 150, 40), RGB(80, 220, 120), RGB(170, 170, 180)};
    const int names[4] = {S_M_LESSONS, S_M_QUIZ, S_M_LAB, S_M_SETTINGS};
    const int subs[4] = {S_M_LESSONS_SUB, S_M_QUIZ_SUB, S_M_LAB_SUB, S_M_SETTINGS_SUB};
    int i;
    ui_header("THEORY-1", S(S_TAGLINE), C_GRAY);
    for (i = 0; i < 4; i++) {
        int32_t y = 30 + i * 39, sel = i == ui.menu;
        cv_rect(4, y, 232, 36, sel ? COL_SELBG : COL_PANEL);
        cv_rect(4, y, 4, 36, COL[i]);
        t1_tx(16, y + 1, &FONT_M, S(names[i]), sel ? C_WHITE : C_GRAY);
        t1_tx(16, y + 22, &FONT_XS, S(subs[i]), sel ? C_HI : C_DIM);
    }
    hint(S(S_H_MENU));
    mk.scale = WHITE_PCS;
    mk.root = PCB(0);
}

static void draw_topics(void)
{
    int i;
    ui_header(S(S_M_LESSONS), 0, 0);
    for (i = 0; i < NTOPICS; i++) {
        int32_t y = 29 + i * 22, sel = i == ui.topic_sel;
        if (sel)
            cv_rect(4, y, 232, 21, COL_SELBG);
        t1_tx(10, y + 2, &FONT_B, TOPIC_NAME[th_ui][i], sel ? C_WHITE : C_HI);
        text_r(230, y + 4, &FONT_XS, TOPIC_SUB[th_ui][i], sel ? C_HI : C_GRAY);
    }
    hint(S(S_H_TOPICS));
}

/* a row of note boxes across the width: names (and letters), and a label under each */
static void note_row(int32_t y, const nt_t *n, int count, const char *const *labels, int hl, uint16_t hl_col,
                     int root_box)
{
    int i, per = count > 8 ? (count + 1) / 2 : count;
    int32_t bh = box_h(), row_h = labels ? bh + 14 : bh + 4;
    int32_t w = (232 - (per - 1) * 3) / per;
    for (i = 0; i < count; i++) {
        char t[8], a[8];
        int r = i / per, c = i % per;
        int32_t x = 4 + c * (w + 3), yy = y + r * row_h;
        uint16_t bg = i == hl ? hl_col : (i == 0 && root_box) ? RGB(90, 50, 10) : COL_PANEL;
        note_name(n[i], t);
        note_alt(n[i], a);
        note_box(x, yy, w, bh, t, a, bg, C_WHITE, i == 0 && root_box ? COL_ROOT : C_LINE);
        if (labels && labels[i])
            t1_tx(x + (w - t1_tw(&FONT_XS, labels[i])) / 2, yy + bh + 1, &FONT_XS, labels[i], C_GRAY);
    }
}

/* splits "1 2 b3 4" into labels */
static int split_labels(char *s, const char **out, int max)
{
    int n = 0;
    while (*s && n < max) {
        while (*s == ' ')
            *s++ = 0;
        if (!*s)
            break;
        out[n++] = s;
        while (*s && *s != ' ')
            s++;
    }
    return n;
}

/* a scale or a mode: its title, its notes, its degrees, its steps, an extra line, its text */
static void draw_scale_like(const scale_t *s, nt_t root, const char *title, int char_deg, const char *extra)
{
    nt_t n[12];
    char deg[48], steps[48], t[96];
    const char *lab[12];
    int i;
    int32_t y;
    uint16_t m = 0;
    for (i = 0; i < s->n; i++) {
        n[i] = scale_note(s, root, i);
        m |= (uint16_t)PCB(pc_of(n[i]));
    }
    t1_tx(8, 29, fit_font(title, 224), title, C_WHITE);
    if (s->n == 12) {
        note_row(56, n, 12, 0, -1, 0, 1);
        y = 56 + 2 * (box_h() + 4);
        t1_tx(8, y, &FONT_XS, S(S_CHROM_STEPS), C_HI);
        y += 14;
    } else {
        degree_labels(s, deg);
        split_labels(deg, lab, 12);
        note_row(56, n, s->n, lab, char_deg, RGB(150, 70, 10), char_deg < 0);
        y = 56 + box_h() + 15;
        scale_steps(s, steps);
        t1_strcpy(t, S(S_STEPS));
        str_cat(t, steps);
        t1_tx(8, y, &FONT_XS, t, C_HI);
        y += 13;
    }
    if (extra) {
        y = text_wrap(8, y, 226, &FONT_XS, extra, COL_ROOT, 13, y + 26);
    }
    para(8, y + 2, 226, TR(s->text), C_GRAY, 184);
    mk.scale = m;
    mk.root = (uint16_t)PCB(pc_of(root));
}

static void draw_lesson(void)
{
    char t[96], r[48], u[96], v[32];
    int it = ui.item[ui.topic];
    nt_t root = ROOTS[ui.root[ui.topic]];
    t1_strcpy(r, TOPIC_NAME[th_ui][ui.topic]);
    str_cat(r, " ");
    cat_int(r, it + 1);
    str_cat(r, "/");
    cat_int(r, ui.topic == T_HARMONY ? NHSCALES : TOPIC_ITEMS[ui.topic]);
    switch (ui.topic) {
    case T_BASICS: {
        const page_t *p = &BASICS[it];
        ui_header(TOPIC_NAME[th_ui][T_BASICS], r, C_GRAY);
        t1_tx(8, 29, fit_font(TR(p->title), 224), TR(p->title), C_WHITE);
        para(8, 60, 226, TR(p->text), C_HI, 184);
        mk.scale = p->mask;
        mk.root = p->root;
        hint(S(S_H_BASICS));
        break;
    }
    case T_INTERVALS: {
        const interval_t *iv = &INTERVALS[it];
        nt_t top = interval_top(root, it);
        int32_t i, x;
        ui_header(TOPIC_NAME[th_ui][T_INTERVALS], r, C_GRAY);
        t1_tx(8, 29, fit_font(TR(iv->name), 150), TR(iv->name), C_WHITE);
        int_str(t, iv->semis);
        str_cat(t, " ");
        str_cat(t, S(iv->semis == 1 ? S_SEMITONE : S_SEMITONES));
        text_r(232, 35, &FONT_B, t, C_HI);
        for (i = 0; i <= 12; i++) {          /* the distance, drawn: 13 cells, the two notes lit */
            x = 8 + i * 17;
            cv_rect(x, 60, 15, 14, i == 0 ? COL_ROOT : i == iv->semis ? COL_OK : i < iv->semis ? COL_SELBG : COL_PANEL);
        }
        note_full(root, t);
        str_cat(t, " -> ");
        note_full(top, u);
        str_cat(t, u);
        if (it == 12) {
            str_cat(t, " ");
            str_cat(t, S(S_ABOVE));
        }
        t1_tx(8, 80, fit_font(t, 226) == &FONT_M ? &FONT_B : fit_font(t, 226), t, C_WHITE);
        t1_strcpy(t, S(S_SYMBOL));
        str_cat(t, ": ");
        str_cat(t, iv->abbr);
        t1_tx(8, 98, &FONT_XS, t, C_GRAY);
        para(8, 114, 226, TR(iv->text), C_GRAY, 184);
        mk.root = (uint16_t)PCB(pc_of(root));
        mk.ok = (uint16_t)PCB(pc_of(top));
        hint(S(S_H_INTERVALS));
        break;
    }
    case T_SCALES: {
        note_full(root, t);
        str_cat(t, " ");
        str_cat(t, TR(SCALES[it].name));
        ui_header(TOPIC_NAME[th_ui][T_SCALES], r, C_GRAY);
        draw_scale_like(&SCALES[it], root, t, -1, 0);
        hint(S(S_H_SCALES));
        break;
    }
    case T_MODES: {
        scale_t s;
        nt_t par = mode_parent(root, it), ch;
        char a[16], b[16];
        mode_scale(it, &s);
        ch = scale_note(&s, root, MODES[it].ch);
        note_full(root, t);
        str_cat(t, " ");
        str_cat(t, TR(MODES[it].name));
        ui_header(TOPIC_NAME[th_ui][T_MODES], r, C_GRAY);
        note_name(par, a);
        note_name(root, b);
        note_name(ch, v);
        t1_fmt(u, S(S_MODE_PARENT), a, b, v);
        draw_scale_like(&s, root, t, MODES[it].ch, u);
        hint(S(S_H_MODES));
        break;
    }
    case T_CHORDS: {
        const chord_t *c = &CHORDS[it];
        nt_t n[4];
        char f[16];
        const char *lab[4];
        int i;
        uint16_t m = 0;
        ui_header(TOPIC_NAME[th_ui][T_CHORDS], r, C_GRAY);
        chord_name(it, root, t);
        t1_tx(8, 29, fit_font(t, 226), t, C_WHITE);
        for (i = 0; i < c->n; i++) {
            n[i] = chord_note(it, root, i);
            m |= (uint16_t)PCB(pc_of(n[i]));
        }
        t1_strcpy(f, c->formula);
        split_labels(f, lab, 4);
        note_row(56, n, c->n, lab, -1, 0, 1);
        v[0] = 0;
        for (i = 1; i < c->n; i++) {
            if (i > 1)
                str_cat(v, " + ");
            cat_int(v, c->semi[i] - c->semi[i - 1]);
        }
        t1_fmt(t, S(S_STRUCTURE), v, 0, 0);
        t1_tx(8, 56 + box_h() + 16, &FONT_XS, t, C_HI);
        chord_symbol(it, root, u);
        text_r(232, 56 + box_h() + 13, &FONT_B, u, COL_ROOT);
        para(8, 56 + box_h() + 32, 226, TR(c->text), C_GRAY, 184);
        mk.scale = m;
        mk.root = (uint16_t)PCB(pc_of(root));
        hint(S(S_H_CHORDS));
        break;
    }
    case T_HARMONY: {
        scale_t s;
        int d, n = ui.hseventh ? 4 : 3;
        uint16_t m = 0;
        hscale(it, &s);
        note_name(root, t);
        str_cat(t, " ");
        str_cat(t, TR(s.name));
        ui_header(TOPIC_NAME[th_ui][T_HARMONY], t, COL_ROOT);
        for (d = 0; d < 7; d++) {
            int ty = degree_chord(&s, d, n), i, sel = d == ui.hdeg;
            int32_t y = 28 + d * 18;
            nt_t cr = scale_note(&s, root, d), cn[4];
            if (sel)
                cv_rect(2, y, 236, 18, COL_SELBG);
            roman(d, ty, u);
            t1_tx(8, y + 1, &FONT_B, u, sel ? C_WHITE : C_HI);
            if (ty >= 0) {
                chord_symbol(ty, cr, u);
                t1_tx(78, y + 1, &FONT_B, u, sel ? COL_ROOT : C_WHITE);
                for (i = 0; i < n; i++)
                    cn[i] = chord_note(ty, cr, i);
                notes_text(cn, n, u);            /* (long spellings, Do## and the like: the small capitals) */
                if (t1_tw(&FONT_XS, u) <= 236 - 150)
                    t1_tx(150, y + 3, &FONT_XS, u, sel ? C_WHITE : C_GRAY);
                else if (t1_tw(&FONT_T, u) <= 236 - 150)
                    t1_tx(150, y + 5, &FONT_T, u, sel ? C_WHITE : C_GRAY);
                /* (else, in the far keys, the symbol says it: the notes are on the keys and in the line below) */
                if (sel)
                    for (i = 0; i < n; i++)
                        m |= (uint16_t)PCB(pc_of(cn[i]));
            }
        }
        {                                    /* the chosen degree: its name and its function */
            nt_t cr = scale_note(&s, root, ui.hdeg);
            int ty = degree_chord(&s, ui.hdeg, n);
            t1_strcpy(t, degree_name(&s, ui.hdeg));
            str_cat(t, ": ");
            if (ty >= 0) {
                chord_name(ty, cr, u);
                str_cat(t, u);
                if (t1_tw(&FONT_XS, t) > 226) {     /* a long name: the symbol instead */
                    t1_strcpy(t, degree_name(&s, ui.hdeg));
                    str_cat(t, ": ");
                    chord_symbol(ty, cr, u);
                    str_cat(t, u);
                }
            }
            t1_tx(8, 156, &FONT_XS, t, C_WHITE);
            if (it < 4)
                t1_fmt(t, S(S_FUNCTION), FN_NAME[th_ui][degree_fn(ui.hdeg)], 0, 0);
            else
                t1_strcpy(t, S(S_MODE_CHORDS));
            t1_tx(8, 170, &FONT_XS, t, C_HI);
            mk.root = (uint16_t)PCB(pc_of(cr));
        }
        mk.scale = m;
        hint(S(ui.hseventh ? S_H_HARMONY4 : S_H_HARMONY3));
        break;
    }
    case T_KEYS: {
        const keysig_t *k = &KEYSIGS[it];
        static const nt_t POS_ROOT[12] = {{0, 0}, {4, 0}, {1, 0}, {5, 0}, {2, 0}, {6, 0}, {3, 1}, {1, -1}, {5, -1},
                                          {2, -1}, {6, -1}, {3, 0}};
        static const int8_t CS[12] = {0, 50, 87, 100, 87, 50, 0, -50, -87, -100, -87, -50};   /* sin x 100 */
        int pos = k->count >= 0 ? k->count % 12 : (12 + k->count) % 12, i, n = k->count < 0 ? -k->count : k->count;
        int32_t cx = 60, cy = 96, rr = 46;
        nt_t rel = relative_minor(k->root);
        ui_header(TOPIC_NAME[th_ui][T_KEYS], r, C_GRAY);
        for (i = 0; i < 12; i++)             /* the circle: a 12-gon is near enough at this size */
            cv_line(cx + CS[i] * rr / 100, cy - CS[(i + 3) % 12] * rr / 100, cx + CS[(i + 1) % 12] * rr / 100,
                    cy - CS[(i + 4) % 12] * rr / 100, C_LINE);
        for (i = 0; i < 12; i++) {
            int32_t x = cx + CS[i] * (rr - 2) / 100, y = cy - CS[(i + 3) % 12] * (rr - 2) / 100;
            char lab[8];
            note_name(i == pos ? k->root : POS_ROOT[i], lab);
            if (i == pos) {
                int32_t dx, dy;
                for (dy = -11; dy <= 11; dy++)
                    for (dx = -15; dx <= 15; dx++)
                        if (dx * dx * 121 + dy * dy * 225 <= 121 * 225)
                            cv_pset(x + dx, y + dy, COL_ROOT);
            }
            t1_tx(x - t1_tw(&FONT_T, lab) / 2, y - 5, &FONT_T, lab, i == pos ? C_BLACK : C_WHITE);
        }
        t1_tx(cx - t1_tw(&FONT_T, S(S_FIFTHS)) / 2, cy - 5, &FONT_T, S(S_FIFTHS), C_DIM);
        note_full(k->root, u);
        t1_fmt(t, S(S_KEY_MAJ), u, 0, 0);
        t1_tx(124, 29, t1_tw(&FONT_B, t) <= 112 ? &FONT_B : &FONT_XS, t, C_WHITE);
        if (n == 0)
            t1_strcpy(t, S(S_NO_ALT));
        else {
            int_str(t, n);
            str_cat(t, " ");
            str_cat(t, S(k->count > 0 ? (n == 1 ? S_SHARP_1 : S_SHARP_N) : (n == 1 ? S_FLAT_1 : S_FLAT_N)));
        }
        t1_tx(124, 48, t1_tw(&FONT_B, t) <= 112 ? &FONT_B : &FONT_XS, t,
           k->count > 0 ? COL_ROOT : k->count < 0 ? COL_SCALE : C_HI);
        key_alterations(k->count, u);
        text_wrap(124, 66, 112, &FONT_XS, u, C_HI, 13, 108);
        note_full(rel, u);
        t1_fmt(t, S(S_RELATIVE), u, 0, 0);
        text_wrap(124, text_wrap(124, 108, 112, &FONT_XS, t, C_WHITE, 13, 134) + 3, 112, &FONT_XS,
                  S(k->count >= 0 ? S_CW : S_CCW), C_GRAY, 13, 184);
        {                                    /* the order of the sharps or the flats */
            char nn[8];
            t1_strcpy(t, S(k->count >= 0 ? S_SHARPS : S_FLATS));
            for (i = 0; i < 7; i++) {
                str_cat(t, " ");
                note_name(k->count >= 0 ? SHARP_ORDER[i] : FLAT_ORDER[i], nn);
                str_cat(t, nn);
            }
            text_wrap(8, 152, 112, &FONT_XS, t, C_DIM, 13, 184);
        }
        {
            int j;
            uint16_t m = 0;
            for (j = 0; j < 7; j++)
                m |= (uint16_t)PCB(pc_of(scale_note(&SCALES[SC_MAJOR], k->root, j)));
            mk.scale = m;
            mk.root = (uint16_t)PCB(pc_of(k->root));
        }
        hint(S(S_H_KEYS));
        break;
    }
    default:
        break;
    }
}

/* -------------------------------------------------------------- the quiz --- */
static void draw_qsetup(void)
{
    int i;
    char t[16];
    ui_header(S(S_M_QUIZ), S(S_RECORD), C_GRAY);
    for (i = 0; i < NQT; i++) {
        int32_t y = 28 + i * 14, sel = i == ui.qsel;
        if (sel)
            cv_rect(4, y, 232, 14, COL_SELBG);
        t1_tx(10, y + 1, &FONT_XS, QT_NAMES[th_ui][i], sel ? C_WHITE : C_HI);
        if (cfg.best[i]) {
            int_str(t, cfg.best[i]);
            str_cat(t, "/100");
        } else {
            t1_strcpy(t, "-");
        }
        text_r(230, y + 1, &FONT_XS, t, sel ? C_WHITE : C_GRAY);
    }
    cv_rect(4, 170, 232, 1, C_LINE);
    t1_tx(t1_tx(10, 172, &FONT_XS, S(S_LEVEL_K1), C_GRAY) + 8, 172, &FONT_XS, LEVEL_NAMES[th_ui][cfg.level], COL_ROOT);
    hint(S(S_H_QSETUP));
}

static void q_feedback(const char *s, uint16_t c)
{
    t1_strcpy(q.fb, s);
    q.fb_col = c;
}

static uint8_t q_pick(const uint8_t *list, int n) { return list[rnd((uint32_t)n)]; }
static const uint8_t NATURAL_ROOTS[7] = {0, 3, 6, 7, 10, 13, 16};

static int qtype_now;

/* the reference note an interval question gives: played, and lit */
static void q_play_ref(void)
{
    if (q.ref_pc >= 0)
        play_note(plat_ms() + 20u, base_midi(q.ref), 700u, 95);
}

static void q_play_listen(void)
{
    if (q.listen_kind == 0)
        play_interval(q.listen[0], q.listen[1] - q.listen[0], 1);
    else {
        uint32_t t = plat_ms() + 30u;
        int i;
        for (i = 0; i < q.nlisten; i++)
            play_note(t, q.listen[i], 1400u, 85);
        for (i = 0; i < q.nlisten; i++)
            play_note(t + 1600u + (uint32_t)i * step_ms(), q.listen[i], step_ms(), 80);
    }
}

/* the choices: the right answer at a random place, up to max - 1 others from the pool, all different */
static void q_choices(const char *right, const char *const *pool, int npool, int max)
{
    char tmp[4][40];
    int n = 1, tries = 0, i, j = 1;
    t1_strcpy(tmp[0], right);
    if (max > 4)
        max = 4;
    while (n < max && tries++ < 300) {
        const char *c = pool[rnd((uint32_t)npool)];
        int dup = 0;
        for (i = 0; i < n; i++)
            if (t1_streq(tmp[i], c))
                dup = 1;
        if (!dup)
            t1_strcpy(tmp[n++], c);
    }
    q.kind = K_CHOICE;
    q.nopt = (uint8_t)n;
    q.correct = (uint8_t)rnd((uint32_t)n);
    q.sel = 0;
    q.gone = 0;
    for (i = 0; i < n; i++)
        t1_strcpy(q.opt[i], i == q.correct ? tmp[0] : tmp[j++]);
}

/* "3 diesis", "1 flat", "keine" */
static void count_text(int c, char *b)
{
    int n = c < 0 ? -c : c;
    if (c == 0) {
        t1_strcpy(b, S(S_Q_NONE));
        return;
    }
    int_str(b, n);
    str_cat(b, " ");
    str_cat(b, S(c > 0 ? (n == 1 ? S_SHARP_1 : S_SHARP_N) : (n == 1 ? S_FLAT_1 : S_FLAT_N)));
}

/* notes a student should not meet in an answer: a double sharp or flat, and before level 3 E#, B#, Fb, Cb
 * (right as theory, but a question in an easier key teaches the same thing) */
static int awkward(const nt_t *n, int count, int lv)
{
    int i;
    for (i = 0; i < count; i++) {
        if (n[i].acc > 1 || n[i].acc < -1)
            return 1;
        if (lv < 2 && ((n[i].acc == 1 && (n[i].letter == 2 || n[i].letter == 6)) ||
                       (n[i].acc == -1 && (n[i].letter == 3 || n[i].letter == 0))))
            return 1;
    }
    return 0;
}

/* a question of type t at the chosen level, once (q_make draws again until the answer reads well) */
static void q_make1(int t)
{
    int lv = cfg.level, i;
    char a[40], b[40];
    q.type = (uint8_t)t;
    q.kind = K_SEQ;
    q.n = 0;
    q.pos = 0;
    q.found = 0;
    q.ref_pc = -1;
    q.wrong_pc = -1;
    q.errors = q.hints = q.hint_on = 0;
    q.nopt = 0;
    q.nlisten = 0;
    q.done = 0;
    q.l1[0] = q.l2[0] = q.ans[0] = q.fb[0] = 0;
    q.ref.letter = 0;                         /* C: the reference note when a question has no other */
    q.ref.acc = 0;
    switch (t) {
    case QT_NOTE: {
        nt_t n = ROOTS[lv == 0 ? q_pick(NATURAL_ROOTS, 7) : rnd(NROOTS)];
        q.kind = K_ONE;
        q.n = 1;
        q.nt[0] = n;
        q.pc[0] = (uint8_t)pc_of(n);
        t1_strcpy(q.l1, S(S_Q_NOTE));
        note_full(n, q.l2);
        if (lv > 0 && n.acc) {               /* the other name too: the enharmonic */
            nt_t e = spell(n, 0, n.acc > 0 ? 1 : 6);
            note_full(e, a);
            t1_fmt(q.ans, S(S_Q_SAMEKEY), a, 0, 0);
        }
        break;
    }
    case QT_INTERVAL: {
        static const uint8_t L1[4] = {3, 4, 5, 7}, L2[8] = {1, 2, 3, 4, 5, 7, 8, 9},
                             L3[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        int iv = lv == 0 ? q_pick(L1, 4) : lv == 1 ? q_pick(L2, 8) : q_pick(L3, 11);
        nt_t r = ROOTS[lv == 0 ? q_pick(NATURAL_ROOTS, 7) : rnd(NROOTS)], top = interval_top(r, iv);
        q.kind = K_ONE;
        q.n = 1;
        q.nt[0] = top;
        q.pc[0] = (uint8_t)pc_of(top);
        q.ref = r;
        q.ref_pc = (int8_t)pc_of(r);
        t1_strcpy(q.l1, S(S_Q_INT1));
        note_full(r, a);
        t1_fmt(q.l2, S(S_Q_INT2), TR(INTERVALS[iv].name), a, 0);
        note_full(top, q.ans);
        break;
    }
    case QT_SCALE: case QT_MODE: {
        scale_t s;
        nt_t r;
        if (t == QT_SCALE) {
            static const uint8_t EASY[4] = {0, 10, 7, 3};   /* C G F D */
            int sc = lv == 0 ? SC_MAJOR : lv == 1 ? (int)rnd(2) : (int)rnd(6);
            r = ROOTS[lv == 0 ? q_pick(EASY, 4) : sc == SC_MAJOR || sc == SC_PENTA_MAJ ?
                q_pick(COMMON_MAJOR_ROOTS, sizeof COMMON_MAJOR_ROOTS) : q_pick(COMMON_MINOR_ROOTS, sizeof COMMON_MINOR_ROOTS)];
            s = SCALES[sc];
            t1_strcpy(q.l1, S(S_Q_SCALE));
        } else {
            static const uint8_t WHITE_MODE_ROOT[7] = {0, 3, 6, 7, 10, 13, 16};   /* C D E F G A B: white keys */
            int m = (int)rnd(7);
            r = ROOTS[lv == 0 ? WHITE_MODE_ROOT[m] : lv == 1 ? q_pick(NATURAL_ROOTS, 7) :
                q_pick(COMMON_MAJOR_ROOTS, sizeof COMMON_MAJOR_ROOTS)];
            mode_scale(m, &s);
            t1_strcpy(q.l1, S(S_Q_MODE));
        }
        note_full(r, q.l2);
        str_cat(q.l2, " ");
        str_cat(q.l2, TR(s.name));
        q.n = s.n;
        for (i = 0; i < s.n; i++) {
            q.nt[i] = scale_note(&s, r, i);
            q.pc[i] = (uint8_t)pc_of(q.nt[i]);
        }
        q.ref = r;
        notes_text(q.nt, q.n, q.ans);
        break;
    }
    case QT_CHORD: {
        int ty = lv == 0 ? (int)rnd(2) : lv == 1 ? (int)rnd(NTRIADS) : (int)rnd(NCHORDS);
        nt_t r = ROOTS[lv == 0 ? q_pick(NATURAL_ROOTS, 7) : q_pick(COMMON_MAJOR_ROOTS, sizeof COMMON_MAJOR_ROOTS)];
        q.kind = K_SET;
        q.n = CHORDS[ty].n;
        for (i = 0; i < q.n; i++) {
            q.nt[i] = chord_note(ty, r, i);
            q.pc[i] = (uint8_t)pc_of(q.nt[i]);
        }
        t1_strcpy(q.l1, S(S_Q_CHORD));
        chord_name(ty, r, q.l2);
        chord_symbol(ty, r, a);
        t1_strcpy(q.ans, S(S_SYMBOL));
        str_cat(q.ans, ": ");
        str_cat(q.ans, a);
        q.ref = r;
        break;
    }
    case QT_DEGREE: {
        static const uint8_t EASY[3] = {0, 10, 7};   /* C G F */
        static const uint8_t PRIMARY[3] = {0, 3, 4}; /* I IV V */
        static const char *const NUM[7] = {"I", "II", "III", "IV", "V", "VI", "VII"};
        int harm = lv == 2 && rnd(2), sev = lv == 2 && rnd(2), d = lv == 0 ? q_pick(PRIMARY, 3) : (int)rnd(7), ty;
        const scale_t *s = &SCALES[harm ? SC_HARMMINOR : SC_MAJOR];
        nt_t r = ROOTS[lv == 0 ? q_pick(EASY, 3) : harm ? q_pick(COMMON_MINOR_ROOTS, sizeof COMMON_MINOR_ROOTS) :
                       q_pick(COMMON_MAJOR_ROOTS, sizeof COMMON_MAJOR_ROOTS)], cr;
        ty = degree_chord(s, d, sev ? 4 : 3);
        cr = scale_note(s, r, d);
        q.kind = K_SET;
        q.n = CHORDS[ty].n;
        for (i = 0; i < q.n; i++) {
            q.nt[i] = chord_note(ty, cr, i);
            q.pc[i] = (uint8_t)pc_of(q.nt[i]);
        }
        note_full(r, a);
        t1_fmt(q.l1, S(harm ? S_Q_DEG_MIN : S_Q_DEG_MAJ), a, 0, 0);
        t1_fmt(q.l2, S(sev ? S_Q_DEG_SEV : S_Q_DEG_TRI), NUM[d], 0, 0);
        chord_name(ty, cr, a);
        chord_symbol(ty, cr, b);
        t1_strcpy(q.ans, a);
        str_cat(q.ans, " - ");
        str_cat(q.ans, b);
        q.ref = r;
        break;
    }
    case QT_EAR_INT: {
        static const uint8_t L1[5] = {3, 4, 5, 7, 12}, L2[9] = {1, 2, 3, 4, 5, 7, 8, 9, 12},
                             L3[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        const uint8_t *L = lv == 0 ? L1 : lv == 1 ? L2 : L3;
        int nl = lv == 0 ? 5 : lv == 1 ? 9 : 12, iv = q_pick(L, nl), root = 55 + (int)rnd(10);
        const char *pool[12];
        for (i = 0; i < nl; i++)
            pool[i] = TR(INTERVALS[L[i]].name);
        q.listen[0] = (uint8_t)root;
        q.listen[1] = (uint8_t)(root + INTERVALS[iv].semis);
        q.nlisten = 2;
        q.listen_kind = 0;
        t1_strcpy(q.l1, S(S_Q_EAR_INT));
        t1_strcpy(q.l2, S(S_Q_REPLAY));
        q_choices(TR(INTERVALS[iv].name), pool, nl, 4);
        t1_strcpy(q.ans, TR(INTERVALS[iv].name));
        break;
    }
    case QT_EAR_CHORD: {
        static const uint8_t L1[2] = {CH_MAJ, CH_MIN}, L2[4] = {CH_MAJ, CH_MIN, CH_DIM, CH_AUG},
                             L3[7] = {CH_MAJ, CH_MIN, CH_MAJ7, CH_DOM7, CH_MIN7, CH_HALFDIM, CH_DIM7};
        const uint8_t *L = lv == 0 ? L1 : lv == 1 ? L2 : L3;
        int nl = lv == 0 ? 2 : lv == 1 ? 4 : 7, ty = q_pick(L, nl), root = 55 + (int)rnd(9);
        const char *pool[7];
        for (i = 0; i < nl; i++)
            pool[i] = TR(CHORDS[L[i]].name);
        q.nlisten = CHORDS[ty].n;
        for (i = 0; i < q.nlisten; i++)
            q.listen[i] = (uint8_t)(root + CHORDS[ty].semi[i]);
        q.listen_kind = 1;
        t1_strcpy(q.l1, S(S_Q_EAR_CHORD));
        t1_strcpy(q.l2, S(S_Q_REPLAY));
        q_choices(TR(CHORDS[ty].name), pool, nl, 4);
        t1_strcpy(q.ans, TR(CHORDS[ty].name));
        break;
    }
    case QT_KEYSIG: default: {
        int maxc = lv == 0 ? 2 : lv == 1 ? 4 : 7, k, sub = (int)rnd(3), c;
        const char *pool[16];
        static char names[16][40];
        int np = 0;
        do
            k = (int)rnd(NKEYSIG);
        while ((KEYSIGS[k].count < 0 ? -KEYSIGS[k].count : KEYSIGS[k].count) > maxc);
        c = KEYSIGS[k].count;
        note_full(KEYSIGS[k].root, a);
        if (sub == 0) {                      /* how many? */
            int x;
            t1_strcpy(q.l1, S(S_Q_HOWMANY));
            t1_fmt(q.l2, S(S_MAJOR_Q), a, 0, 0);
            for (x = -maxc; x <= maxc && np < 16; x++) {
                count_text(x, names[np]);
                if (x == c)
                    t1_strcpy(b, names[np]);
                pool[np] = names[np];
                np++;
            }
        } else if (sub == 1) {               /* the relative minor */
            char m[24];
            t1_strcpy(q.l1, S(S_Q_REL));
            t1_fmt(q.l2, S(S_MAJOR_Q), a, 0, 0);
            note_full(relative_minor(KEYSIGS[k].root), m);
            t1_fmt(b, S(S_MINOR_OF), m, 0, 0);
            for (i = 0; i < NKEYSIG && np < 16; i++) {
                int cc = KEYSIGS[i].count < 0 ? -KEYSIGS[i].count : KEYSIGS[i].count;
                if (cc > maxc + 1)
                    continue;
                note_full(relative_minor(KEYSIGS[i].root), m);
                t1_fmt(names[np], S(S_MINOR_OF), m, 0, 0);
                pool[np] = names[np];
                np++;
            }
        } else {                             /* which key has these? */
            char m[24];
            t1_strcpy(q.l1, S(S_Q_WHICH));
            if (c == 0)
                t1_strcpy(q.l2, S(S_Q_NOALT_Q));
            else {
                count_text(c, q.l2);
                str_cat(q.l2, "?");
            }
            t1_fmt(b, S(S_MAJOR_OF), a, 0, 0);
            for (i = 0; i < NKEYSIG && np < 16; i++) {
                int cc = KEYSIGS[i].count < 0 ? -KEYSIGS[i].count : KEYSIGS[i].count;
                if (cc > maxc + 1)
                    continue;
                note_full(KEYSIGS[i].root, m);
                t1_fmt(names[np], S(S_MAJOR_OF), m, 0, 0);
                pool[np] = names[np];
                np++;
            }
        }
        q_choices(b, pool, np, 4);
        t1_strcpy(q.ans, b);
        q.ref = KEYSIGS[k].root;
        break;
    }
    }
}

static void q_make(int t)
{
    int tries = 0;
    do
        q_make1(t);
    while (q.kind != K_CHOICE && awkward(q.nt, q.n, cfg.level) && ++tries < 50);
}

static void q_new(void)
{
    int t = qtype_now == QT_MIXED ? 1 + (int)rnd(NQT - 1) : qtype_now;
    play_stop();
    q_make(t);
    q.num++;
    if (q.ref_pc >= 0)
        q_play_ref();
    if (q.nlisten)
        q_play_listen();
    ui.redraw = 1;
}

static void quiz_start(void)
{
    t1_rng ^= plat_ms() * 2654435761u + 1u;
    qtype_now = ui.qsel;
    q.num = 0;
    q.score = 0;
    ui.screen = SCR_QUIZ;
    q_new();
}

static int q_points(void)
{
    int p = 10 - 2 * q.errors - 3 * q.hints;
    return p < 2 ? 2 : p;
}

/* the question answered (right 1, wrong 0, skipped 2): points, the answer played */
static void q_finish(int right)
{
    uint32_t t = plat_ms() + 150u;
    int i, b;
    char s[24];
    q.done = (uint8_t)(right == 1 ? 1 : 2);
    q.done_at = plat_ms();
    q.gained = (uint8_t)(right == 1 ? q_points() : 0);
    q.score = (uint8_t)(q.score + q.gained);
    if (right == 1) {
        int_str(s, q.gained);
        t1_fmt(q.fb, S(S_F_RIGHT), s, 0, 0);
        q.fb_col = COL_OK;
    } else {
        q_feedback(S(right == 2 ? S_F_SKIPPED : S_F_WRONG), COL_BAD);
    }
    if (q.kind == K_CHOICE) {                /* an ear question: heard again */
        if (q.nlisten)
            q_play_listen();
        return;
    }
    b = base_midi(q.nt[0]);                   /* the answer, heard: the note, the scale up, the chord */
    if (q.ref_pc >= 0) {
        play_note(t, base_midi(q.ref), 500u, 80);
        play_note(t + 500u, base_midi(q.ref) + (q.pc[0] - q.ref_pc + 12) % 12 + ((q.pc[0] == q.ref_pc) ? 12 : 0),
                  700u, 90);
    } else if (q.kind == K_SEQ) {
        int prev = b;
        for (i = 0; i < q.n; i++, t += 170u) {
            int nn = b + (q.pc[i] - q.pc[0] + 12) % 12;
            if (nn < prev)
                nn += 12;
            prev = nn;
            play_note(t, nn, 160u, 85);
        }
        play_note(t, b + 12, 400u, 90);
    } else {
        int prev = b;
        for (i = 0; i < q.n; i++) {
            int nn = b + (q.pc[i] - q.pc[0] + 12) % 12;
            if (nn < prev)
                nn += 12;
            prev = nn;
            play_note(t, nn, 900u, 80);
        }
    }
}

/* a note played during a question */
static void q_note(int note)
{
    int pc = note % 12, i;
    char nn[16];
    if (q.done || q.kind == K_CHOICE)
        return;
    note_full(nt_of_pc(pc), nn);
    if (q.kind == K_ONE) {
        if (pc == q.pc[0])
            q_finish(1);
        else if (q.ref_pc >= 0 && pc == q.ref_pc) {
            q_feedback(S(S_F_START), C_GRAY);
        } else {
            q.errors++;
            q.wrong_pc = (int8_t)pc;
            t1_fmt(q.fb, S(S_F_PLAYED), nn, 0, 0);
            q.fb_col = COL_BAD;
        }
    } else if (q.kind == K_SEQ) {
        if (pc == q.pc[q.pos]) {
            q.pos++;
            q.wrong_pc = -1;
            q.hint_on = 0;
            q_feedback(S(q.pos == 1 ? S_F_GOON : S_F_YES), COL_OK);
            if (q.pos >= q.n)
                q_finish(1);
        } else if (q.pos > 0 && pc == q.pc[q.pos - 1]) {
            /* the same note again: no harm */
        } else {
            q.errors++;
            q.wrong_pc = (int8_t)pc;
            t1_fmt(q.fb, S(q.pos == 0 ? S_F_FROM_TONIC : S_F_NOT_NEXT), nn, 0, 0);
            q.fb_col = COL_BAD;
        }
    } else {                                  /* K_SET */
        for (i = 0; i < q.n; i++)
            if (q.pc[i] == pc)
                break;
        if (i < q.n) {
            q.found |= (uint16_t)(1u << i);
            q.wrong_pc = -1;
            q_feedback(S(S_F_IN_CHORD), COL_OK);
            if (q.found == (1u << q.n) - 1u)
                q_finish(1);
        } else {
            q.errors++;
            q.wrong_pc = (int8_t)pc;
            t1_fmt(q.fb, S(S_F_NOT_CHORD), nn, 0, 0);
            q.fb_col = COL_BAD;
        }
    }
    if (!q.done && q.errors >= 3 && !q.hint_on) {   /* three wrong: the answer's next notes light up */
        q.hint_on = 1;
        str_cat(q.fb, S(S_F_LOOK));
    }
    ui.redraw = 1;
}

static void q_hint(void)
{
    if (q.done)
        return;
    if (q.kind == K_CHOICE) {                /* a wrong choice struck out */
        int i, left = 0;
        for (i = 0; i < q.nopt; i++)
            left += i != q.correct && !(q.gone >> i & 1u);
        if (left > 1)
            for (;;) {
                int k = (int)rnd(q.nopt);
                if (k != q.correct && !(q.gone >> k & 1u)) {
                    q.gone |= (uint8_t)(1u << k);
                    q.hints++;
                    break;
                }
            }
    } else if (!q.hint_on) {
        q.hint_on = 1;
        q.hints++;
    }
    q_feedback(S(S_F_HINT), COL_SCALE);
    ui.redraw = 1;
}

static void q_answer_choice(void)
{
    if (q.done || q.kind != K_CHOICE || (q.gone >> q.sel & 1u))
        return;
    if (q.sel == q.correct)
        q_finish(1);
    else {
        q_finish(0);
        q.errors = 1;
    }
    ui.redraw = 1;
}

static void quiz_next(void)
{
    if (q.num >= NQ) {
        q.record = 0;
        if (q.score > cfg.best[qtype_now]) {
            cfg.best[qtype_now] = q.score;
            set_dirty = 1;
            q.record = 1;
            settings_save();
        }
        ui.screen = SCR_QEND;
        play_stop();
        {                                     /* a little fanfare */
            uint32_t t = plat_ms() + 100u;
            static const uint8_t UP[4] = {60, 64, 67, 72}, DOWN[4] = {64, 60, 57, 52};
            const uint8_t *m = q.score >= 60 ? UP : DOWN;
            int i;
            for (i = 0; i < 4; i++)
                play_note(t + (uint32_t)i * 140u, m[i], i == 3 ? 700u : 130u, 85);
        }
        ui.redraw = 1;
    } else {
        q_new();
    }
}

static void draw_quiz(void)
{
    char t[96], s[24], u[24];
    int i;
    int_str(s, q.num);
    int_str(u, NQ);
    t1_fmt(t, S(S_Q_HEADER), s, u, 0);
    int_str(u, q.score);
    t1_fmt(s, S(S_Q_SCORE), u, 0, 0);
    ui_header(t, s, COL_ROOT);
    t1_tx(8, 29, &FONT_XS, q.l1, C_GRAY);
    t1_tx(8, 42, fit_font(q.l2, 228), q.l2, q.kind == K_CHOICE && q.nlisten ? C_DIM : C_WHITE);
    mk.scale = mk.root = mk.ok = mk.bad = mk.hint = 0;
    if (q.ref_pc >= 0)
        mk.root = (uint16_t)PCB(q.ref_pc);
    if (q.wrong_pc >= 0 && !q.done)
        mk.bad = (uint16_t)PCB(q.wrong_pc);
    if (q.kind == K_CHOICE) {
        for (i = 0; i < q.nopt; i++) {
            int32_t y = 72 + i * 22;
            uint16_t bg = COL_PANEL, fg = C_HI;
            if (q.done && i == q.correct)
                bg = RGB(30, 110, 60), fg = C_WHITE;
            else if (q.done && i == q.sel && q.sel != q.correct)
                bg = RGB(120, 40, 35), fg = C_WHITE;
            else if (i == q.sel)
                bg = COL_SELBG, fg = C_WHITE;
            cv_rect(4, y, 232, 20, bg);
            s[0] = (char)('A' + i);
            s[1] = 0;
            t1_tx(10, y + 2, &FONT_B, s, C_GRAY);
            t1_tx(30, y + 2, t1_tw(&FONT_B, q.opt[i]) <= 200 ? &FONT_B : &FONT_XS, q.opt[i], (q.gone >> i & 1u) ? C_DIM : fg);
            if (q.gone >> i & 1u)
                cv_rect(28, y + 10, t1_tw(&FONT_B, q.opt[i]) + 4, 1, C_GRAY);
        }
    } else {
        /* the answer's boxes: found notes named, the rest '?' (named in blue with a hint, all named at the end) */
        int n = q.n;
        int32_t w = (232 - (n - 1) * 3) / n, bh = box_h(), y2;
        if (w > 44)
            w = 44;
        for (i = 0; i < n; i++) {
            int got = q.kind == K_SEQ ? i < q.pos : q.kind == K_SET ? (q.found >> i & 1u) : q.done == 1;
            int show_hint = q.hint_on && !got && (q.kind != K_SEQ || i == q.pos);
            uint16_t bg = got ? RGB(30, 110, 60) : show_hint ? RGB(40, 60, 120) : COL_PANEL;
            char nn[8], aa[8];
            int32_t x = 4 + i * (w + 3);
            aa[0] = 0;
            if (got || show_hint || q.done) {
                note_name(q.nt[i], nn);
                note_alt(q.nt[i], aa);
            } else {
                t1_strcpy(nn, "?");
            }
            note_box(x, 72, w, bh, nn, aa, bg, C_WHITE, q.kind == K_SEQ && i == q.pos && !q.done ? C_HI : C_LINE);
            if (got)
                mk.ok |= (uint16_t)PCB(q.pc[i]);
            if (show_hint)
                mk.hint |= (uint16_t)PCB(q.pc[i]);
        }
        y2 = 72 + bh + 4;
        if (q.fb[0])
            t1_tx(8, y2, t1_tw(&FONT_B, q.fb) <= 226 ? &FONT_B : &FONT_XS, q.fb, q.fb_col);
        if (q.done) {
            if (q.ans[0]) {
                if (q.done == 1)
                    t1_strcpy(t, q.ans);
                else
                    t1_fmt(t, S(S_F_ANSWER), q.ans, 0, 0);
                text_wrap(8, y2 + 20, 226, &FONT_XS, t, q.done == 1 ? C_HI : C_WHITE, 13, 184);
            }
        } else {
            int_str(u, q.errors);
            t1_fmt(t, S(S_F_ERRORS), u, 0, 0);
            if (q.hints) {
                int_str(u, q.hints);
                t1_fmt(s, S(S_F_HINTS), u, 0, 0);
                str_cat(t, s);
            }
            t1_tx(8, y2 + 20, &FONT_XS, t, C_GRAY);
            text_wrap(8, y2 + 36, 226, &FONT_XS,
                      S(q.kind == K_SEQ ? S_F_HELP_SEQ : q.kind == K_SET ? S_F_HELP_SET : S_F_HELP_ONE), C_DIM, 13,
                      184);
        }
    }
    if (q.kind == K_CHOICE) {
        if (q.fb[0])
            t1_tx(8, 160, &FONT_B, q.fb, q.fb_col);
        if (q.done && q.done != 1 && q.ans[0]) {
            t1_fmt(t, S(S_F_ANSWER), q.ans, 0, 0);
            t1_tx(8 + t1_tw(&FONT_B, q.fb) + 10, 163, &FONT_XS, t, C_WHITE);
        }
    }
    if (q.done)                               /* the answer on the keys */
        for (i = 0; i < q.n && q.kind != K_CHOICE; i++)
            mk.ok |= (uint16_t)PCB(q.pc[i]);
    hint(S(q.done ? S_H_NEXT : q.kind == K_CHOICE ? S_H_QCHOICE : S_H_QPLAY));
}

/* a five-point star, filled */
static void star(int32_t cx, int32_t cy, int32_t r, uint16_t c)
{
    static const int8_t PX[10] = {0, 22, 95, 36, 59, 0, -59, -36, -95, -22},
                        PY[10] = {-100, -31, -31, 12, 81, 38, 81, 12, -31, -31};
    int32_t x, y;
    for (y = -r; y <= r; y++)
        for (x = -r; x <= r; x++) {
            int i, j, in = 0;
            int32_t X = x * 100 / r, Y = y * 100 / r;
            for (i = 0, j = 9; i < 10; j = i++)
                if ((PY[i] > Y) != (PY[j] > Y) &&
                    X < (PX[j] - PX[i]) * (Y - PY[i]) / (PY[j] - PY[i] ? PY[j] - PY[i] : 1) + PX[i])
                    in = !in;
            if (in)
                cv_pset(cx + x, cy + y, c);
        }
}

static void draw_qend(void)
{
    char t[48], u[16];
    int i, stars = q.score >= 90 ? 3 : q.score >= 70 ? 2 : q.score >= 40 ? 1 : 0;
    ui_header(S(S_END), QT_NAMES[th_ui][qtype_now], C_GRAY);
    int_str(t, q.score);
    str_cat(t, "/100");
    text_c(34, &FONT_L, t, C_WHITE);
    for (i = 0; i < 3; i++)
        star(80 + i * 40, 94, 16, i < stars ? COL_ROOT : C_LINE);
    text_c(118, &FONT_B, S(stars == 3 ? S_END_3 : stars == 2 ? S_END_2 : stars == 1 ? S_END_1 : S_END_0), C_HI);
    int_str(u, cfg.best[qtype_now]);
    t1_fmt(t, S(S_BEST), u, 0, 0);
    text_c(140, &FONT_B, q.record ? S(S_NEW_BEST) : t, q.record ? COL_OK : C_GRAY);
    t1_fmt(t, S(S_LEVEL), LEVEL_NAMES[th_ui][cfg.level], 0, 0);
    text_c(162, &FONT_XS, t, C_GRAY);
    hint(S(S_H_END));
    mk.scale = 0;
    mk.root = 0;
}

/* -------------------------------------------------------- the laboratory --- */
#define NLABSCALES (1 + NSCALES + NMODES)
static int lab_scale(scale_t *s)               /* 0: none */
{
    int i = ui.lab_scale;
    if (i == 0)
        return 0;
    if (i <= NSCALES)
        *s = SCALES[i - 1];
    else
        mode_scale(i - 1 - NSCALES, s);
    return 1;
}

/* a pitch class spelled in the lab: by the scale chosen if it is in it, else the usual way */
static nt_t lab_spell(int pc)
{
    scale_t s;
    int i;
    if (lab_scale(&s))
        for (i = 0; i < s.n; i++) {
            nt_t n = scale_note(&s, ROOTS[ui.lab_root], i);
            if (pc_of(n) == pc)
                return n;
        }
    return nt_of_pc(pc);
}

static int lab_scale_play(void)
{
    scale_t s;
    if (!lab_scale(&s))
        return 0;
    play_stop();
    play_scale(&s, ROOTS[ui.lab_root], 1);
    return 1;
}

static void draw_lab(void)
{
    char t[96], u[64], nn[16];
    int i, count = 0, low = -1, high = -1, inscale = 1;
    uint32_t mask = 0;
    scale_t s;
    int has = lab_scale(&s);
    for (i = 0; i < 128; i++)
        if (ui.held[i]) {
            count++;
            mask |= 1u << (i % 12);
            if (low < 0)
                low = i;
            high = i;
        }
    if (has) {
        note_name(ROOTS[ui.lab_root], t);
        str_cat(t, " ");
        str_cat(t, TR(s.name));
    } else {
        t1_strcpy(t, S(S_NO_SCALE));
    }
    ui_header(S(S_M_LAB), t, has ? COL_SCALE : C_DIM);
    mk.scale = mk.root = mk.ok = mk.bad = mk.hint = 0;
    if (has) {
        for (i = 0; i < s.n; i++)
            mk.scale |= (uint16_t)PCB(pc_of(scale_note(&s, ROOTS[ui.lab_root], i)));
        mk.root = (uint16_t)PCB(pc_of(ROOTS[ui.lab_root]));
        if (mask & ~(uint32_t)mk.scale)
            inscale = 0;
        mk.bad = (uint16_t)(mask & ~(uint32_t)mk.scale);
    }
    if (count == 0) {
        t1_tx(8, 30, &FONT_M, S(S_PLAY_SOMETHING), C_WHITE);
        para(8, 62, 226, S(S_LAB_INTRO), C_GRAY, 144);
    } else if (count == 1) {
        nt_t n = lab_spell(low % 12);
        note_full(n, t);
        t1_tx(8, 29, &FONT_M, t, C_WHITE);
        if (n.acc) {                          /* its other name */
            nt_t e = spell(n, 0, n.acc > 0 ? 1 : 6);
            note_full(e, u);
            t1_strcpy(t, "= ");
            str_cat(t, u);
            t1_tx(8, 56, &FONT_B, t, C_GRAY);
        }
        int_str(u, low / 12 - 1);
        int_str(nn, low);
        t1_fmt(t, S(S_OCTAVE_MIDI), u, nn, 0);
        t1_tx(8, 76, &FONT_XS, t, C_GRAY);
        if (has) {
            for (i = 0; i < s.n; i++)
                if (pc_of(scale_note(&s, ROOTS[ui.lab_root], i)) == low % 12)
                    break;
            if (i < s.n) {
                int_str(u, i + 1);
                t1_fmt(t, S(S_DEGREE_OF), u, 0, 0);
                if (s.n == 7) {
                    str_cat(t, " (");
                    str_cat(t, degree_name(&s, i));
                    str_cat(t, ")");
                }
            } else {
                t1_strcpy(t, S(S_OUT_OF_SCALE));
            }
            t1_tx(8, 92, t1_tw(&FONT_B, t) <= 226 ? &FONT_B : &FONT_XS, t, i < s.n ? COL_SCALE : COL_BAD);
        }
    } else if (t1_bits(mask) <= 2) {             /* two notes (or one note doubled): the interval */
        int semis = high - low, iv = semis % 12;
        nt_t a = lab_spell(low % 12), b = lab_spell(high % 12);
        if (t1_bits(mask) == 1)
            iv = 12;
        t1_strcpy(t, TR(INTERVALS[iv].name));
        if (semis > 12 && iv != 12)
            str_cat(t, S(S_PLUS_OCTAVE));
        t1_tx(8, 29, fit_font(t, 228), t, C_WHITE);
        note_full(a, u);
        t1_strcpy(t, u);
        str_cat(t, " - ");
        note_full(b, u);
        str_cat(t, u);
        str_cat(t, "   ");
        cat_int(t, semis);
        str_cat(t, " ");
        str_cat(t, S(semis == 1 ? S_SEMITONE : S_SEMITONES));
        t1_tx(8, 58, t1_tw(&FONT_B, t) <= 226 ? &FONT_B : &FONT_XS, t, C_HI);
        text_wrap(8, 78, 226, &FONT_XS, TR(INTERVALS[iv].text), C_GRAY, 13, 130);
    } else {
        int ty, rp, inv;
        if (chord_find(mask, low % 12, &ty, &rp, &inv)) {
            nt_t r = lab_spell(rp), cn[4];
            int k;
            int32_t x;
            char a[48];
            chord_name(ty, r, t);
            t1_tx(8, 29, fit_font(t, 226), t, C_WHITE);
            chord_symbol(ty, r, u);
            x = t1_tx(8, 56, &FONT_B, u, COL_ROOT);
            if (inv >= 0) {
                t1_tx(x + 12, 56, &FONT_B, INV_NAME[th_ui][inv], C_HI);
                t1_tx(8, 110, &FONT_XS, INV_WHY[th_ui][inv], C_GRAY);
            }
            for (k = 0; k < CHORDS[ty].n; k++)
                cn[k] = chord_note(ty, r, k);
            notes_text(cn, CHORDS[ty].n, a);
            t1_fmt(t, S(S_NOTES), a, CHORDS[ty].formula, 0);
            t1_tx(8, 76, &FONT_XS, t, C_GRAY);
            mk.root = (uint16_t)PCB(rp);
            if (has) {                        /* its degree in the scale */
                for (k = 0; k < s.n; k++)
                    if (pc_of(scale_note(&s, ROOTS[ui.lab_root], k)) == rp)
                        break;
                if (k < s.n && s.n == 7 && inscale) {
                    roman(k, ty, u);
                    t1_tx(t1_tx(8, 90, &FONT_B, S(S_DEGREE), COL_SCALE), 90, &FONT_B, u, COL_SCALE);
                }
            }
        } else {
            t1_tx(8, 29, &FONT_B, S(S_UNKNOWN_CHORD), C_WHITE);
            t[0] = 0;
            for (i = 0; i < 12; i++)
                if (mask >> i & 1u) {
                    note_name(lab_spell(i), nn);
                    str_cat(t, nn);
                    str_cat(t, " ");
                }
            t1_tx(8, 50, &FONT_B, t, C_HI);
            text_wrap(8, 72, 226, &FONT_XS, S(S_UNKNOWN_TIP), C_GRAY, 13, 130);
        }
    }
    if (has && count && !inscale)
        t1_tx(8, 132, &FONT_B, S(S_OUT_RED), COL_BAD);
    {                                         /* the last notes played: the newest that fit */
        char h[64];
        int first = ui.nhist;
        for (;;) {
            if (first == 0)
                break;
            h[0] = 0;
            for (i = first - 1; i < ui.nhist; i++) {
                note_name(lab_spell(ui.hist[i] % 12), nn);
                str_cat(h, nn);
                str_cat(h, " ");
            }
            t1_strcpy(t, S(S_LAST));
            str_cat(t, h);
            if (t1_tw(&FONT_XS, t) > 226)
                break;
            first--;
        }
        t1_strcpy(t, S(S_LAST));
        for (i = first; i < ui.nhist; i++) {
            note_name(lab_spell(ui.hist[i] % 12), nn);
            str_cat(t, nn);
            str_cat(t, " ");
        }
    }
    cv_rect(4, 152, 232, 1, C_LINE);
    t1_tx(8, 158, &FONT_XS, t, C_GRAY);
    t1_tx(8, 172, &FONT_XS, S(has ? S_LAB_SEL1 : S_LAB_SEL0), C_DIM);
    hint(S(S_H_LAB));
}

/* -------------------------------------------------------------- settings --- */
enum { ST_LANG, ST_NAMES, ST_SOUND, ST_SPEED, ST_EXT, ST_BLE, ST_DEVICE, ST_RESET, NST };
static void draw_settings(void)
{
    static const int LABEL[NST] = {S_SET_LANG, S_SET_NAMES, S_SET_SOUND, S_SET_SPEED, S_SET_EXT, S_SET_BLE,
                                   S_SET_DEVICE, S_SET_RESET};
    int i;
    ui_header(S(S_M_SETTINGS), set_dirty ? "SAVE" : 0, COL_ROOT);
    for (i = 0; i < NST; i++) {
        int32_t y = 29 + i * 19, sel = i == ui.set_sel;
        const char *v = "";
        char dev[40];
        switch (i) {
        case ST_LANG: v = LANG_SELF[cfg.ui % NLANGS]; break;
        case ST_NAMES: v = NAMES_SELF[cfg.names % NNAMES]; break;
        case ST_SOUND: v = SOUND_NAMES[th_ui][cfg.sound % NSOUNDS]; break;
        case ST_SPEED: v = SPEED_NAMES[th_ui][cfg.speed % 3]; break;
        case ST_EXT: v = S(cfg.ext_mute ? S_EXT_MUTE : S_EXT_BOTH); break;
        case ST_BLE: v = !FM1B_BLE ? S(S_NOT_BUILT) : S(cfg.ble_off ? S_OFF : S_ON); break;
        case ST_DEVICE:
            if (!FM1B_BLE_CENTRAL)
                v = "-";
            else if (plat_ble_device() == BLE_DEV_READY) {
                t1_fmt(dev, S(S_CONNECTED), btdev_name(), 0, 0);
                v = dev;
            } else
                v = S(ui.looking ? S_SEARCHING : S_REC_SEARCH);
            break;
        case ST_RESET: v = S(S_SEL_RESET); break;
        default: break;
        }
        if (sel)
            cv_rect(4, y, 232, 18, COL_SELBG);
        t1_tx(10, y + 3, &FONT_XS, S(LABEL[i]), sel ? C_WHITE : C_GRAY);
        text_r(230, y + 3, &FONT_XS, v, sel ? COL_ROOT : C_HI);
    }
    hint(S(S_H_SETTINGS));
}

static void settings_change(int d)
{
    switch (ui.set_sel) {
    case ST_LANG: cfg.ui = (uint8_t)((cfg.ui + NLANGS + d) % NLANGS); break;
    case ST_NAMES: cfg.names = (uint8_t)((cfg.names + NNAMES + d) % NNAMES); break;
    case ST_SOUND: cfg.sound = (uint8_t)((cfg.sound + NSOUNDS + d) % NSOUNDS); break;
    case ST_SPEED: cfg.speed = (uint8_t)((cfg.speed + 3 + d) % 3); break;
    case ST_EXT: cfg.ext_mute = (uint8_t)!cfg.ext_mute; break;
    case ST_BLE:
        if (!FM1B_BLE)
            return;
        cfg.ble_off = (uint8_t)!cfg.ble_off;
        plat_ble_enable(!cfg.ble_off);
        break;
    case ST_RESET: {
        int i;
        for (i = 0; i < NQT; i++)
            cfg.best[i] = 0;
        app_say(S(S_SAY_RECORDS), S(S_SAY_RESET));
        break;
    }
    default: return;
    }
    set_dirty = 1;
    settings_apply();
    if (ui.set_sel == ST_SOUND) {            /* hear it */
        play_stop();
        play_note(plat_ms() + 20u, 60, 300u, 90);
        play_note(plat_ms() + 20u, 64, 300u, 90);
        play_note(plat_ms() + 20u, 67, 300u, 90);
    }
}

/* ================================================================= input === */
static void lesson_play(void)
{
    int it = ui.item[ui.topic], i;
    nt_t root = ROOTS[ui.root[ui.topic]];
    uint32_t t = plat_ms() + 30u;
    play_stop();
    switch (ui.topic) {
    case T_BASICS: {
        const page_t *p = &BASICS[it];
        for (i = 0; i < p->nplay; i++)
            play_note(p->together ? t : t + (uint32_t)i * step_ms(), p->play[i], p->together ? 1500u : step_ms() - 30u,
                      90);
        if (p->together)
            for (i = 0; i < p->nplay; i++)
                play_note(t + 1700u + (uint32_t)i * step_ms(), p->play[i], step_ms(), 80);
        break;
    }
    case T_INTERVALS: play_interval(base_midi(root), INTERVALS[it].semis, 1); break;
    case T_SCALES: play_scale(&SCALES[it], root, 1); break;
    case T_MODES: {
        scale_t s;
        mode_scale(it, &s);
        play_scale(&s, root, 1);
        break;
    }
    case T_CHORDS: play_chord(it, root, t, 1); break;
    case T_HARMONY: {
        scale_t s;
        int ty;
        hscale(it, &s);
        ty = degree_chord(&s, ui.hdeg, ui.hseventh ? 4 : 3);
        if (ty >= 0) {                       /* in its place: above the scale's root */
            int b = base_midi(root) + s.semi[ui.hdeg];
            for (i = 0; i < CHORDS[ty].n; i++)
                play_note(t, b + CHORDS[ty].semi[i], 1500u, 85);
        }
        break;
    }
    case T_KEYS: play_scale(&SCALES[SC_MAJOR], KEYSIGS[it].root, 0); break;
    default: break;
    }
}

/* ARP in Harmony: every degree's chord in turn, and the tonic again */
static void harmony_all(void)
{
    scale_t s;
    nt_t root = ROOTS[ui.root[T_HARMONY]];
    uint32_t t = plat_ms() + 30u, st = step_ms() * 2u;
    int d, i, b = base_midi(root);
    hscale(ui.item[T_HARMONY], &s);
    play_stop();
    for (d = 0; d <= 7; d++, t += st) {
        int ty = degree_chord(&s, d % 7, ui.hseventh ? 4 : 3);
        for (i = 0; ty >= 0 && i < CHORDS[ty].n; i++)
            play_note(t, b + s.semi[d % 7] + 12 * (d / 7) + CHORDS[ty].semi[i], d == 7 ? st * 2u : st - 40u, 80);
    }
}

/* a note in, from a key or MIDI: sounded (unless an outside keyboard sounds itself), and given to the screen */
static void note_in(int note, int vel, int on, int outside)
{
    if (note < 0 || note > 127)
        return;
    if (on) {
        if (!outside || !cfg.ext_mute)
            synth_note_on(note, vel);
        if (ui.held[note] < 255)
            ui.held[note]++;
        if (ui.screen == SCR_LAB) {
            int i;
            if (ui.nhist == 8)
                for (i = 0; i < 7; i++)
                    ui.hist[i] = ui.hist[i + 1];
            ui.hist[ui.nhist < 8 ? ui.nhist++ : 7] = (uint8_t)note;
        } else if (ui.screen == SCR_QUIZ) {
            q_note(note);
        }
    } else {
        if (!outside || !cfg.ext_mute)
            synth_note_off(note);
        if (ui.held[note])
            ui.held[note]--;
    }
    ui.redraw = 1;
}

/* the host's "solve" and "wrong": notes played as a player would, a moment apart */
static struct {
    uint8_t note[16], n, i;
    uint32_t next;
} fake;

static void midi_in(void)
{
    uint32_t pkt;
    int port;
    while (plat_midi_in(&pkt, &port)) {
        uint32_t st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = pkt >> 24 & 0x7Fu;
        if ((st & 0xF0u) == 0x90u && d2)
            note_in((int)d1, (int)d2, 1, 1);
        else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u)
            note_in((int)d1, 0, 0, 1);
    }
}

static void ui_go(int screen)
{
    ui.screen = (uint8_t)screen;
    ui.redraw = 1;
    play_stop();
}

static void ui_back(void)
{
    switch (ui.screen) {
    case SCR_LESSON: ui_go(SCR_TOPICS); break;
    case SCR_QUIZ: case SCR_QEND: ui_go(SCR_QSETUP); break;
    case SCR_SETTINGS: settings_save(); ui_go(SCR_MENU); break;
    default: ui_go(SCR_MENU); break;
    }
}

static void ui_enter(void)
{
    switch (ui.screen) {
    case SCR_MENU:
        ui_go(ui.menu == 0 ? SCR_TOPICS : ui.menu == 1 ? SCR_QSETUP : ui.menu == 2 ? SCR_LAB : SCR_SETTINGS);
        break;
    case SCR_TOPICS:
        ui.topic = ui.topic_sel;
        ui_go(SCR_LESSON);
        lesson_play();
        break;
    case SCR_QSETUP: quiz_start(); break;
    case SCR_QUIZ:
        if (q.done)
            quiz_next();
        else
            q_answer_choice();
        break;
    case SCR_QEND: quiz_start(); break;
    case SCR_SETTINGS: settings_change(1); break;
    default: break;
    }
}

static int t1_wrap(int v, int n) { return ((v % n) + n) % n; }

static void ui_encoder(int e, int d)
{
    switch (ui.screen) {
    case SCR_MENU:
        if (e == EN_SELECT)
            ui.menu = (uint8_t)t1_wrap(ui.menu + d, 4);
        break;
    case SCR_TOPICS:
        if (e == EN_SELECT || e == EN_ALGO)
            ui.topic_sel = (uint8_t)t1_wrap(ui.topic_sel + d, NTOPICS);
        break;
    case SCR_LESSON: {
        int n = ui.topic == T_HARMONY ? NHSCALES : TOPIC_ITEMS[ui.topic];
        if (e == EN_SELECT)
            ui.item[ui.topic] = (uint8_t)t1_wrap(ui.item[ui.topic] + d, n);
        else if (e == EN_ALGO) {
            ui.topic = (uint8_t)t1_wrap(ui.topic + d, NTOPICS);
            ui.topic_sel = ui.topic;
        } else if (e == EN_K1 || e == EN_PRESET) {
            if (ui.topic == T_KEYS)
                ui.item[T_KEYS] = (uint8_t)t1_wrap(ui.item[T_KEYS] + d, NKEYSIG);
            else
                ui.root[ui.topic] = (uint8_t)t1_wrap(ui.root[ui.topic] + d, NROOTS);
        } else if (e == EN_K2 && ui.topic == T_HARMONY) {
            ui.hdeg = (uint8_t)t1_wrap(ui.hdeg + d, 7);
            lesson_play();                   /* a degree chosen: heard */
        } else if (e == EN_K3 && ui.topic == T_HARMONY)
            ui.hseventh = (uint8_t)!ui.hseventh;
        else
            return;
        break;
    }
    case SCR_QSETUP:
        if (e == EN_SELECT)
            ui.qsel = (uint8_t)t1_wrap(ui.qsel + d, NQT);
        else if (e == EN_K1 || e == EN_ALGO || e == EN_PRESET) {
            cfg.level = (uint8_t)t1_wrap(cfg.level + d, 3);
            set_dirty = 1;
        }
        break;
    case SCR_QUIZ:
        if (e == EN_SELECT && q.kind == K_CHOICE && !q.done)
            q.sel = (uint8_t)t1_wrap(q.sel + d, q.nopt);
        break;
    case SCR_LAB:
        if (e == EN_SELECT)
            ui.lab_scale = (uint8_t)t1_wrap(ui.lab_scale + d, NLABSCALES);
        else if (e == EN_K1 || e == EN_PRESET)
            ui.lab_root = (uint8_t)t1_wrap(ui.lab_root + d, NROOTS);
        break;
    case SCR_SETTINGS:
        if (e == EN_SELECT)
            ui.set_sel = (uint8_t)t1_wrap(ui.set_sel + d, NST);
        else if (e == EN_K1 || e == EN_PRESET)
            settings_change(d);
        if (FM1B_BLE_CENTRAL && e == EN_ALGO && ui.looking)
            btdev_choose(btdev_pick() + d);
        break;
    default: break;
    }
    ui.redraw = 1;
}

/* REC: a Bluetooth MIDI piano or keyboard (lib/btdev): look, pick with ALGORITHM, REC again to connect */
static void ble_device_button(void)
{
    int ev;
    if (!FM1B_BLE_CENTRAL || cfg.ble_off)
        return;
    if (!btdev_busy() && !ui.looking) {
        ui.looking = 1;
        app_say("BLUETOOTH", S(S_SAY_SEARCHING));
        return;
    }
    ev = btdev_save();
    ui.looking = 0;
    app_say(ev == BTDEV_CONNECTING ? S(S_SAY_CONNECTING) : "BLUETOOTH",
            ev == BTDEV_CONNECTING ? btdev_name() : btdev_word(ev));
}

/* EDIT: the next way of naming the notes */
static void names_next(void)
{
    cfg.names = (uint8_t)((cfg.names + 1u) % NNAMES);
    set_dirty = 1;
    settings_apply();
    app_say(S(S_SET_NAMES), NAMES_SELF[cfg.names]);
}

static void ui_button(int b)
{
    switch (b) {
    case B_HOME: ui_back(); return;
    case B_SEL: ui_enter(); return;
    case B_GLO: ui_go(SCR_SETTINGS); return;
    case B_OCTDN: case B_OCTUP: {
        int o = clamp(ui.octave + (b == B_OCTUP ? 1 : -1), -2, 2);
        int i;
        if (o != ui.octave) {
            for (i = 0; i < 128; i++)          /* a key held across the change would not find its note off */
                if (ui.held[i]) {
                    synth_note_off(i);
                    ui.held[i] = 0;
                }
            ui.keys = 0;
            ui.octave = (int8_t)o;
            app_say(S(S_SAY_OCTAVE), o < 0 ? (o == -1 ? "-1" : "-2") : o == 0 ? "0" : o == 1 ? "+1" : "+2");
        }
        return;
    }
    case B_SAVE:                              /* (nothing changed: nothing written) */
        settings_save();
        app_say(S(set_dirty ? S_SAY_ERROR : S_SAY_SAVED), "");
        return;
    case B_REC: ble_device_button(); return;
    default: break;
    }
    switch (ui.screen) {
    case SCR_LESSON:
        if (b == B_PLAY)
            lesson_play();
        else if (b == B_ARP && ui.topic == T_HARMONY)
            harmony_all();
        else if (b == B_EDIT)
            names_next();
        break;
    case SCR_QUIZ:
        if (b == B_PLAY) {
            if (q.nlisten)
                q_play_listen();
            else if (q.ref_pc >= 0)
                q_play_ref();
            else if (q.kind != K_CHOICE)      /* the key's root (the note quiz: C), as a reference; a key-signature
                                               * question gets nothing: its root would be the answer */
                play_note(plat_ms() + 20u, base_midi(q.ref), 700u, 90);
        } else if (b == B_EDIT)
            q_hint();
        else if (b == B_FX && !q.done) {
            q.errors = 0;
            q_finish(2);
        }
        break;
    case SCR_LAB:
        if (b == B_PLAY)
            lab_scale_play();
        else if (b == B_EDIT)
            names_next();
        break;
    default: break;
    }
    ui.redraw = 1;
}

static void ui_input(void)
{
    uint32_t keys, btn = plat_buttons(), k, now = plat_ms();
    int32_t e;
    touch_poll();
    keys = touch_keys();
    for (k = 0; k < NKEYS; k++) {            /* key 0 is F3 (MIDI 53) */
        uint32_t bit = 1u << k;
        int note = 53 + (int)k + 12 * ui.octave;
        if ((keys & bit) && !(ui.keys & bit)) {
            uint32_t v = touch_vel(k);
            note_in(note, (int)(v ? v : 100u), 1, 0);
            plat_midi_send(0x09u | 0x90u << 8 | (uint32_t)note << 16 | (v ? v : 100u) << 24, TO_USB | TO_BLE_HOST);
        } else if (!(keys & bit) && (ui.keys & bit)) {
            note_in(note, 0, 0, 0);
            plat_midi_send(0x08u | 0x80u << 8 | (uint32_t)note << 16, TO_USB | TO_BLE_HOST);
        }
    }
    ui.keys = keys;
    midi_in();
    if (fake.i < fake.n && (int32_t)(now - fake.next) >= 0) {   /* the host's notes, one every 120 ms */
        note_in(fake.note[fake.i], 100, 1, 0);
        note_in(fake.note[fake.i], 0, 0, 0);
        fake.i++;
        fake.next = now + 120u;
    }
    for (k = 0; k < NB; k++)
        if ((btn >> k & 1u) && !(ui.btn >> k & 1u))
            ui_button((int)k);
    if (btn != ui.btn)
        ui.redraw = 1;
    ui.btn = btn;
    for (k = 0; k < NE; k++)
        if ((e = plat_enc((int)k)) != 0)
            ui_encoder((int)k, e);
    play_tick();
}

void app_input_only(void) { ui_input(); }

void app_say(const char *a, const char *b)
{
    uint32_t i;
    char t[96];
    for (i = 0; i < 39u && a && a[i]; i++)
        ui.msg[0][i] = a[i];
    ui.msg[0][i] = 0;
    for (i = 0; i < 39u && b && b[i]; i++)
        ui.msg[1][i] = b[i];
    ui.msg[1][i] = 0;
    ui.msg_until = plat_ms() + 1300u;
    ui.redraw = 1;
    if (ann_on()) {                          /* and to a screen reader (announce.c, fm1-reader) */
        t1_strcpy(t, ui.msg[0]);
        if (ui.msg[1][0]) {
            str_cat(t, " ");
            str_cat(t, ui.msg[1]);
        }
        ann_say(t, 0, ANN_NOW);
    }
}

/* a screen reader that asks hears where it is */
static void speak(void)
{
    if (ann_on() && ann_where()) {
        char t[160];
        const int where[8] = {S_TAGLINE, S_M_LESSONS, S_M_LESSONS, S_M_QUIZ, S_M_QUIZ, S_END, S_M_LAB, S_M_SETTINGS};
        t1_strcpy(t, S(where[ui.screen & 7u]));
        if (ui.screen == SCR_QUIZ) {
            str_cat(t, ". ");
            str_cat(t, q.l1);
            str_cat(t, " ");
            str_cat(t, q.l2);
        }
        ann_say(t, 0, ANN_NOW);
    }
    ann_tick(plat_ms());
}

static void draw_all(void)
{
    mk.scale = mk.root = mk.ok = mk.bad = mk.hint = 0;
    btn_live = 1u << B_HOME | 1u << B_GLO | (FM1B_BLE_CENTRAL ? 1u << B_REC : 0u) | 1u << B_OCTDN | 1u << B_OCTUP;
    switch (ui.screen) {
    case SCR_MENU: draw_menu(); btn_live |= 1u << B_SEL; break;
    case SCR_TOPICS: draw_topics(); btn_live |= 1u << B_SEL; break;
    case SCR_LESSON:
        draw_lesson();
        btn_live |= 1u << B_PLAY | 1u << B_EDIT | (ui.topic == T_HARMONY ? 1u << B_ARP : 0u);
        break;
    case SCR_QSETUP: draw_qsetup(); btn_live |= 1u << B_SEL; break;
    case SCR_QUIZ:
        draw_quiz();
        btn_live |= 1u << B_PLAY | 1u << B_EDIT | 1u << B_FX | (q.kind == K_CHOICE || q.done ? 1u << B_SEL : 0u);
        break;
    case SCR_QEND: draw_qend(); btn_live |= 1u << B_SEL; break;
    case SCR_LAB: draw_lab(); btn_live |= 1u << B_PLAY | 1u << B_EDIT; break;
    case SCR_SETTINGS: draw_settings(); btn_live |= 1u << B_SEL | 1u << B_SAVE; break;
    default: break;
    }
    ui_keyboard();
}

/* the screen in two halves of 120 rows: the same drawing, each half keeping its own rows */
static void ui_draw(void)
{
    uint32_t half;
    for (half = 0; half < 2u; half++) {
        cv_begin(240, 120, C_BLACK);
        cv_oy = -(int32_t)(half * 120u);
        draw_all();
        cv_blit(0, half * 120u);
    }
    ui_lights();
}

void app_ui_init(void)
{
    touch_on(1);
    lcd_fill(0, 0, 240, 240, C_BLACK);
    ui.redraw = 1;
}

void app_frame(void)
{
    static uint32_t msg_was;
    uint32_t now = plat_ms();
    ui_input();
    speak();
    if (ui.screen == SCR_QUIZ && q.done && now - q.done_at >= (q.done == 1 ? 2200u : 3500u))
        quiz_next();
    if (FM1B_BLE_CENTRAL) {                  /* looking: a device found is named (ALGORITHM picks, REC connects) */
        static int found;
        int n = ui.looking ? btdev_count() : 0, ev;
        char name[21];
        btdev_page(ui.looking && !cfg.ble_off);
        if (n > found && btdev_found(btdev_pick(), name, 0))
            app_say(S(S_SAY_FOUND), name);
        found = n;
        while ((ev = btdev_event()) != BTDEV_NONE)
            if (ev == BTDEV_CONNECTED || ev == BTDEV_NO_ANSWER || ev == BTDEV_LOST)
                app_say("BLUETOOTH", btdev_word(ev));
    }
    if (ui.msg_until && now >= ui.msg_until) {
        ui.msg_until = 0;
        ui.redraw = 1;
    }
    if (ui.msg_until != msg_was) {
        msg_was = ui.msg_until;
        ui.redraw = 1;
    }
    if (ui.redraw) {
        ui.redraw = 0;
        ui_draw();
    }
}

void app_init(void)
{
    if (plat_store_load(OBJ_SETTINGS, &cfg, sizeof cfg) != (int)sizeof cfg || cfg.magic != SET_MAGIC ||
        cfg.names >= NNAMES || cfg.ui >= NLANGS || cfg.sound >= NSOUNDS || cfg.speed > 2u || cfg.level > 2u)
        settings_defaults();
    {
        int i;
        for (i = 0; i < NQT; i++)            /* a record past 100: none */
            if (cfg.best[i] > 100u)
                cfg.best[i] = 0;
        cfg.ext_mute = (uint8_t)(cfg.ext_mute != 0);
        cfg.ble_off = (uint8_t)(cfg.ble_off != 0);
    }
    settings_apply();
    ui.screen = SCR_MENU;
}

/* =========================================================== host checks === */
#ifdef PLAT_HOST
int app_host_fails(void) { return cut_count; }
int app_expect(const char *what, int *got)
{
    if (!strcmp(what, "screen"))
        *got = ui.screen;
    else if (!strcmp(what, "topic"))
        *got = ui.topic;
    else if (!strcmp(what, "score"))
        *got = q.score;
    else if (!strcmp(what, "qnum"))
        *got = q.num;
    else if (!strcmp(what, "qtype"))
        *got = q.type;
    else if (!strcmp(what, "done"))
        *got = q.done;
    else if (!strcmp(what, "errors"))
        *got = q.errors;
    else if (!strcmp(what, "voices"))
        *got = synth_active();
    else if (!strcmp(what, "names"))
        *got = cfg.names;
    else if (!strcmp(what, "lang"))
        *got = cfg.ui;
    else if (!strcmp(what, "level"))
        *got = cfg.level;
    else if (!strcmp(what, "best0"))
        *got = cfg.best[0];
    else if (!strcmp(what, "dirty"))
        *got = set_dirty;
    else
        return 0;
    return 1;
}

/* solve: the right answer played (or chosen); wrong: a note that is not the answer; seed N: the questions fixed;
 * quizof T: the next quiz this type; lang L: the language, at once */
int app_host_cmd(const char *cmd, const char *a, const char *b, const char *c, const char *d)
{
    int i;
    (void)b, (void)c, (void)d;
    if (!strcmp(cmd, "solve")) {
        if (ui.screen != SCR_QUIZ || q.done)
            return 1;
        if (q.kind == K_CHOICE) {
            q.sel = q.correct;
            q_answer_choice();
            return 1;
        }
        fake.n = q.n;
        fake.i = 0;
        fake.next = plat_ms();
        for (i = 0; i < q.n; i++)
            fake.note[i] = (uint8_t)(60 + q.pc[i]);
        return 1;
    }
    if (!strcmp(cmd, "wrong")) {
        if (ui.screen != SCR_QUIZ || q.done)
            return 1;
        if (q.kind == K_CHOICE) {            /* a wrong option still standing */
            for (i = 1; i < q.nopt; i++)
                if (!(q.gone >> ((q.correct + i) % q.nopt) & 1u))
                    break;
            q.sel = (uint8_t)((q.correct + i) % q.nopt);
            q_answer_choice();
            return 1;
        }
        for (i = 0; i < 12; i++) {           /* a pitch class not in the answer (nor the reference) */
            int k, in = i == q.ref_pc;
            for (k = 0; k < q.n; k++)
                in |= q.pc[k] == i;
            if (!in)
                break;
        }
        fake.n = 1;
        fake.i = 0;
        fake.note[0] = (uint8_t)(60 + i);
        fake.next = plat_ms();
        return 1;
    }
    if (!strcmp(cmd, "qcheck")) {             /* qcheck N: N questions of every type at every level, each checked */
        int n = atoi(a), t, lv, k, bad = 0, lv0 = cfg.level;
        for (lv = 0; lv < 3; lv++)
            for (t = 1; t < NQT; t++)
                for (k = 0; k < n; k++) {
                    int j, m, ok = 1;
                    cfg.level = (uint8_t)lv;
                    q_make(t);
                    if (!q.l1[0] || !q.l2[0])
                        ok = 0;
                    if (q.kind == K_CHOICE) {    /* different options, the right one among them once */
                        for (j = 0; j < q.nopt; j++)
                            for (m = j + 1; m < q.nopt; m++)
                                if (t1_streq(q.opt[j], q.opt[m]))
                                    ok = 0;
                        if (q.nopt < 2 || q.correct >= q.nopt || !t1_streq(q.opt[q.correct], q.ans))
                            ok = 0;
                    } else {                     /* the pitch classes are the spelled notes', and read well */
                        for (j = 0; j < q.n; j++)
                            if (q.pc[j] != pc_of(q.nt[j]))
                                ok = 0;
                        if (q.n < 1 || awkward(q.nt, q.n, lv))
                            ok = 0;
                    }
                    if (!ok) {
                        bad++;
                        fprintf(stderr, "QUESTION BAD (type %d level %d): %s / %s\n", t, lv, q.l1, q.l2);
                    }
                }
        cfg.level = (uint8_t)lv0;
        q.done = 0;
        cut_count += bad;
        printf("  qcheck: %d questions, %d bad\n", n * 3 * (NQT - 1), bad);
        return 1;
    }
    if (!strcmp(cmd, "seed")) {
        t1_rng = (uint32_t)atoi(a) | 1u;
        return 1;
    }
    if (!strcmp(cmd, "quizof")) {
        ui.qsel = (uint8_t)atoi(a);
        return 1;
    }
    if (!strcmp(cmd, "lang")) {
        cfg.ui = (uint8_t)(atoi(a) % NLANGS);
        settings_apply();
        ui.redraw = 1;
        return 1;
    }
    if (!strcmp(cmd, "names")) {
        cfg.names = (uint8_t)(atoi(a) % NNAMES);
        settings_apply();
        ui.redraw = 1;
        return 1;
    }
    return 0;
}
#endif
