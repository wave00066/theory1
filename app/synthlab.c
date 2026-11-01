/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* The SYNTH section: how a synthesizer works, in chapters (synth_lessons.h, from app/tools/gen_synth.py). The keys
 * play a teaching synthesizer (synth.c's patch engine); K1..K4 move its knobs; each page draws what they do. A page
 * is read (T), tried (P) or a step with a goal on one knob (S). Part of app.c's unity: its helpers are in scope. */

static struct {
    uint8_t sel, ch, page;               /* the chapter chosen in the list; the chapter and page shown */
    uint8_t hsel;                        /* additive: the harmonic K1 picked, 0..7 */
    int8_t last_k;                       /* the knob moved last (the block diagram lights its block) */
    uint8_t done;                        /* a step's goal reached */
    uint8_t note;                        /* the last note played (the pictures' frequency) */
    int16_t p[NPAR];                     /* the patch */
    uint32_t anim;                       /* the next redraw of a moving picture */
} syn = {0, 0, 0, 0, -1, 0, 57, {0}, 0};

static int syn_is(int screen) { return screen == SCR_SYNLIST || screen == SCR_SYNPAGE; }
static const spage_t *syn_pg(void) { return &SPAGES[SCHAPS[syn.ch].first + syn.page]; }
static const char *SW(int i) { return SSTR[i][th_ui]; }

/* ------------------------------------------------------------ the patch --- */
static void syn_range(int p, int *lo, int *hi)
{
    *lo = 0;
    *hi = 100;
    switch (p) {
    case P_ENGINE: *hi = NENGINES - 1; break;
    case P_WAVE: *hi = NWAVES - 1; break;
    case P_WAVE2: *hi = W_PULSE; break;
    case P_PW: *lo = 5, *hi = 95; break;
    case P_OCT: *lo = -2, *hi = 1; break;
    case P_DETUNE: *hi = 50; break;
    case P_FTYPE: *hi = 2; break;
    case P_FAMT: *lo = -100; break;
    case P_PAMT: *hi = 48; break;
    case P_LDEST: *hi = NLDEST - 1; break;
    case P_LSHAPE: *hi = 4; break;
    case P_RATIO: *hi = NRATIO - 1; break;
    case P_POLY: *hi = 1; break;
    case V_HSEL: *hi = 7; break;
    default: break;
    }
}

static void syn_set(int p, int v)
{
    int lo, hi;
    if (p == V_HSEL) {
        syn_range(p, &lo, &hi);
        syn.hsel = (uint8_t)clamp(v, lo, hi);
        return;
    }
    if (p == V_HLVL)
        p = P_H1 + syn.hsel;
    if (p < 0 || p >= NPAR)
        return;
    syn_range(p, &lo, &hi);
    syn.p[p] = (int16_t)clamp(v, lo, hi);
    synth_set(p, syn.p[p]);
}
static int syn_get(int p)
{
    if (p == V_HSEL)
        return syn.hsel;
    if (p == V_HLVL)
        return syn.p[P_H1 + syn.hsel];
    return p >= 0 && p < NPAR ? syn.p[p] : 0;
}
static void syn_send(void)
{
    int i;
    for (i = 0; i < NPAR; i++)
        synth_set(i, syn.p[i]);
}

static int syn_goal_met(void)
{
    const spage_t *g = syn_pg();
    int v;
    if (g->kind != 2)
        return 0;
    v = syn_get(g->gp);
    return g->gop == 1 ? v >= g->gv : g->gop == 2 ? v <= g->gv : v >= g->gv - g->gtol && v <= g->gv + g->gtol;
}

/* a page shown: what it sets */
static void syn_page_enter(void)
{
    const spage_t *g = syn_pg();
    int i;
    for (i = 0; i < g->nset; i++)
        syn_set(g->sp[i], g->sv[i]);
    if (g->kind == 2 && (g->gp == P_H1 + 2))   /* the additive step: K1 starts on harmonic 1 */
        syn.hsel = 0;
    syn.done = (uint8_t)syn_goal_met();
    syn.last_k = -1;
    ui.redraw = 1;
}

static void syn_chapter(int c)
{
    int i;
    syn.ch = (uint8_t)c;
    syn.page = 0;
    if (!SCHAPS[c].keep)
        for (i = 0; i < NPAR; i++)
            syn.p[i] = SCHAPS[c].patch[i];
    syn_send();
    syn_page_enter();
}

/* ------------------------------------------------------------- the words --- */
static void cat_dec(char *b, int tenths)          /* 12 -> "1,2" (a dot in English) */
{
    cat_int(b, tenths / 10);
    if (tenths % 10) {
        str_cat(b, th_ui == L_EN ? "." : ",");
        cat_int(b, tenths % 10);
    }
}
static void syn_time(char *b, float ms)
{
    if (ms < 1000.0f) {
        cat_int(b, (int)(ms + 0.5f));
        str_cat(b, " ms");
    } else {
        cat_dec(b, (int)(ms / 100.0f + 0.5f));
        str_cat(b, " s");
    }
}
static const char *syn_value(int p, char *b)
{
    int v = syn_get(p);
    b[0] = 0;
    switch (p) {
    case P_ENGINE: return SENUM_ENGINE[v][th_ui];
    case P_WAVE: case P_WAVE2: return SENUM_WAVE[v][th_ui];
    case P_FTYPE: return SENUM_FTYPE[v][th_ui];
    case P_LDEST: return SENUM_LDEST[v][th_ui];
    case P_LSHAPE: return SENUM_LSHAPE[v][th_ui];
    case P_POLY: return SENUM_POLY[v][th_ui];
    case P_OCT:
        if (v > 0)
            str_cat(b, "+");
        cat_int(b, v);
        return b;
    case P_DETUNE: cat_int(b, v); str_cat(b, " ct"); return b;
    case P_PAMT: cat_int(b, v); str_cat(b, " st"); return b;
    case P_FAMT:
        if (v > 0)
            str_cat(b, "+");
        cat_int(b, v);
        str_cat(b, "%");
        return b;
    case P_RATIO: {
        static const uint8_t R10[NRATIO] = {5, 10, 14, 20, 30, 35, 40, 50, 70, 140};
        str_cat(b, "1:");
        if (v == 2) {                         /* 1:1,41 */
            str_cat(b, "1");
            str_cat(b, th_ui == L_EN ? ".41" : ",41");
        } else
            cat_dec(b, R10[v]);
        return b;
    }
    case P_LRATE: {
        float hz = syn_lfo_hz(v);
        if (hz < 0.95f) {                     /* 0,15 Hz */
            int c = (int)(hz * 100.0f + 0.5f);
            str_cat(b, "0");
            str_cat(b, th_ui == L_EN ? "." : ",");
            if (c < 10)
                str_cat(b, "0");
            cat_int(b, c);
        } else if (hz < 10.0f)
            cat_dec(b, (int)(hz * 10.0f + 0.5f));
        else
            cat_int(b, (int)(hz + 0.5f));
        str_cat(b, " Hz");
        return b;
    }
    case P_GLIDE:
        if (!v)
            return SW(SS_OFF);
        syn_time(b, syn_ms(v));
        return b;
    case P_MDEC:
        if (v >= 100)
            return SW(SS_HOLD);
        syn_time(b, syn_ms(v));
        return b;
    case P_A: case P_D: case P_R: case P_FA: case P_FD: case P_FR: case P_PDEC:
        syn_time(b, v ? syn_ms(v) : 0.0f);
        return b;
    case V_HSEL: cat_int(b, v + 1); return b;
    default: cat_int(b, v); str_cat(b, "%"); return b;
    }
}

/* ---------------------------------------------------------- the pictures --- */
#define C_ORANGE COL_ROOT
static void box(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t line, uint16_t fill)
{
    cv_rect(x, y, w, h, fill);
    cv_rect(x, y, w, 1, line);
    cv_rect(x, y + h - 1, w, 1, line);
    cv_rect(x, y, 1, h, line);
    cv_rect(x + w - 1, y, 1, h, line);
}
static void box_t(int32_t x, int32_t y, int32_t w, int32_t h, const char *s, uint16_t line, uint16_t fill, uint16_t tc)
{
    const felucca_font_t *f = t1_tw(&FONT_XS, s) <= w - 2 ? &FONT_XS : &FONT_T;
    box(x, y, w, h, line, fill);
    t1_tx(x + (w - t1_tw(f, s)) / 2, y + (h - (int32_t)f->h) / 2, f, s, tc);
}
static void dash_v(int32_t x, int32_t y0, int32_t y1, uint16_t c)
{
    int32_t y;
    for (y = y0; y < y1; y += 4)
        cv_rect(x, y, 1, 2, c);
}
static void dash_h(int32_t x0, int32_t x1, int32_t y, uint16_t c)
{
    int32_t x;
    for (x = x0; x < x1; x += 4)
        cv_rect(x, y, 2, 1, c);
}
static void arrow_r(int32_t x0, int32_t x1, int32_t y, uint16_t c)
{
    cv_rect(x0, y, x1 - x0, 1, c);
    cv_line(x1 - 4, y - 3, x1, y, c);
    cv_line(x1 - 4, y + 3, x1, y, c);
}
static void arrow_v(int32_t x, int32_t y0, int32_t y1, uint16_t c, int dashed)   /* towards y1 */
{
    int d = y1 > y0 ? 1 : -1;
    if (dashed)
        dash_v(x, y0 < y1 ? y0 : y1, y0 < y1 ? y1 : y0, c);
    else
        cv_rect(x, y0 < y1 ? y0 : y1, 1, (y0 < y1 ? y1 - y0 : y0 - y1), c);
    cv_line(x - 3, y1 - 4 * d, x, y1, c);
    cv_line(x + 3, y1 - 4 * d, x, y1, c);
}
static void small_c(int32_t cx, int32_t y, const char *s, uint16_t c) { t1_tx(cx - t1_tw(&FONT_XS, s) / 2, y, &FONT_XS, s, c); }

/* a pseudo-random value for t (the noise as drawn: the same every time) */
static float syn_hash(float t)
{
    uint32_t h = (uint32_t)(t * 9973.0f) * 2654435761u;
    h ^= h >> 15;
    return (float)(h & 0xFFFFu) / 32768.0f - 1.0f;
}

/* the patch's sound for one cycle, as drawn (no filter): t 0..1 */
static float syn_view(float t)
{
    const float pw = (float)syn.p[P_PW] * 0.01f;
    float y;
    switch (syn.p[P_ENGINE]) {
    case E_FM:
        y = fm_sin_turns(t + syn_index(syn.p[P_INDEX]) * 0.159155f *
                                 fm_sin_turns(t * SYN_RATIO[syn.p[P_RATIO] % NRATIO]));
        break;
    case E_ADD: {
        float sum = 0.0f;
        int h;
        y = 0.0f;
        for (h = 0; h < 8; h++) {
            float a = (float)syn.p[P_H1 + h] * 0.01f;
            y += a * fm_sin_turns(t * (float)(h + 1) - fm_floorf(t * (float)(h + 1)));
            sum += a;
        }
        y = sum > 0.0f ? y / (sum > 1.0f ? sum : 1.0f) : 0.0f;
        break;
    }
    case E_WT: y = syn_wt((float)syn.p[P_WTPOS], t); break;
    default:
        y = syn.p[P_WAVE] == W_NOISE ? syn_hash(t) : syn_wave(syn.p[P_WAVE], t, pw);
        if (syn.p[P_OSC2] && syn.p[P_WAVE] != W_NOISE) {
            float o2 = (float)syn.p[P_OSC2] * 0.01f;
            y = (y + o2 * syn_wave(syn.p[P_WAVE2], t, pw)) / (1.0f + o2);
        }
        break;
    }
    if (syn.p[P_NOISE]) {
        float n = (float)syn.p[P_NOISE] * 0.01f;
        y = y * (1.0f - n) + syn_hash(t) * n;
    }
    return y;
}

/* cycles of the patch's wave across x0..x1, amplitude a pixels around y */
static void wave_draw(int32_t x0, int32_t x1, int32_t y, int32_t a, float cycles, uint16_t c)
{
    int32_t x, py = 0;
    for (x = x0; x <= x1; x++) {
        float t = (float)(x - x0) / (float)(x1 - x0) * cycles;
        int32_t yy = y - (int32_t)(syn_view(t - fm_floorf(t)) * (float)a);
        if (x > x0)
            cv_line(x - 1, py, x, yy, c);
        py = yy;
    }
}

/* the first ten harmonics of the wave as drawn (a 64-point DFT), 0..1 */
static void harmonics(float *h)
{
    float x[64];
    int n, k;
    float mx = 0.0f;
    for (n = 0; n < 64; n++)
        x[n] = syn_view((float)n / 64.0f);
    for (k = 1; k <= 10; k++) {
        float re = 0.0f, im = 0.0f;
        for (n = 0; n < 64; n++) {
            float ph = (float)((k * n) & 63) / 64.0f;
            re += x[n] * fm_sin_turns(ph + 0.25f);
            im += x[n] * fm_sin_turns(ph);
        }
        h[k - 1] = fm_sqrtf(re * re + im * im) / 32.0f;
        if (h[k - 1] > mx)
            mx = h[k - 1];
    }
    for (k = 0; k < 10; k++)
        h[k] = mx > 0.0f ? h[k] / mx : 0.0f;
}
static void spectrum_draw(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    float h[10];
    int k;
    int32_t bw = (x1 - x0) / 10;
    harmonics(h);
    cv_rect(x0, y1, x1 - x0, 1, C_LINE);
    for (k = 0; k < 10; k++) {
        int32_t bh = (int32_t)(h[k] * (float)(y1 - y0 - 12));
        if (syn.p[P_ENGINE] == E_SUB && syn.p[P_WAVE] == W_NOISE)
            bh = (int32_t)((0.5f + 0.3f * syn_hash((float)k * 0.37f)) * (float)(y1 - y0 - 12));
        cv_rect(x0 + k * bw + 2, y1 - bh, bw - 4, bh, k ? C_HI : C_ORANGE);
    }
    small_c(x0 + bw / 2, y0, "1", C_ORANGE);
    t1_tx(x0 + bw + 2, y0, &FONT_XS, SW(SS_HARMS), C_GRAY);
}

static float note_hz(void) { return 440.0f * fm_exp2f(((float)syn.note + 12.0f * (float)syn.p[P_OCT] - 69.0f) / 12.0f); }

static void d_sound(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t ym = (y0 + y1) / 2 + 4, a = (int32_t)((float)((y1 - y0) / 2 - 8) * (float)syn.p[P_VOL] * 0.01f);
    int32_t xm = x0 + (x1 - x0 - 50) / 2;
    char b[24];
    cv_rect(x0, ym, x1 - x0 - 50, 1, C_LINE);
    wave_draw(x0, x1 - 50, ym, a, 2.0f, C_WHITE);
    /* the period over the first cycle, the amplitude at its peak */
    cv_rect(x0, y0 + 4, xm - x0, 1, C_ORANGE);
    cv_rect(x0, y0 + 1, 1, 7, C_ORANGE);
    cv_rect(xm, y0 + 1, 1, 7, C_ORANGE);
    small_c((x0 + xm) / 2, y0 + 6, SW(SS_PERIOD), C_ORANGE);
    dash_v(x0 + (xm - x0) / 4, ym - a, ym, COL_SCALE);
    t1_tx(x0 + (xm - x0) / 4 + 3, ym + 2, &FONT_XS, SW(SS_AMPL), COL_SCALE);
    b[0] = 0;
    {
        float hz = note_hz();
        if (hz < 100.0f)
            cat_dec(b, (int)(hz * 10.0f + 0.5f));
        else
            cat_int(b, (int)(hz + 0.5f));
    }
    str_cat(b, " Hz");
    text_r(x1, ym - 18, &FONT_B, b, C_WHITE);
    text_r(x1, ym + 2, &FONT_XS, SENUM_WAVE[syn.p[P_WAVE]][th_ui], C_GRAY);
}

static void d_wave(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t xm = (x0 + x1) / 2, ym = (y0 + y1) / 2;
    cv_rect(x0, ym, xm - 6 - x0, 1, C_LINE);
    wave_draw(x0, xm - 6, ym, (y1 - y0) / 2 - 4, 2.0f, C_WHITE);
    spectrum_draw(xm + 2, y0, x1, y1);
}

static void d_blocks(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int ctl)
{
    const int32_t bw = 48, bh = 18, gap = (x1 - x0 - 4 * bw) / 3;
    int32_t ym = (y0 + y1) / 2 - bh / 2, xs[4], i;
    const int names[4] = {SS_OSC, SS_FILTER, SS_AMP, SS_OUT};
    const int lit = syn.last_k < 0 ? -1 : syn_pg()->knob[syn.last_k] == P_WAVE ? 0 :
                    syn_pg()->knob[syn.last_k] == P_CUTOFF ? 1 : syn_pg()->knob[syn.last_k] == P_VOL ? 2 : -1;
    uint16_t pc = ctl ? C_GRAY : C_WHITE, mc = ctl ? C_ORANGE : C_GRAY;
    for (i = 0; i < 4; i++) {
        xs[i] = x0 + i * (bw + gap);
        box_t(xs[i], ym, bw, bh, SW(names[i]), i == 3 ? C_HI : pc, i == lit ? COL_SELBG : i == 3 ? COL_PANEL : C_BLACK,
              pc);
        if (i)
            arrow_r(xs[i - 1] + bw, xs[i] - 1, ym + bh / 2, pc);
    }
    /* the controls: the LFO above the oscillator, the keys and two envelopes below */
    box_t(xs[0], y0, bw, 14, SW(SS_LFO), mc, C_BLACK, mc);
    arrow_v(xs[0] + bw / 2, y0 + 14, ym - 1, mc, 1);
    box_t(xs[0], y1 - 14, bw, 14, SW(SS_KEYS), mc, C_BLACK, mc);
    arrow_v(xs[0] + bw / 2, y1 - 14, ym + bh + 1, mc, 1);
    for (i = 1; i < 3; i++) {
        box_t(xs[i], y1 - 14, bw, 14, SW(SS_ENV), mc, C_BLACK, mc);
        arrow_v(xs[i] + bw / 2, y1 - 14, ym + bh + 1, mc, 1);
    }
}

/* the filter's response in dB at f (Hz) */
static float filt_db(float f, float fc, float q, int type)
{
    float w = f / fc, d = (1.0f - w * w) * (1.0f - w * w) + (w / q) * (w / q), m;
    d = fm_sqrtf(d);
    m = type == 0 ? 1.0f / d : type == 1 ? 1.5f * (w / q) / d : w * w / d;
    return 6.0206f * fm_log2f(fm_maxf(m, 1e-4f));
}
static void d_filter(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const float fc = syn_cutoff((float)syn.p[P_CUTOFF]), q = syn_q(syn.p[P_RESO]), f0 = note_hz();
    const int type = syn.p[P_FTYPE];
    const int32_t yb = y1 - 10, y0db = y0 + 10 + (yb - y0 - 10) / 4;    /* +12 .. -36 dB */
    const float dbpx = (float)(yb - y0 - 10) / 48.0f;
    int32_t x, py = 0, xc;
    int n;
    char b[16];
    cv_rect(x0, y0db, x1 - x0, 1, C_LINE);
    cv_rect(x0, yb, x1 - x0, 1, C_LINE);
    /* a saw's harmonics on the note, as the filter leaves them */
    for (n = 1; n < 80; n++) {
        float f = f0 * (float)n, db;
        int32_t xx, h;
        if (f > 18000.0f)
            break;
        xx = x0 + (int32_t)(fm_log2f(f / 30.0f) / 9.4f * (float)(x1 - x0));
        db = filt_db(f, fc, q, type) - 6.0206f * fm_log2f((float)n);
        h = (int32_t)((db + 36.0f) * dbpx);
        if (h > 0)
            cv_rect(xx, yb - h, 1, h, n == 1 ? C_ORANGE : COL_SCALE);
    }
    for (x = x0; x <= x1; x++) {
        float f = 30.0f * fm_exp2f((float)(x - x0) / (float)(x1 - x0) * 9.4f);
        float db = fm_clampf(filt_db(f, fc, q, type), -36.0f, 12.0f);
        int32_t yy = y0db - (int32_t)(db * dbpx);
        if (x > x0)
            cv_line(x - 1, py, x, yy, C_WHITE);
        py = yy;
    }
    xc = x0 + (int32_t)(fm_log2f(fc / 30.0f) / 9.4f * (float)(x1 - x0));
    dash_v(xc, y0 + 10, yb, C_ORANGE);
    b[0] = 0;
    if (fc >= 1000.0f) {
        cat_dec(b, (int)(fc / 100.0f + 0.5f));
        str_cat(b, " kHz");
    } else {
        cat_int(b, (int)(fc + 0.5f));
        str_cat(b, " Hz");
    }
    t1_tx(xc + 3 > x1 - 50 ? xc - 3 - t1_tw(&FONT_XS, b) : xc + 3, y0, &FONT_XS, b, C_ORANGE);
    t1_tx(x0, y0, &FONT_XS, SENUM_FTYPE[type][th_ui], C_WHITE);
    t1_tx(x0, yb + 1, &FONT_T, "30 HZ", C_DIM);
    text_r(x1, yb + 1, &FONT_T, "20 KHZ", C_DIM);
}

/* an ADSR as drawn: its points from the four knobs, time on a log scale so short stages stay visible */
static int32_t seg_w(int knob) { return knob ? 4 + knob * 40 / 100 : 2; }
static void adsr_draw(int32_t x0, int32_t y0, int32_t y1, int a, int d, int s, int r, uint16_t c, int labels,
                      int32_t plateau)
{
    int32_t xa = x0 + seg_w(a), xd = xa + seg_w(d), xs = xd + plateau, xr = xs + seg_w(r);
    int32_t ys = y1 - (y1 - y0) * s / 100, i, py = y0;
    cv_line(x0, y1, xa, y0, c);
    for (i = 1; i <= xd - xa; i++) {           /* the decay falls exponentially */
        float k = fm_expf(-5.0f * (float)i / (float)(xd - xa));
        int32_t yy = ys - (int32_t)((float)(ys - y0) * k);
        cv_line(xa + i - 1, py, xa + i, yy, c);
        py = yy;
    }
    cv_line(xd, ys, xs, ys, c);
    py = ys;
    for (i = 1; i <= xr - xs; i++) {
        float k = fm_expf(-5.0f * (float)i / (float)(xr - xs));
        int32_t yy = y1 - (int32_t)((float)(y1 - ys) * k);
        cv_line(xs + i - 1, py, xs + i, yy, c);
        py = yy;
    }
    if (labels) {
        dash_h(xd, xs, ys - 0, C_ORANGE);
        small_c((x0 + xa) / 2, y0 - 13, "A", C_HI);
        small_c((xa + xd) / 2, y0 - 13, "D", C_HI);
        small_c((xd + xs) / 2, y0 - 13, "S", C_ORANGE);
        small_c((xs + xr) / 2, y0 - 13, "R", C_HI);
        cv_rect(x0, y1 + 3, xs - x0, 3, C_GRAY);
        t1_tx(x0, y1 + 6, &FONT_XS, SW(SS_KEYDOWN), C_GRAY);
    }
}
static void d_adsr(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    (void)x1;
    cv_rect(x0, y1 - 18, x1 - x0, 1, C_LINE);
    adsr_draw(x0, y0 + 14, y1 - 18, syn.p[P_A], syn.p[P_D], syn.p[P_S], syn.p[P_R], C_WHITE, 1, 40);
}
static void d_adsr4(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    static const uint8_t SH4[4][4] = {{0, 0, 100, 5}, {0, 79, 0, 62}, {68, 75, 80, 70}, {0, 57, 0, 57}};
    const int names[4] = {SS_ORGAN, SS_PIANO, SS_STRINGS, SS_DRUM};
    int32_t w = (x1 - x0) / 4, i;
    for (i = 0; i < 4; i++) {
        int32_t x = x0 + i * w;
        cv_rect(x + 2, y1 - 16, w - 6, 1, C_LINE);
        adsr_draw(x + 2, y0 + 4, y1 - 16, SH4[i][0] / 2, SH4[i][1] / 2, SH4[i][2], SH4[i][3] / 2, C_WHITE, 0, 14);
        small_c(x + w / 2, y1 - 13, SW(names[i]), C_HI);
    }
}

static void d_fenv(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    /* the cutoff in octaves over 30 Hz (0 .. 9.4), the envelope added: the curve is what the filter does */
    const float c = (float)syn.p[P_CUTOFF] * 0.094f, amt = (float)syn.p[P_FAMT] * 0.07f;
    const int32_t top = y0 + 12, bot = y1 - 2, h = bot - top;
    const int32_t yc = bot - (int32_t)(c / 9.4f * (float)h);
    int32_t xa = x0 + seg_w(syn.p[P_FA]) * 2, xd = xa + seg_w(syn.p[P_FD]) * 2, xs = xd + 50,
            xr = xs + seg_w(syn.p[P_FR]) * 2, x, py = yc;
    const float s = (float)syn.p[P_FS] * 0.01f;
    t1_tx(x0, y0, &FONT_XS, "20 kHz", C_DIM);
    cv_rect(x0, top, x1 - x0, 1, C_LINE);
    dash_h(x0, x1, yc, COL_SCALE);
    text_r(x1, yc - 13 < top ? yc + 2 : yc - 13, &FONT_XS, "cutoff", COL_SCALE);
    for (x = x0; x <= x1; x++) {
        float e;
        int32_t yy;
        if (x <= xa)
            e = xa > x0 ? (float)(x - x0) / (float)(xa - x0) : 1.0f;
        else if (x <= xd)
            e = s + (1.0f - s) * fm_expf(-5.0f * (float)(x - xa) / (float)(xd - xa));
        else if (x <= xs)
            e = s;
        else
            e = s * fm_expf(-5.0f * (float)(x - xs) / (float)(xr - xs > 0 ? xr - xs : 1));
        yy = bot - (int32_t)(fm_clampf(c + e * amt, 0.0f, 9.4f) / 9.4f * (float)h);
        if (x > x0)
            cv_line(x - 1, py, x, yy, C_ORANGE);
        py = yy;
    }
}

static void d_penv(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t ym = (y0 + y1) / 2, w = x1 - x0;
    const float tau = syn_ms(syn.p[P_PDEC]) / 3.0f, amt = (float)syn.p[P_PAMT];
    const float dtau = syn_ms(syn.p[P_D]) / 5.0f, sus = (float)syn.p[P_S] * 0.01f;
    int32_t x, py = 0, pw = 0;
    float ph = 0.0f;
    t1_tx(x0, y0, &FONT_XS, SW(SS_PITCH), C_ORANGE);
    text_r(x1, ym - 12, &FONT_T, "0 - 300 MS", C_DIM);
    for (x = x0; x <= x1; x++) {
        float ms = (float)(x - x0) / (float)w * 300.0f, st = amt * fm_expf(-ms / tau);
        int32_t yy = ym - 2 - (int32_t)(st / 48.0f * (float)(ym - y0 - 6)), yw;
        float hz = 55.0f * fm_exp2f(st / 12.0f), a = sus + (1.0f - sus) * fm_expf(-ms / dtau);
        ph += hz * 300.0f / 1000.0f / (float)w;
        yw = (y1 + ym) / 2 + 2 - (int32_t)(fm_sin_turns(ph - fm_floorf(ph)) * a * (float)((y1 - ym) / 2 - 3));
        if (x > x0) {
            cv_line(x - 1, py, x, yy, C_ORANGE);
            cv_line(x - 1, pw, x, yw, C_WHITE);
        }
        py = yy;
        pw = yw;
    }
}

static float lfo_shape(int s, float t)
{
    switch (s) {
    case 0: return fm_sin_turns(t);
    case 1: return syn_wave(W_TRI, t, 0.5f);
    case 2: return 1.0f - 2.0f * t;
    case 3: return t < 0.5f ? 1.0f : -1.0f;
    default: return syn_hash((float)(int)(t * 4.0f) * 0.31f + 0.2f);   /* four steps a cycle, as drawn */
    }
}
static void lfo_draw(int32_t x0, int32_t x1, int32_t y, int32_t a, int shape, float cycles, uint16_t c)
{
    int32_t x, py = y;
    for (x = x0; x <= x1; x++) {
        float t = (float)(x - x0) / (float)(x1 - x0) * cycles;
        int32_t yy = y - (int32_t)(lfo_shape(shape, t - fm_floorf(t)) * (float)a);
        if (x > x0)
            cv_line(x - 1, py, x, yy, c);
        py = yy;
    }
}
static void d_lfo(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t ym = (y0 + y1) / 2 + 6, xr = x1 - 64;
    const float hz = syn_lfo_hz(syn.p[P_LRATE]);
    int32_t a = (int32_t)((float)(y1 - ym - 2) * fm_maxf((float)syn.p[P_LDEPTH] * 0.01f, 0.04f));
    float t;
    char b[16];
    cv_rect(x0, ym, xr - x0, 1, C_LINE);
    lfo_draw(x0, xr, ym, a, syn.p[P_LSHAPE], 2.0f, C_ORANGE);
    /* the dot: where the LFO is now */
    t = (float)(plat_ms() % 100000u) * 0.001f * hz;
    t = (t - fm_floorf(t)) * 0.5f + 0.5f * (float)((int)(plat_ms() * hz * 0.001f) & 1);
    {
        int32_t xx = x0 + (int32_t)(t * (float)(xr - x0));
        float u = t * 2.0f;
        int32_t yy = ym - (int32_t)(lfo_shape(syn.p[P_LSHAPE], u - fm_floorf(u)) * (float)a);
        cv_rect(xx - 2, yy - 2, 5, 5, C_WHITE);
    }
    t1_tx(x0, y0, &FONT_XS, "LFO", C_ORANGE);
    syn_value(P_LRATE, b);
    t1_tx(x0 + 28, y0, &FONT_XS, b, C_HI);
    /* where it goes */
    arrow_r(xr + 4, xr + 18, ym, C_ORANGE);
    box_t(xr + 20, ym - 9, 44, 18, SENUM_LDEST[syn.p[P_LDEST]][th_ui], C_WHITE, C_BLACK, C_WHITE);
}
static void d_lfo5(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    int32_t w = (x1 - x0) / 5, i;
    for (i = 0; i < 5; i++) {
        int32_t x = x0 + i * w;
        lfo_draw(x + 3, x + w - 5, (y0 + y1) / 2 - 6, (y1 - y0) / 2 - 14, i, 2.0f, i == syn.p[P_LSHAPE] ? C_ORANGE : C_WHITE);
        small_c(x + w / 2, y1 - 13, SENUM_LSHAPE[i][th_ui], C_HI);
    }
}

/* Bessel's J_n(x), by its series: FM's sidebands */
static float bessel(int n, float x)
{
    float h = x * 0.5f, term = 1.0f, sum;
    int m;
    for (m = 1; m <= n; m++)
        term *= h / (float)m;
    sum = term;
    for (m = 0; m < 30; m++) {
        term *= -h * h / ((float)(m + 1) * (float)(m + 1 + n));
        sum += term;
    }
    return sum;
}
static void d_fm(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const float ratio = SYN_RATIO[syn.p[P_RATIO] % NRATIO], I = syn_index(syn.p[P_INDEX]);
    const int32_t bx = x0, bw = 66, sx = x0 + 84, yb = y1 - 12;
    float amp[24] = {0};
    int k;
    char b[16];
    /* the two operators */
    syn_value(P_RATIO, b);
    box_t(bx, y0, bw, 18, SW(SS_MODULATOR), C_ORANGE, C_BLACK, C_ORANGE);
    t1_tx(bx + bw + 2, y0 + 2, &FONT_XS, b, C_ORANGE);
    arrow_v(bx + bw / 2, y0 + 18, (y0 + y1) / 2 - 8, C_ORANGE, 0);
    box_t(bx, (y0 + y1) / 2 - 8, bw, 18, SW(SS_CARRIER), C_WHITE, C_BLACK, C_WHITE);
    arrow_v(bx + bw / 2, (y0 + y1) / 2 + 10, y1 - 2, C_WHITE, 0);
    if (syn.p[P_FB]) {                           /* the feedback: a loop on the modulator */
        cv_rect(bx + bw, y0 + 13, 6, 1, C_ORANGE);
        cv_rect(bx + bw + 5, y0 - 2 < 0 ? 0 : y0, 1, 14, C_ORANGE);
    }
    /* the sidebands at 1 + k ratio (folded), 0 .. 10 times the carrier */
    for (k = -12; k <= 12; k++) {
        float f = 1.0f + (float)k * ratio, a = fm_fabsf(bessel(k < 0 ? -k : k, I));
        int idx;
        if (f < 0.0f)
            f = -f;
        idx = (int)(f * 2.0f + 0.5f);              /* half steps */
        if (idx < 24)
            amp[idx] += a;
    }
    cv_rect(sx, yb, x1 - sx, 1, C_LINE);
    for (k = 0; k < 21; k++)
        if (amp[k] > 0.02f) {
            int32_t h = (int32_t)(fm_minf(amp[k], 1.0f) * (float)(yb - y0 - 4));
            int32_t xx = sx + k * (x1 - sx - 4) / 20;
            cv_rect(xx, yb - h, 3, h, k == 2 ? C_ORANGE : C_HI);
        }
    t1_tx(sx, yb + 1, &FONT_T, "0", C_DIM);
    t1_tx(sx + 2 * (x1 - sx - 4) / 20 - 2, yb + 1, &FONT_T, "1", C_ORANGE);
    text_r(x1, yb + 1, &FONT_T, "10", C_DIM);
}
static void d_fmalg(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t w = (x1 - x0) / 3, ob = 34;
    int32_t c;
    /* a stack */
    c = x0 + w / 2;
    box_t(c - ob / 2, y0, ob, 16, "OP 2", C_ORANGE, C_BLACK, C_ORANGE);
    arrow_v(c, y0 + 16, y0 + 32, C_ORANGE, 0);
    box_t(c - ob / 2, y0 + 33, ob, 16, "OP 1", C_WHITE, C_BLACK, C_WHITE);
    arrow_v(c, y0 + 49, y0 + 60, C_WHITE, 0);
    small_c(c, y1 - 13, SW(SS_STACK), C_HI);
    /* in parallel */
    c = x0 + w + w / 2;
    box_t(c - ob - 2, y0 + 33, ob, 16, "OP 1", C_WHITE, C_BLACK, C_WHITE);
    box_t(c + 2, y0 + 33, ob, 16, "OP 2", C_WHITE, C_BLACK, C_WHITE);
    cv_rect(c - ob / 2 - 2, y0 + 49, ob + 4, 1, C_WHITE);
    arrow_v(c, y0 + 50, y0 + 60, C_WHITE, 0);
    small_c(c, y1 - 13, SW(SS_PARALLEL), C_HI);
    /* with feedback */
    c = x0 + 2 * w + w / 2;
    box_t(c - ob / 2, y0, ob, 16, "OP 2", C_ORANGE, C_BLACK, C_ORANGE);
    cv_rect(c + ob / 2, y0 + 8, 6, 1, C_ORANGE);
    cv_rect(c + ob / 2 + 5, y0 - 4 < 0 ? 0 : y0 - 4, 1, 12, C_ORANGE);
    cv_rect(c, y0 - 4 < 0 ? 0 : y0 - 4, ob / 2 + 6, 1, C_ORANGE);
    arrow_v(c, y0 + 16, y0 + 32, C_ORANGE, 0);
    box_t(c - ob / 2, y0 + 33, ob, 16, "OP 1", C_WHITE, C_BLACK, C_WHITE);
    arrow_v(c, y0 + 49, y0 + 60, C_WHITE, 0);
    small_c(c, y1 - 13, SW(SS_FEEDBACK), C_HI);
}

static void d_add(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t bw = 12, yb = y1 - 13, xm = x0 + 8 * (bw + 3) + 8;
    int h;
    for (h = 0; h < 8; h++) {
        int32_t x = x0 + h * (bw + 3), bh = (int32_t)((float)syn.p[P_H1 + h] * 0.01f * (float)(yb - y0 - 2));
        char n[4];
        cv_rect(x, y0, bw, yb - y0, COL_PANEL);
        cv_rect(x, yb - bh, bw, bh, h ? C_HI : C_ORANGE);
        if (h == syn.hsel && syn_pg()->knob[0] == V_HSEL) {
            cv_rect(x - 1, y0 - 1, bw + 2, 1, C_ORANGE);
            cv_rect(x - 1, yb, bw + 2, 1, C_ORANGE);
        }
        n[0] = (char)('1' + h);
        n[1] = 0;
        small_c(x + bw / 2, yb + 1, n, h == syn.hsel ? C_ORANGE : C_GRAY);
    }
    cv_rect(xm, (y0 + yb) / 2, x1 - xm, 1, C_LINE);
    wave_draw(xm, x1, (y0 + yb) / 2, (yb - y0) / 2 - 2, 1.0f, C_WHITE);
    small_c((xm + x1) / 2, yb + 1, SW(SS_SUM), C_GRAY);
}

static void d_wt(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t fw = 84, fh = 22;
    float pos = (float)syn.p[P_WTPOS];
    int f, x;
    if (syn.p[P_LDEST] == L_WT && syn.p[P_LDEPTH])      /* the LFO moving it: where it is now */
        pos = fm_clampf(pos + synth_lfo() * (float)syn.p[P_LDEPTH] * 0.5f, 0.0f, 100.0f);
    for (f = 0; f < 5; f++) {
        int32_t ox = x0 + f * 18, oy = y1 - 14 - fh / 2 - f * 9, py = oy;
        for (x = 0; x <= fw; x++) {
            int32_t yy = oy - (int32_t)(syn_wt((float)f * 25.0f, (float)x / (float)fw) * (float)(fh / 2));
            if (x)
                cv_line(ox + x - 1, py, ox + x, yy, C_GRAY);
            py = yy;
        }
    }
    {
        float fr = pos / 25.0f;
        int32_t ox = x0 + (int32_t)(fr * 18.0f), oy = y1 - 14 - fh / 2 - (int32_t)(fr * 9.0f), py = oy;
        for (x = 0; x <= fw; x++) {
            int32_t yy = oy - (int32_t)(syn_wt(pos, (float)x / (float)fw) * (float)(fh / 2));
            if (x)
                cv_line(ox + x - 1, py, ox + x, yy, C_ORANGE);
            py = yy;
        }
    }
    arrow_r(x0, x0 + 4 * 18 + 30, y1 - 4, C_GRAY);
    t1_tx(x0 + 4 * 18 + 34, y1 - 12, &FONT_XS, SW(SS_POSITION), C_GRAY);
    (void)x1;
}

static void d_voices(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t xm = (x0 + x1) / 2, lh = (y1 - y0 - 16) / 4;
    const char *nn[4] = {"C", "E", "G", "B"};
    int i;
    uint16_t mc = syn.p[P_POLY] ? C_ORANGE : C_GRAY, pc = syn.p[P_POLY] ? C_GRAY : C_ORANGE;
    t1_tx(x0, y0, &FONT_XS, "Mono", mc);
    t1_tx(xm + 4, y0, &FONT_XS, "Poly", pc);
    for (i = 0; i < 4; i++) {
        int32_t y = y0 + 15 + i * lh;
        cv_rect(x0, y + lh - 2, xm - x0 - 6, 1, C_LINE);
        cv_rect(xm + 4, y + lh - 2, x1 - xm - 4, 1, C_LINE);
        if (i < 3) {                                  /* mono: the notes one after the other, on one voice */
            int32_t w = (xm - x0 - 6) / 3;
            cv_rect(x0 + i * w, y0 + 15, w - 2, lh - 4, mc);
            t1_tx(x0 + i * w + 2, y0 + 15, &FONT_T, nn[i], C_BLACK);
        }
        cv_rect(xm + 4 + i * 12, y, x1 - xm - 4 - i * 12, lh - 4, pc);   /* poly: each its own voice */
        t1_tx(xm + 6 + i * 12, y, &FONT_T, nn[i], C_BLACK);
    }
}
static void d_glide(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t xm = (x0 + x1) / 2, yl = y1 - 18, yh = y0 + 6;
    const int on = syn.p[P_GLIDE] > 0;
    uint16_t c0 = on ? C_GRAY : C_ORANGE, c1 = on ? C_ORANGE : C_GRAY;
    int32_t x, py = yl, w = (int32_t)(fm_minf(syn_ms(syn.p[P_GLIDE]) / 400.0f, 1.0f) * (float)(xm - x0 - 30)) + 4;
    cv_rect(x0, yl, (xm - x0) / 2 - 6, 1, c0);
    cv_rect(x0 + (xm - x0) / 2 - 6, yh, 1, yl - yh, c0);
    cv_rect(x0 + (xm - x0) / 2 - 6, yh, (xm - x0) / 2 - 2, 1, c0);
    small_c((x0 + xm) / 2, y1 - 13, SW(SS_NOGLIDE), c0);
    for (x = xm + 6; x <= x1; x++) {                 /* the pitch glides there */
        int32_t s = xm + 6 + (x1 - xm - 6) / 3, yy;
        if (x < s)
            yy = yl;
        else {
            float k = fm_minf((float)(x - s) / (float)w, 1.0f);
            k = k * k * (3.0f - 2.0f * k);
            yy = yl - (int32_t)(k * (float)(yl - yh));
        }
        if (x > xm + 6)
            cv_line(x - 1, py, x, yy, c1);
        py = yy;
    }
    small_c((xm + x1) / 2 + 3, y1 - 13, SW(SS_GLIDE), c1);
}

static void d_fx(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int names[5] = {SS_SYNTH, -1, -1, SS_DELAY, SS_REVERB};
    const int32_t bw = 40, gap = (x1 - x0 - 5 * bw) / 4;
    const float g = 1.0f + (float)syn.p[P_DRIVE] * 0.08f;
    int i;
    int32_t x, py = 0, cx0 = x0, cy = (y0 + 22 + y1) / 2 + 2, ch = (y1 - y0 - 26) / 2;
    for (i = 0; i < 5; i++) {
        int32_t bx = x0 + i * (bw + gap);
        const char *s = i == 1 ? "DRIVE" : i == 2 ? "CHORUS" : SW(names[i]);
        uint16_t c = i == 0 ? C_HI : i < 3 ? C_ORANGE : C_DIM;
        box_t(bx, y0, bw, 16, s, c, i == 0 ? COL_PANEL : C_BLACK, c);
        if (i)
            arrow_r(bx - gap, bx - 1, y0 + 8, i < 3 ? C_ORANGE : C_DIM);
    }
    /* the drive's curve: in -> out */
    cv_rect(cx0, cy, 2 * ch, 1, C_LINE);
    cv_rect(cx0 + ch, cy - ch, 1, 2 * ch, C_LINE);
    for (x = 0; x <= 2 * ch; x++) {
        float in = (float)(x - ch) / (float)ch;
        int32_t yy = cy - (int32_t)(fm_tanhf(in * g) / fm_tanhf(g) * (float)ch);
        if (x)
            cv_line(cx0 + x - 1, py, cx0 + x, yy, C_ORANGE);
        py = yy;
    }
    t1_tx(cx0 + 2 * ch + 4, cy - ch, &FONT_XS, "drive", C_ORANGE);
    /* the chorus: the sound and its wavering copies */
    {
        int32_t sx = x0 + 120, k;
        for (k = 0; k < 3; k++) {
            int32_t c = k == 0 ? (int32_t)C_WHITE : (int32_t)COL_SCALE;
            float sh = k == 0 ? 0.0f : (float)syn.p[P_CHORUS] * 0.0015f * (float)(k == 1 ? 1 : -1);
            py = cy;
            if (k && !syn.p[P_CHORUS])
                break;
            for (x = 0; x <= x1 - sx; x++) {
                float t = (float)x / (float)(x1 - sx) * 2.0f + sh;
                int32_t yy = cy - (int32_t)(fm_sin_turns(t - fm_floorf(t)) * (float)(ch - 2));
                if (x)
                    cv_line(sx + x - 1, py, sx + x, yy, (uint16_t)c);
                py = yy;
            }
        }
        text_r(x1, cy - ch, &FONT_XS, "chorus", COL_SCALE);
    }
}

static void d_patch(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    const int32_t w = (x1 - x0 - 9) / 4, h = y1 - y0 - 14;
    const int names[4] = {SS_OSC, SS_FILTER, SS_ENV, SS_AMP};
    int i;
    for (i = 0; i < 4; i++) {
        int32_t x = x0 + i * (w + 3);
        box(x, y0 + 14, w, h, C_LINE, COL_PANEL);
        small_c(x + w / 2, y0, SW(names[i]), i == 2 ? C_ORANGE : C_HI);
    }
    wave_draw(x0 + 3, x0 + w - 3, y0 + 14 + h / 2, h / 2 - 4, 2.0f, C_WHITE);
    {   /* the filter's curve, small */
        const float fc = syn_cutoff((float)syn.p[P_CUTOFF]), q = syn_q(syn.p[P_RESO]);
        int32_t x, py = 0, xs = x0 + w + 3;
        for (x = 2; x < w - 2; x++) {
            float f = 30.0f * fm_exp2f((float)x / (float)w * 9.4f);
            int32_t yy = y0 + 14 + h / 3 - (int32_t)(fm_clampf(filt_db(f, fc, q, syn.p[P_FTYPE]), -36.0f, 12.0f) *
                                                     (float)h / 60.0f);
            if (x > 2)
                cv_line(xs + x - 1, py, xs + x, yy, C_WHITE);
            py = yy;
        }
    }
    adsr_draw(x0 + 2 * (w + 3) + 3, y0 + 20, y1 - 4, syn.p[P_FA] / 2, syn.p[P_FD] / 2, syn.p[P_FS], syn.p[P_FR] / 2,
              syn.p[P_FAMT] ? C_ORANGE : C_DIM, 0, 8);
    adsr_draw(x0 + 3 * (w + 3) + 3, y0 + 20, y1 - 4, syn.p[P_A] / 2, syn.p[P_D] / 2, syn.p[P_S], syn.p[P_R] / 2,
              C_WHITE, 0, 8);
}

static void syn_diag(int d, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    switch (d) {
    case SD_SOUND: d_sound(x0, y0, x1, y1); break;
    case SD_WAVE: d_wave(x0, y0, x1, y1); break;
    case SD_BLOCKS: d_blocks(x0, y0, x1, y1, 0); break;
    case SD_BLOCKS2: d_blocks(x0, y0, x1, y1, 1); break;
    case SD_FILTER: d_filter(x0, y0, x1, y1); break;
    case SD_ADSR: d_adsr(x0, y0, x1, y1); break;
    case SD_ADSR4: d_adsr4(x0, y0, x1, y1); break;
    case SD_FENV: d_fenv(x0, y0, x1, y1); break;
    case SD_PENV: d_penv(x0, y0, x1, y1); break;
    case SD_LFO: d_lfo(x0, y0, x1, y1); break;
    case SD_LFO5: d_lfo5(x0, y0, x1, y1); break;
    case SD_FM: d_fm(x0, y0, x1, y1); break;
    case SD_FMALG: d_fmalg(x0, y0, x1, y1); break;
    case SD_ADD: d_add(x0, y0, x1, y1); break;
    case SD_WT: d_wt(x0, y0, x1, y1); break;
    case SD_VOICES: d_voices(x0, y0, x1, y1); break;
    case SD_GLIDE: d_glide(x0, y0, x1, y1); break;
    case SD_FX: d_fx(x0, y0, x1, y1); break;
    case SD_PATCH: d_patch(x0, y0, x1, y1); break;
    default: break;
    }
}
/* the pictures that move by themselves: redrawn while they are shown */
static int syn_animated(void)
{
    const spage_t *g = syn_pg();
    return g->diag == SD_LFO || (g->diag == SD_WT && syn.p[P_LDEPTH] && syn.p[P_LDEST] == L_WT);
}

/* ------------------------------------------------------------- the knobs --- */
static void knob_draw(int i, int p, int32_t y)
{
    const spage_t *g = syn_pg();
    const int32_t x0 = 4 + i * 58, cx = x0 + 28, cy = y + 12, r = 11;
    int lo, hi, k, goal = g->kind == 2 && (p == g->gp || (p == V_HLVL && g->gp == P_H1 + syn.hsel));
    char b[24], kn[3] = {'K', (char)('1' + i), 0};
    const char *v, *name = SPNAME[p][th_ui];
    float f;
    uint16_t ring = goal ? (syn.done ? COL_OK : C_ORANGE) : C_GRAY;
    syn_range(p == V_HLVL ? P_H1 : p, &lo, &hi);
    f = (float)(syn_get(p) - lo) / (float)(hi - lo);
    if (syn.last_k == i)
        cv_rect(x0, y - 1, 56, 61, COL_SELBG);
    t1_tx(x0 + 2, y, &FONT_T, kn, C_DIM);
    for (k = 0; k < 64; k++) {                       /* the ring */
        float a = (float)k / 64.0f;
        cv_pset(cx + (int32_t)(fm_sin_turns(a) * (float)r), cy - (int32_t)(fm_sin_turns(a + 0.25f) * (float)r), ring);
    }
    for (k = 0; k <= (int)(f * 48.0f); k++) {        /* the arc travelled, from 7 o'clock */
        float a = -0.375f + (float)k / 48.0f * 0.75f;
        cv_pset(cx + (int32_t)(fm_sin_turns(a - fm_floorf(a)) * (float)(r + 2)),
                cy - (int32_t)(fm_sin_turns(a - fm_floorf(a) + 0.25f) * (float)(r + 2)), C_HI);
    }
    {
        float a = -0.375f + f * 0.75f;
        a -= fm_floorf(a);
        cv_line(cx, cy, cx + (int32_t)(fm_sin_turns(a) * (float)(r - 2)), cy - (int32_t)(fm_sin_turns(a + 0.25f) *
                (float)(r - 2)), C_WHITE);
    }
    if (goal) {                                      /* the goal: a mark outside the ring */
        float a = -0.375f + (float)(g->gv - lo) / (float)(hi - lo) * 0.75f;
        a -= fm_floorf(a);
        cv_rect(cx + (int32_t)(fm_sin_turns(a) * (float)(r + 5)) - 1,
                cy - (int32_t)(fm_sin_turns(a + 0.25f) * (float)(r + 5)) - 1, 3, 3, syn.done ? COL_OK : C_ORANGE);
    }
    if (t1_tw(&FONT_XS, name) > 56)
        text_cut(name);
    small_c(cx, y + 26, name, goal ? (syn.done ? COL_OK : C_ORANGE) : C_HI);
    v = syn_value(p, b);
    if (t1_tw(&FONT_B, v) <= 56)
        t1_tx(cx - t1_tw(&FONT_B, v) / 2, y + 41, &FONT_B, v, C_WHITE);
    else {
        if (t1_tw(&FONT_XS, v) > 56)
            text_cut(v);
        small_c(cx, y + 43, v, C_WHITE);
    }
}

/* ------------------------------------------------------------- the screens --- */
static const char *syn_ch_name(int i)
{
    static char b[48];
    b[0] = 0;
    cat_int(b, i + 1);
    str_cat(b, ". ");
    str_cat(b, SCHAPS[i].title[th_ui]);
    return b;
}
static const char *syn_ch_sub(int i, char *b) { (void)b; return SMOD[SCHAPS[i].module][th_ui]; }
static void draw_synlist(void)
{
    ui_header(SW(SS_MENU), 0, 0);
    big_list(NSCHAP, syn.sel, syn_ch_name, syn_ch_sub, C_HI);
    hint(S(S_H_TOPICS));
}

static void draw_synpage(void)
{
    const spage_t *g = syn_pg();
    const schap_t *c = &SCHAPS[syn.ch];
    char t[12];
    int i;
    t[0] = 0;
    cat_int(t, syn.page + 1);
    str_cat(t, "/");
    cat_int(t, c->n);
    ui_header(c->title[th_ui], t, C_GRAY);
    if (g->kind == 0) {
        if (g->diag != SD_NONE) {
            syn_diag(g->diag, 8, 30, 232, 116);
            para(8, 122, 224, g->text[th_ui], C_WHITE, 222);
        } else
            para(8, 32, 224, g->text[th_ui], C_WHITE, 222);
    } else {
        para(8, 27, 224, g->text[th_ui], g->kind == 2 && !syn.done ? COL_ALT : C_WHITE, 80);
        syn_diag(g->diag, 8, 85, 232, 148);
        for (i = 0; i < g->nk; i++)
            knob_draw(i, g->knob[i], 152);
    }
    if (g->kind == 2 && syn.done)
        text_c(222, &FONT_B, SW(SS_DONE), COL_OK);
    else
        text_c(228, &FONT_T, SW(SS_H_PAGE), C_GRAY);
}

/* --------------------------------------------------------------- playing --- */
static void syn_demo(void)
{
    static const uint8_t SCALE[8] = {60, 62, 64, 65, 67, 69, 71, 72};
    static const uint8_t LEG[7] = {60, 62, 64, 67, 64, 62, 60};
    static const uint8_t BASS[8] = {36, 36, 48, 36, 43, 36, 46, 48};
    uint32_t t = plat_ms() + 30u;
    int i;
    play_stop();
    switch (SCHAPS[syn.ch].demo) {
    case SDEMO_SCALE:
        for (i = 0; i < 8; i++)
            play_note(t + (uint32_t)i * 300u, SCALE[i], 260u, 100);
        break;
    case SDEMO_CHORD:
        for (i = 0; i < 4; i++)
            play_note(t + (uint32_t)i * 20u, SCALE[i * 2], 1800u, 90);
        break;
    case SDEMO_PLUCK:
        for (i = 0; i < 6; i++)
            play_note(t + (uint32_t)i * 220u, 48 + SCALE[i % 5] - 60 + (i > 2 ? 12 : 0), 140u, 110);
        break;
    case SDEMO_KICK:
        for (i = 0; i < 4; i++)
            play_note(t + (uint32_t)i * 450u, 36, 150u, 120);
        break;
    case SDEMO_HOLD: play_note(t, 57, 2600u, 100); break;
    case SDEMO_LEGATO:
        for (i = 0; i < 7; i++)
            play_note(t + (uint32_t)i * 320u, LEG[i], 380u, 100);
        break;
    case SDEMO_BASS:
        for (i = 0; i < 8; i++)
            play_note(t + (uint32_t)i * 220u, BASS[i], 160u, 110);
        break;
    case SDEMO_STAB:
        for (i = 0; i < 3; i++) {
            play_note(t + (uint32_t)i * 380u, 60, 150u, 110);
            play_note(t + (uint32_t)i * 380u, 64, 150u, 110);
            play_note(t + (uint32_t)i * 380u, 67, 150u, 110);
        }
        break;
    default: play_note(t, 57, 1200u, 100); break;
    }
    syn.note = 57;
}

static void syn_knob(int k, int d)
{
    const spage_t *g = syn_pg();
    int p, step;
    if (g->kind == 0 || k >= g->nk)
        return;
    p = g->knob[k];
    step = p == P_FAMT ? 2 : 1;
    syn_set(p, syn_get(p) + d * step);
    syn.last_k = (int8_t)k;
    if (g->kind == 2 && !syn.done && syn_goal_met()) {
        syn.done = 1;
        app_say(SW(SS_GOAL), "OK");
    }
    if (g->kind == 2 && !syn_goal_met())
        syn.done = 0;
}

static void syn_page_go(int d)
{
    int n = SCHAPS[syn.ch].n, p = syn.page + d;
    if (p < 0 || p >= n)
        return;
    syn.page = (uint8_t)p;
    syn_page_enter();
}
