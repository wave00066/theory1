/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* A two-operator FM voice (a modulator into a carrier), three timbres: an electric piano (the index decays with
 * the note), an organ (steady, a ratio of 2) and a bell (an inharmonic ratio, a long ring). Single precision only,
 * no libm (fastmath.h): the FM-1's FPU. */
#include "synth.h"
#include "fastmath.h"

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
    uint32_t age;
} voice_t;

typedef struct {
    float ratio, idx0, idx_decay, decay, release, level;
} sound_t;
/* per-sample multipliers at 44.1 kHz: 0.99996 falls 60 dB in about 4 s, 0.9990 in about 0.16 s */
static const sound_t SOUNDS[NSOUNDS] = {
    {1.0f, 0.30f, 0.99985f, 0.99996f, 0.9990f, 0.30f},   /* electric piano */
    {2.0f, 0.10f, 1.0f, 1.0f, 0.9993f, 0.20f},           /* organ */
    {3.5f, 0.45f, 0.99993f, 0.99998f, 0.9996f, 0.26f},   /* bell */
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
void synth_sound(int s) { snd = s < 0 || s >= NSOUNDS ? 0 : s; }

void synth_panic(void)
{
    int i;
    for (i = 0; i < NV; i++) {
        v[i].note = -1;
        v[i].amp = 0.0f;
    }
    ev_r = ev_w;
}

int synth_active(void)
{
    int i, n = 0;
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
    v[pick].age = age_now++;
}

static void events(void)
{
    while (ev_r != ev_w) {
        uint32_t e = ev_q[ev_r % RING], type = e >> 16, note = e & 0x7Fu;
        int i;
        ev_r = ev_r + 1u;
        if (type == 1u)
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
                vo->amp *= vo->held ? s->decay : s->release;
            }
            vo->idx *= s->idx_decay;
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
