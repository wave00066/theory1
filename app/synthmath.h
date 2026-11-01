/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* The teaching synth's parameters and the curves that turn a knob's position into a time, a frequency or a level:
 * one place, so the sound (synth.c) and the pictures (synthlab.c) agree. Single precision only (fastmath.h). */
#pragma once
#include <stdint.h>
#include "fastmath.h"

/* a patch: what the SYNTH section's knobs move */
enum {
    P_ENGINE,                   /* 0 subtractive, 1 FM, 2 additive, 3 wavetable */
    P_WAVE,                     /* oscillator 1: W_* */
    P_PW,                       /* the pulse's width, 5..95 % */
    P_OCT,                      /* -2..+1 octaves */
    P_OSC2,                     /* oscillator 2's level, 0..100 */
    P_WAVE2,                    /* oscillator 2's wave: W_SINE..W_PULSE */
    P_DETUNE,                   /* oscillator 2, 0..50 cents */
    P_NOISE,                    /* the noise in the mix, 0..100 */
    P_CUTOFF,                   /* 0..100: 30 Hz .. 20 kHz */
    P_RESO,                     /* 0..100: Q 0.5 .. 30 */
    P_FTYPE,                    /* 0 low-pass, 1 band-pass, 2 high-pass */
    P_KTRACK,                   /* 0..100 % */
    P_A, P_D, P_S, P_R,         /* the amplifier's envelope: times 0..100 (1 ms .. 10 s), sustain 0..100 */
    P_FA, P_FD, P_FS, P_FR,     /* the filter's envelope */
    P_FAMT,                     /* -100..100: up to 7 octaves */
    P_PAMT,                     /* the pitch envelope: 0..48 semitones */
    P_PDEC,                     /* and its fall, a time */
    P_LRATE,                    /* the LFO: 0..100 (0.05 .. 20 Hz) */
    P_LDEPTH,                   /* 0..100 */
    P_LDEST,                    /* L_* */
    P_LSHAPE,                   /* 0 sine, 1 triangle, 2 saw, 3 square, 4 sample & hold */
    P_RATIO,                    /* FM: the modulator's ratio, an index into SYN_RATIO */
    P_INDEX,                    /* FM: 0..100 (0 .. 8) */
    P_MDEC,                     /* FM: the modulator's fall, a time; 100: it holds */
    P_FB,                       /* FM: the modulator's feedback, 0..100 */
    P_H1, P_H2, P_H3, P_H4, P_H5, P_H6, P_H7, P_H8,   /* additive: each harmonic's level, 0..100 */
    P_WTPOS,                    /* wavetable: the position, 0..100 */
    P_POLY,                     /* 0 poly, 1 mono */
    P_GLIDE,                    /* a time; 0 none */
    P_DRIVE, P_CHORUS, P_VOL,   /* 0..100 */
    P_VELAMT,                   /* velocity on the filter envelope, 0..100 */
    NPAR
};
enum { W_SINE, W_TRI, W_SAW, W_SQUARE, W_PULSE, W_NOISE, NWAVES };
enum { L_PITCH, L_CUTOFF, L_AMP, L_PW, L_WT, NLDEST };
enum { E_SUB, E_FM, E_ADD, E_WT, NENGINES };
#define NRATIO 10
static const float SYN_RATIO[NRATIO] = {0.5f, 1.0f, 1.41f, 2.0f, 3.0f, 3.5f, 4.0f, 5.0f, 7.0f, 14.0f};
#define SYN_FS 44100.0f

/* a time knob: 0 -> 1 ms, 25 -> 10 ms, 50 -> 100 ms, 75 -> 1 s, 100 -> 10 s */
static inline float syn_ms(int x) { return fm_exp2f((float)x * (3.321928f / 25.0f)); }
/* the cutoff: 30 Hz .. 20 kHz, an octave every 10.6 */
static inline float syn_cutoff(float x) { return 30.0f * fm_exp2f(x * 0.094f); }
static inline float syn_q(int x) { return 0.5f * fm_expf((float)x * 0.040943f); }       /* 0.5 .. 30 */
static inline float syn_lfo_hz(int x) { return 0.05f * fm_exp2f((float)x * 0.086439f); } /* 0.05 .. 20 */
static inline float syn_index(int x) { return (float)x * 0.08f; }                       /* radians, 0 .. 8 */

/* a wave as drawn (no anti-aliasing), t in turns 0..1, pw 0..1 */
static inline float syn_wave(int w, float t, float pw)
{
    switch (w) {
    case W_SINE: return fm_sin_turns(t);
    case W_TRI: return t < 0.25f ? 4.0f * t : t < 0.75f ? 2.0f - 4.0f * t : 4.0f * t - 4.0f;
    case W_SAW: return t < 0.5f ? 2.0f * t : 2.0f * t - 2.0f;
    case W_SQUARE: return t < 0.5f ? 1.0f : -1.0f;
    case W_PULSE: return t < pw ? 1.0f : -1.0f;
    default: return 0.0f;
    }
}

/* the wavetable: five frames, sine, triangle, saw, square, a narrow pulse; the position morphs between them */
static inline float syn_wt(float pos, float t)
{
    static const uint8_t FR[5] = {W_SINE, W_TRI, W_SAW, W_SQUARE, W_PULSE};
    float f = fm_clampf(pos, 0.0f, 100.0f) * 0.04f, a;
    int i = (int)f;
    if (i >= 4)
        return syn_wave(W_PULSE, t, 0.12f);
    a = f - (float)i;
    return syn_wave(FR[i], t, 0.12f) * (1.0f - a) + syn_wave(FR[i + 1], t, 0.12f) * a;
}
