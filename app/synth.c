/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* A two-operator FM voice (a modulator into a carrier), four timbres: a piano (a bright hammer that softens at
 * once, low notes ringing longer than high ones, a damper on release), an electric piano (the index decays with
 * the note), an organ (steady, a ratio of 2) and a bell (an inharmonic ratio, a long ring). Single precision only,
 * no libm (fastmath.h): the FM-1's FPU. */
#include "synth.h"
#include "fastmath.h"
#include "synthmath.h"

static void patch_event(uint32_t type, int note, int vel);

#define NV 8
#define RING 64u
#define SR 44100.0f

typedef struct {
    int note;                           /* -1 free */
    int held;                           /* key still down: no release yet */
    float ph, inc;                      /* the carrier's phase and step, in turns */
    float amp, amp_target;
    float idx;                          /* the modulation index, in turns */
    float vel;
    float dec, idec;                    /* this note's decay and brightness multipliers (key tracking) */
    uint32_t age;
} voice_t;

typedef struct {
    float ratio, idx0, idx_decay, decay, release, level;
    float track;                        /* 0: every note decays alike; 1: twice as fast each two octaves up */
} sound_t;
/* per-sample multipliers at 44.1 kHz: 0.99996 falls 60 dB in about 4 s, 0.9990 in about 0.16 s */
static const sound_t SOUNDS[NSOUNDS] = {
    {1.0f, 0.42f, 0.99955f, 0.99994f, 0.9975f, 0.34f, 1.0f},  /* piano: middle C fades in about 2.6 s */
    {1.0f, 0.30f, 0.99985f, 0.99996f, 0.9990f, 0.30f, 0.0f},  /* electric piano */
    {2.0f, 0.10f, 1.0f, 1.0f, 0.9993f, 0.20f, 0.0f},          /* organ */
    {3.5f, 0.45f, 0.99993f, 0.99998f, 0.9996f, 0.26f, 0.0f},  /* bell */
};

static voice_t v[NV];
static volatile uint32_t ev_w, ev_r;
static uint32_t ev_q[RING];
static volatile int snd;
static uint32_t age_now;
static float lim = 1.0f;                /* the limiter's gain */

static void push(uint32_t e)
{
    uint32_t w = ev_w;
    if (w - ev_r >= RING)               /* full: drop (64 events a block is far more than a person plays) */
        return;
    ev_q[w % RING] = e;
    ev_w = w + 1u;                      /* published after the slot is written */
}

void synth_note_on(int note, int vel)
{
    if (note < 0 || note > 127)
        return;
    push(1u << 16 | (uint32_t)note | (uint32_t)(vel < 1 ? 1 : vel > 127 ? 127 : vel) << 8);
}
void synth_note_off(int note)
{
    if (note >= 0 && note <= 127)
        push(2u << 16 | (uint32_t)note);
}
void synth_all_off(void) { push(3u << 16); }
void synth_mode(int patch) { push(4u << 16 | (uint32_t)(patch != 0)); }
void synth_sound(int s) { snd = s < 0 || s >= NSOUNDS ? 0 : s; }

void synth_panic(void)
{
    int i;
    for (i = 0; i < NV; i++) {
        v[i].note = -1;
        v[i].amp = 0.0f;
    }
    patch_event(5u, 0, 0);
    ev_r = ev_w;
}

static int mode;                        /* 0 the lessons' sounds, 1 the SYNTH section's patch */
static int patch_active(void);
static void patch_render(int32_t *lr, uint32_t n, float gain);

int synth_active(void)
{
    int i, n = 0;
    if (mode)
        return patch_active();
    for (i = 0; i < NV; i++)
        n += v[i].note >= 0;
    return n;
}

static void start(int note, int vel)
{
    int i, pick = -1;
    uint32_t oldest = 0;
    for (i = 0; i < NV; i++)            /* the same note again takes its own voice */
        if (v[i].note == note)
            pick = i;
    for (i = 0; pick < 0 && i < NV; i++)   /* else a free one */
        if (v[i].note < 0)
            pick = i;
    if (pick < 0)                       /* else the oldest */
        for (i = 0; i < NV; i++)
            if (pick < 0 || age_now - v[i].age > oldest) {
                oldest = age_now - v[i].age;
                pick = i;
            }
    v[pick].note = note;
    v[pick].held = 1;
    v[pick].ph = 0.0f;
    v[pick].inc = 440.0f * fm_exp2f((float)(note - 69) / 12.0f) / SR;
    v[pick].vel = 0.35f + 0.65f * (float)vel / 127.0f;
    v[pick].amp_target = v[pick].vel;
    v[pick].idx = SOUNDS[snd].idx0 * (0.6f + 0.6f * (float)vel / 127.0f);
    {   /* key tracking: the distance from 1 scaled, 2^((note - 60) / 24) */
        float k = SOUNDS[snd].track > 0.0f ? fm_exp2f((float)(note - 60) / 24.0f * SOUNDS[snd].track) : 1.0f;
        v[pick].dec = 1.0f - (1.0f - SOUNDS[snd].decay) * k;
        v[pick].idec = 1.0f - (1.0f - SOUNDS[snd].idx_decay) * k;
    }
    v[pick].age = age_now++;
}

