#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""Writes THEORY-1's long host-simulator scripts: basic.txt (every quiz at every level answered, a few mistakes, the
lab, the settings across a reboot) and texts_fit.txt (every page in every language, clef and naming: no text off
the screen). Run it after adding a topic, a quiz type, a clef or a language:  python3 app/tests/gen_tests.py"""
from pathlib import Path

NQT, NLANGS = 12, 5
TOPIC_ITEMS = [9, 17, 13, 9, 21, 13, 9, 15, 13]   # basics, staff, intervals, scales, modes, chords, harmony, keys, world
HERE = Path(__file__).resolve().parent


def basic():
    L = ["# THEORY-1 in the host simulator (app/tests/gen_tests.py): every quiz answered right scores 100, a wrong",
         "# note costs points, the lab names chords, the settings survive a reboot.",
         "wait 900", "expect screen 0", "clefs 2", "turn SELECT 1", "tap SEL", "wait 50", "expect screen 3"]
    for lv in range(3):
        if lv:
            L += ["turn K1 1", "wait 30"]
        L += [f"expect level {lv}"]
        for t in range(NQT):
            L += [f"seed {t * 7 + lv * 101 + 3}", f"quizof {t}", "tap SEL", "wait 200", "expect screen 4"]
            for _ in range(10):
                L += ["solve", "wait 1400", "expect done 1", "wait 2000"]
            L += ["expect screen 5", "expect score 100"]
            if t == 0 and lv == 0:
                L += ["expect best0 100"]
            L += ["tap HOME", "wait 50", "expect screen 3"]
    L += ["seed 5", "quizof 3", "tap SEL", "wait 200", "wrong", "wait 300", "expect errors 1", "solve", "wait 1400",
          "expect done 1", "expect score 8", "wait 2000", "expect qnum 2",
          "tap HOME", "wait 50", "seed 9", "quizof 8", "tap SEL", "wait 300", "tap EDIT", "wait 50", "wrong", "wait 200",
          "expect done 2", "expect score 0", "wait 3600", "expect qnum 2",
          "tap HOME", "wait 50", "seed 21", "quizof 10", "tap SEL", "wait 200", "wrong", "wait 300", "expect errors 1",
          "solve", "wait 600", "expect done 1", "expect score 8",
          "tap HOME", "tap HOME", "wait 50", "expect screen 0",
          "turn SELECT 1", "tap SEL", "wait 50", "expect screen 6",
          "key w4 down", "wait 30", "key w6 down", "wait 30", "key w8 down", "wait 30", "expect voices 3",
          "key w4 up", "key w11 down", "wait 30", "key w6 up", "key w8 up", "key w11 up", "wait 30",
          "turn SELECT 1", "key w8 down", "key w10 down", "key w12 down", "wait 30", "key b4 down", "wait 30",
          "key w8 up", "key w10 up", "key w12 up", "key b4 up", "wait 1500",
          "midi USB 90 3E 64", "midi USB 90 41 64", "midi USB 90 45 64", "midi USB 90 48 64", "wait 50",
          "expect voices 4", "midi USB 80 3E 00", "midi USB 80 41 00", "midi USB 80 45 00", "midi USB 80 48 00",
          "wait 600", "tap HOME", "wait 50",
          "turn SELECT 2", "tap SEL", "wait 50", "expect screen 7", "tap SEL", "wait 30", "expect lang 1",
          "expect dirty 1", "tap HOME", "wait 50", "expect dirty 0", "reboot", "wait 900", "expect lang 1",
          "expect best0 100"]
    return L


def texts_fit():
    L = ["# Every lesson page (the staff: every clef, every note, every accidental; Harmony: every scale on every root,",
         "# triads and sevenths), quiz type and level, the lab and the settings, in every language and two ways of",
         "# naming notes: no text may run off the screen (app_host_fails). Written by app/tests/gen_tests.py.",
         "wait 900", "clefs 2"]
    for lang in range(NLANGS):
        for names in (0, 3):
            L += [f"lang {lang}", f"names {names}", "wait 30", "tap SEL", "wait 30", "tap SEL", "wait 30"]
            for t, n in enumerate(TOPIC_ITEMS):
                if t == 1:                     # the staff: 7 clefs x 17 notes x 3 accidentals
                    for c in range(7):
                        L += ["turn SELECT -20", "wait 20"]
                        for i in range(n):
                            L += ["turn K2 1", "wait 15", "turn K2 1", "wait 15", "turn K2 1", "turn SELECT 1", "wait 15"]
                        L += ["turn K1 1", "wait 20"]
                else:
                    for i in range(n):
                        L += ["turn SELECT 1", "wait 20"]
                        if t in (2, 3, 4, 5):
                            L += ["turn K1 5", "wait 20"]
                        if t == 6:
                            for r in range(17):
                                L += ["turn K1 1", "turn K2 1", "wait 20", "turn K3 1", "wait 20", "turn K3 1"]
                L += ["turn ALGO 1", "wait 20"]
            L += ["tap HOME", "tap HOME", "wait 30", "turn SELECT 1", "tap SEL", "wait 30"]
            for qt in range(NQT):
                for lv in range(3):
                    L += [f"seed {qt * 13 + lv * 7 + lang + names + 1}", f"quizof {qt}", "tap SEL", "wait 100", "wrong",
                          "wait 200", "solve", "wait 1300", "tap HOME", "wait 30", "turn K1 1"]
            L += ["tap HOME", "wait 30", "turn SELECT 1", "tap SEL", "wait 30"]
            for sc in range(17):
                L += ["turn SELECT 1", "turn K1 3", "key w4 down", "key w6 down", "key w8 down", "key b2 down", "wait 20",
                      "key w4 up", "key w6 up", "key w8 up", "key b2 up", "key b4 down", "wait 20", "key b4 up", "wait 20"]
            L += ["tap HOME", "wait 30", "turn SELECT 1", "tap SEL", "wait 30", "turn SELECT 8", "wait 30", "tap HOME",
                  "wait 30", "turn SELECT 1", "wait 30"]
    return L


if __name__ == "__main__":
    (HERE / "basic.txt").write_text("\n".join(basic()) + "\n")
    (HERE / "texts_fit.txt").write_text("\n".join(texts_fit()) + "\n")
