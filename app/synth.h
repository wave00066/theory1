/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* The sound: a small two-operator FM synthesizer, 8 voices (an o2 unit: fm1.toml o2). The main loop sends it
 * events through a ring (one writer, the main loop; one reader, the render), so nothing is shared otherwise. */
#pragma once
#include <stdint.h>

enum { SND_PIANO, SND_EPIANO, SND_ORGAN, SND_BELL, NSOUNDS };

void synth_note_on(int note, int vel);     /* main loop: MIDI note, velocity 1..127 */
void synth_note_off(int note);
void synth_all_off(void);
void synth_sound(int s);                   /* SND_*: the timbre */
void synth_panic(void);                    /* render's side: silence now (before a flash write) */
/* the render: n stereo frames into lr (24-bit), gain the output's scale (MASTER and the 24-bit range) */
void synth_render(int32_t *lr, uint32_t n, float gain);
int synth_active(void);                    /* voices sounding (a test's check) */
/* the SYNTH section: the teaching synthesizer (synthmath.h's parameters) */
void synth_mode(int patch);                /* 1: the keys play the patch; 0: the lessons' sounds */
void synth_set(int p, int v);              /* main loop: a parameter (P_*) */
float synth_lfo(void);                     /* the LFO now, -1..1 (the picture's dot) */