static void events(void)
{
    while (ev_r != ev_w) {
        uint32_t e = ev_q[ev_r % RING], type = e >> 16, note = e & 0x7Fu;
        int i;
        ev_r = ev_r + 1u;
        if (type == 4u) {                   /* the engine changes: everything stops */
            for (i = 0; i < NV; i++)
                v[i].note = -1;
            patch_event(3u, 0, 0);
            patch_event(5u, 0, 0);
            mode = (int)(e & 1u);
        } else if (mode)
            patch_event(type, (int)note, (int)(e >> 8 & 0x7Fu));
        else if (type == 1u)
            start((int)note, (int)(e >> 8 & 0x7Fu));
        else
            for (i = 0; i < NV; i++)
                if (v[i].note >= 0 && (type == 3u || v[i].note == (int)note))
                    v[i].held = 0;
    }
}

void synth_render(int32_t *lr, uint32_t n, float gain)
{
    const sound_t *s = &SOUNDS[snd];
    uint32_t i;
    int k;
    events();
    if (mode) {
        patch_render(lr, n, gain);
        return;
    }
    for (i = 0; i < n; i++) {
        float out = 0.0f, a;
        for (k = 0; k < NV; k++) {
            voice_t *vo = &v[k];
            float m;
            if (vo->note < 0)
                continue;
            /* the envelope: a quick attack to the velocity, a decay while held, a release after */
            if (vo->amp_target > 0.0f) {
                vo->amp += (vo->amp_target - vo->amp) * 0.02f;
                if (vo->amp >= vo->amp_target * 0.98f)
                    vo->amp_target = 0.0f;
            } else {
                vo->amp *= vo->held ? vo->dec : s->release;
            }
            vo->idx *= vo->idec;
            m = fm_sin_turns(vo->ph * s->ratio);
            out += fm_sin_turns(vo->ph + vo->idx * m) * vo->amp;
            vo->ph += vo->inc;
            if (vo->ph >= 1.0f)
                vo->ph -= 1.0f;
            if (vo->amp_target <= 0.0f && vo->amp < 0.0005f)
                vo->note = -1;
        }
        out *= s->level;
        /* a gentle limiter: chords of eight notes stay clean */
        a = fm_fabsf(out) * lim;
        if (a > 0.95f)
            lim = 0.95f / fm_fabsf(out);
        else
            lim += (1.0f - lim) * 0.0002f;
        out *= lim;
        lr[2 * i] = lr[2 * i + 1] = (int32_t)(out * gain);
    }
}

/* ===================================================== the SYNTH section === */
/* A teaching synthesizer, the blocks of the SYNTH section's lessons: two oscillators (anti-aliased: polyBLEP) and
 * noise, or two FM operators, or an additive table, or a wavetable; a state-variable filter (12 dB/octave: low,
 * band, high pass); two ADSR envelopes (amplifier, filter), a pitch envelope, one LFO; mono with glide or six
 * voices; drive and a stereo chorus. The control signals run every 16 samples, the audio every sample. */
#define PV 6
#define CR 16u                          /* the control rate: every 16 samples (0.36 ms) */
#define CDT ((float)CR / SYN_FS)

static volatile int16_t P[NPAR];
void synth_set(int p, int v_)
{
    if (p >= 0 && p < NPAR)
        P[p] = (int16_t)v_;
}

typedef struct {
    int note;                           /* -1 free */
    int gate;
    float vel;
    float st, st_to;                    /* the pitch in semitones, gliding to st_to */
    float ph1, ph2, phm, mprev;
    int as;                             /* the amplifier's stage: 0 off, 1 attack, 2 decay and sustain, 3 release */
    float al, al_prev;                  /* its level, now and at the last control step (the audio interpolates) */
    int fs_;                            /* the filter envelope's stage */
    float fl;
    float pl;                           /* the pitch envelope, 1 -> 0 */
    float ml;                           /* the FM modulator's envelope, 1 -> 0 */
    float ic1, ic2, a1, a2, a3, k;      /* the filter */
    float inc1, inc2, incm;
    uint32_t age;
} pvoice_t;

static pvoice_t pv[PV];
static uint32_t vage;                   /* the voices' ages */
static uint8_t stack[8], nstack;        /* mono: the keys held, the last one sounding */
static float last_st = 60.0f;           /* the last note played: glide starts there */
static float lfo_ph, lfo_val, lfo_sh;
static uint32_t nz = 0x9E3779B9u;
static float tbl[257];                  /* the additive wave */
static int32_t tbl_sum = -1;
#define CHN 1024
static int16_t ch_buf[CHN];
static uint32_t ch_w;
static float ch_ph;
static float plim = 1.0f;

static float noise(void)
{
    nz ^= nz << 13;
    nz ^= nz >> 17;
    nz ^= nz << 5;
    return (float)(int32_t)nz * (1.0f / 2147483648.0f);
}

static int patch_active(void)
{
    int i, n = 0;
    for (i = 0; i < PV; i++)
        n += pv[i].note >= 0;
    return n;
}

static void v_trigger(pvoice_t *o, int note, float vel, int retrig)
{
    o->note = note;
    o->gate = 1;
    o->vel = vel;
    o->st_to = (float)note;
    if (retrig) {
        o->st = P[P_GLIDE] > 0 ? last_st : (float)note;
        if (o->as == 0) {
            o->ph1 = o->ph2 = o->phm = 0.0f;
            o->mprev = 0.0f;
            o->ic1 = o->ic2 = 0.0f;
            o->al = o->al_prev = o->fl = 0.0f;
        }
        o->as = 1;
        o->fs_ = 1;
        o->pl = 1.0f;
        o->ml = 1.0f;
    }
    o->age = vage++;
    last_st = (float)note;
}

static void patch_event(uint32_t type, int note, int vel)
{
    int i;
    if (type == 5u) {                   /* silence at once */
        for (i = 0; i < PV; i++) {
            pv[i].note = -1;
            pv[i].as = 0;
            pv[i].al = pv[i].al_prev = 0.0f;
        }
        nstack = 0;
        return;
    }
    if (type == 3u) {                   /* all keys up */
        for (i = 0; i < PV; i++)
            if (pv[i].note >= 0) {
                pv[i].gate = 0;
                pv[i].as = 3;
                pv[i].fs_ = 3;
            }
        nstack = 0;
        return;
    }
    if (P[P_POLY]) {                    /* mono: one voice, the last key held sounds, legato glides */
        pvoice_t *o = &pv[0];
        if (type == 1u) {
            for (i = 0; i < nstack; i++)
                if (stack[i] == note)
                    break;
            if (i == nstack && nstack < 8)
                stack[nstack++] = (uint8_t)note;
            v_trigger(o, note, (float)vel / 127.0f, !(o->gate && nstack > 1));
        } else {
            for (i = 0; i < nstack; i++)
                if (stack[i] == note) {
                    for (; i + 1 < nstack; i++)
                        stack[i] = stack[i + 1];
                    nstack--;
                    break;
                }
            if (nstack && o->gate) {
                o->note = stack[nstack - 1];
                o->st_to = (float)o->note;
                last_st = o->st_to;
            } else if (!nstack && o->gate) {
                o->gate = 0;
                o->as = 3;
                o->fs_ = 3;
            }
        }
        return;
    }
    if (type == 1u) {
        int pick = -1;
        uint32_t oldest = 0;
        for (i = 0; i < PV; i++)        /* the same note again: its voice */
            if (pv[i].note == note)
                pick = i;
        for (i = 0; pick < 0 && i < PV; i++)
            if (pv[i].note < 0)
                pick = i;
        for (i = 0; pick < 0 && i < PV; i++)   /* else one released, the oldest */
            if (!pv[i].gate && vage - pv[i].age >= oldest) {
                oldest = vage - pv[i].age;
                pick = i;
            }
        for (i = 0; pick < 0 && i < PV; i++)
            if (vage - pv[i].age >= oldest) {
                oldest = vage - pv[i].age;
                pick = i;
            }
        v_trigger(&pv[pick], note, (float)vel / 127.0f, 1);
    } else {
        for (i = 0; i < PV; i++)
            if (pv[i].note == note && pv[i].gate) {
                pv[i].gate = 0;
                pv[i].as = 3;
                pv[i].fs_ = 3;
            }
    }
}

/* polyBLEP: the step's correction near a discontinuity */
static inline float blep(float t, float dt)
{
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}
static inline float osc(int w, float t, float dt, float pw)
{
    switch (w) {
    case W_SINE: return fm_sin_turns(t);
    case W_TRI: return t < 0.25f ? 4.0f * t : t < 0.75f ? 2.0f - 4.0f * t : 4.0f * t - 4.0f;
    case W_SAW: {
        float u = t + 0.5f;
        if (u >= 1.0f)
            u -= 1.0f;
        return 2.0f * u - 1.0f - blep(u, dt);
    }
    case W_SQUARE: pw = 0.5f; /* fall through */
    case W_PULSE: {
        float u = t + 1.0f - pw, y = (t < pw ? 1.0f : -1.0f) + blep(t, dt);
        if (u >= 1.0f)
            u -= 1.0f;
        return y - blep(u, dt) - (2.0f * pw - 1.0f);
    }
    default: return noise();
    }
}

/* the additive wave, rebuilt when a harmonic's level changes */
static void tbl_build(void)
{
    int32_t sum = 0, i, h;
    float peak = 0.0f;
    for (h = 0; h < 8; h++)
        sum = sum * 31 + P[P_H1 + h];
    if (sum == tbl_sum)
        return;
    tbl_sum = sum;
    for (i = 0; i < 256; i++) {
        float y = 0.0f;
        for (h = 0; h < 8; h++)
            if (P[P_H1 + h])
                y += (float)P[P_H1 + h] * 0.01f * fm_sin_turns((float)((i * (h + 1)) & 255) * (1.0f / 256.0f));
        tbl[i] = y;
        if (fm_fabsf(y) > peak)
            peak = fm_fabsf(y);
    }
    peak = peak > 0.0f ? 1.0f / peak : 0.0f;
    for (i = 0; i < 256; i++)
        tbl[i] *= peak;
    tbl[256] = tbl[0];
}

/* an envelope's step: the attack a straight line, the decay and the release falling exponentially */
typedef struct {
    float att, dec, sus, rel;           /* the per-step increase; the per-step multipliers */
} envk_t;
static envk_t env_k(int a, int d, int s_, int r)
{
    envk_t k;
    k.att = CDT / (syn_ms(a) * 0.001f);
    k.dec = fm_expf(-CDT * 5.0f / (syn_ms(d) * 0.001f));
    k.sus = (float)s_ * 0.01f;
    k.rel = fm_expf(-CDT * 5.0f / (syn_ms(r) * 0.001f));
    return k;
}
static float env_step(int *stage, float l, const envk_t *k)
{
    switch (*stage) {
    case 1:
        l += k->att;
        if (l >= 1.0f) {
            l = 1.0f;
            *stage = 2;
        }
        return l;
    case 2: return k->sus + (l - k->sus) * k->dec;
    case 3:
        l *= k->rel;
        if (l < 0.0002f) {
            l = 0.0f;
            *stage = 0;
        }
        return l;
    default: return 0.0f;
    }
}

static void patch_render(int32_t *lr, uint32_t n, float gain)
{
    /* the block's constants, from the knobs */
    const int eng = P[P_ENGINE], w1 = P[P_WAVE], w2 = P[P_WAVE2], ftype = P[P_FTYPE], dest = P[P_LDEST];
    const float pw0 = (float)P[P_PW] * 0.01f, o2 = (float)P[P_OSC2] * 0.01f, nmix = (float)P[P_NOISE] * 0.01f;
    const float det = fm_exp2f((float)P[P_DETUNE] / 1200.0f), ratio = SYN_RATIO[P[P_RATIO] % NRATIO];
    const float index = syn_index(P[P_INDEX]) * (1.0f / 6.2831853f), fb = (float)P[P_FB] * 0.004f;
    const float kt = (float)P[P_KTRACK] * 0.01f, famt = (float)P[P_FAMT] * 0.07f, velamt = (float)P[P_VELAMT] * 0.01f;
    const float kq = 1.0f / syn_q(P[P_RESO]), depth = (float)P[P_LDEPTH] * 0.01f;
    const float pdec = fm_expf(-CDT * 3.0f / (syn_ms(P[P_PDEC]) * 0.001f)), pamt = (float)P[P_PAMT];
    const float mdec = P[P_MDEC] >= 100 ? 1.0f : fm_expf(-CDT * 3.0f / (syn_ms(P[P_MDEC]) * 0.001f));
    const float glide = P[P_GLIDE] > 0 ? 1.0f - fm_expf(-CDT * 3.0f / (syn_ms(P[P_GLIDE]) * 0.001f)) : 1.0f;
    const float lfo_inc = syn_lfo_hz(P[P_LRATE]) * CDT, drive = (float)P[P_DRIVE] * 0.08f;
    const float chorus = (float)P[P_CHORUS] * 0.01f, vol = (float)P[P_VOL] * 0.01f;
    const float oct = 12.0f * (float)P[P_OCT];
    const envk_t ka = env_k(P[P_A], P[P_D], P[P_S], P[P_R]), kf = env_k(P[P_FA], P[P_FD], P[P_FS], P[P_FR]);
    const float cut0 = (float)P[P_CUTOFF];
    uint32_t i0, i;
    int k;
    if (eng == E_ADD)
        tbl_build();
    for (i0 = 0; i0 < n; i0 += CR) {
        uint32_t m = n - i0 < CR ? n - i0 : CR;
        float lfo, l_pitch = 0.0f, l_cut = 0.0f, l_amp = 1.0f, l_pw = 0.0f, l_wt = 0.0f;
        float buf[CR];
        /* the LFO: one for all the voices */
        lfo_ph += lfo_inc;
        if (lfo_ph >= 1.0f) {
            lfo_ph -= 1.0f;
            lfo_sh = noise();
        }
        switch (P[P_LSHAPE]) {
        case 0: lfo = fm_sin_turns(lfo_ph); break;
        case 1: lfo = syn_wave(W_TRI, lfo_ph, 0.5f); break;
        case 2: lfo = 1.0f - 2.0f * lfo_ph; break;
        case 3: lfo = lfo_ph < 0.5f ? 1.0f : -1.0f; break;
        default: lfo = lfo_sh; break;
        }
        lfo_val = lfo;
        switch (dest) {
        case L_PITCH: l_pitch = lfo * depth * 2.0f; break;          /* up to 2 semitones */
        case L_CUTOFF: l_cut = lfo * depth * 30.0f; break;          /* up to 3 octaves (in knob steps) */
        case L_AMP: l_amp = 1.0f - depth * (0.5f + 0.5f * lfo); break;
        case L_PW: l_pw = lfo * depth * 0.45f; break;
        default: l_wt = lfo * depth * 50.0f; break;
        }
        for (i = 0; i < m; i++)
            buf[i] = 0.0f;
        for (k = 0; k < PV; k++) {
            pvoice_t *o = &pv[k];
            float pitch, f, fc, g, a0, a1s, pw, wtp;
            if (o->note < 0)
                continue;
            /* the control step: pitch, envelopes, the filter's coefficients */
            o->st += (o->st_to - o->st) * glide;
            pitch = o->st + oct + o->pl * pamt + l_pitch;
            o->pl *= pdec;
            o->ml *= mdec;
            o->al_prev = o->al;
            o->al = env_step(&o->as, o->al, &ka);
            o->fl = env_step(&o->fs_, o->fl, &kf);
            if (o->as == 0) {
                o->note = -1;
                continue;
            }
            f = 440.0f * fm_exp2f((pitch - 69.0f) * (1.0f / 12.0f));
            o->inc1 = fm_minf(f / SYN_FS, 0.45f);
            o->inc2 = fm_minf(o->inc1 * det, 0.45f);
            o->incm = fm_minf(o->inc1 * ratio, 0.45f);
            fc = syn_cutoff(cut0 + l_cut) * fm_exp2f((pitch - 60.0f) * kt * (1.0f / 12.0f) +
                                                    o->fl * famt * (1.0f - velamt + velamt * o->vel));
            fc = fm_clampf(fc, 20.0f, 0.42f * SYN_FS);
            g = fm_tanf(3.14159265f * fc / SYN_FS);
            o->k = kq;
            o->a1 = 1.0f / (1.0f + g * (g + kq));
            o->a2 = g * o->a1;
            o->a3 = g * o->a2;
            pw = fm_clampf(pw0 + l_pw, 0.05f, 0.95f);
            wtp = (float)P[P_WTPOS] + l_wt;
            a0 = o->al_prev * (0.35f + 0.65f * o->vel) * l_amp;
            a1s = (o->al * (0.35f + 0.65f * o->vel) * l_amp - a0) * (1.0f / (float)CR);
            /* the audio */
            for (i = 0; i < m; i++) {
                float s, v1, v2, v3;
                switch (eng) {
                case E_FM: {
                    float md = fm_sin_turns(o->phm + fb * o->mprev);
                    o->mprev = md;
                    s = fm_sin_turns(o->ph1 + index * o->ml * md);
                    break;
                }
                case E_ADD: {
                    float x = o->ph1 * 256.0f;
                    int j = (int)x;
                    s = tbl[j] + (tbl[j + 1] - tbl[j]) * (x - (float)j);
                    break;
                }
                case E_WT: s = syn_wt(wtp, o->ph1); break;
                default:
                    s = osc(w1, o->ph1, o->inc1, pw);
                    if (o2 > 0.0f)
                        s = (s + o2 * osc(w2, o->ph2, o->inc2, pw)) * (1.0f / (1.0f + o2 * 0.6f));
                    break;
                }
                if (nmix > 0.0f)
                    s = s * (1.0f - nmix) + noise() * nmix;
                o->ph1 += o->inc1;
                if (o->ph1 >= 1.0f)
                    o->ph1 -= 1.0f;
                o->ph2 += o->inc2;
                if (o->ph2 >= 1.0f)
                    o->ph2 -= 1.0f;
                o->phm += o->incm;
                if (o->phm >= 1.0f)
                    o->phm -= 1.0f;
                /* the filter (a TPT state-variable filter) */
                v3 = s - o->ic2;
                v1 = o->a1 * o->ic1 + o->a2 * v3;
                v2 = o->ic2 + o->a2 * o->ic1 + o->a3 * v3;
                o->ic1 = 2.0f * v1 - o->ic1;
                o->ic2 = 2.0f * v2 - o->ic2;
                s = ftype == 0 ? v2 : ftype == 1 ? v1 * o->k * 1.5f : s - o->k * v1 - v2;
                buf[i] += s * a0;
                a0 += a1s;
            }
            o->ic1 = fm_flush(o->ic1);
            o->ic2 = fm_flush(o->ic2);
        }
        /* the mix: drive, the chorus (stereo), the volume, a limiter */
        for (i = 0; i < m; i++) {
            float x = buf[i] * 0.45f, l, r, a;
            if (drive > 0.0f)
                x = fm_tanhf(x * (1.0f + drive)) * (1.0f / (1.0f + drive * 0.25f));
            l = r = x;
            if (chorus > 0.0f) {
                float d1, d2;
                int32_t j1, j2;
                ch_buf[ch_w & (CHN - 1u)] = (int16_t)(fm_clampf(x, -1.0f, 1.0f) * 32767.0f);
                ch_ph += 0.6f / SYN_FS;
                if (ch_ph >= 1.0f)
                    ch_ph -= 1.0f;
                d1 = 400.0f + 180.0f * fm_sin_turns(ch_ph);           /* 9 ms, +-4 ms */
                d2 = 400.0f + 180.0f * fm_sin_turns(ch_ph + 0.5f);
                j1 = (int32_t)d1;
                j2 = (int32_t)d2;
                {
                    float y1 = (float)ch_buf[(ch_w - (uint32_t)j1) & (CHN - 1u)] +
                               ((float)ch_buf[(ch_w - (uint32_t)j1 - 1u) & (CHN - 1u)] -
                                (float)ch_buf[(ch_w - (uint32_t)j1) & (CHN - 1u)]) * (d1 - (float)j1);
                    float y2 = (float)ch_buf[(ch_w - (uint32_t)j2) & (CHN - 1u)] +
                               ((float)ch_buf[(ch_w - (uint32_t)j2 - 1u) & (CHN - 1u)] -
                                (float)ch_buf[(ch_w - (uint32_t)j2) & (CHN - 1u)]) * (d2 - (float)j2);
                    l = x * (1.0f - chorus * 0.4f) + y1 * (chorus * 0.7f / 32767.0f);
                    r = x * (1.0f - chorus * 0.4f) + y2 * (chorus * 0.7f / 32767.0f);
                }
                ch_w++;
            }
            l *= vol;
            r *= vol;
            a = fm_maxf(fm_fabsf(l), fm_fabsf(r)) * plim;
            if (a > 0.95f)
                plim = 0.95f / fm_maxf(fm_fabsf(l), fm_fabsf(r));
            else
                plim += (1.0f - plim) * 0.0002f;
            lr[2 * (i0 + i)] = (int32_t)(l * plim * gain);
            lr[2 * (i0 + i) + 1] = (int32_t)(r * plim * gain);
        }
    }
}

float synth_lfo(void) { return lfo_val; }
