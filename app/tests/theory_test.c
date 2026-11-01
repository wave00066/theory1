/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1's theory engine checked against known answers, on a computer:
 *   cc -std=c99 -Wall -o /tmp/theory_test app/tests/theory_test.c && /tmp/theory_test */
#include <stdio.h>
#include <string.h>
#include "../theory.c"

static int fails, checks;

static void check(const char *what, const char *got, const char *want)
{
    checks++;
    if (strcmp(got, want)) {
        fails++;
        printf("FAIL %s: got \"%s\", want \"%s\"\n", what, got, want);
    }
}

static nt_t N(const char *s)            /* "F#" -> note */
{
    static const char L[] = "CDEFGAB";
    nt_t n = {0, 0};
    n.letter = (int8_t)(strchr(L, s[0]) - L);
    for (s++; *s; s++)
        n.acc += *s == '#' ? 1 : *s == 'b' ? -1 : 0;
    return n;
}

static void scale_en(const scale_t *s, nt_t root, char *b)
{
    int i;
    char t[8];
    b[0] = 0;
    for (i = 0; i < s->n; i++) {
        if (i)
            strcat(b, " ");
        note_name_s(scale_note(s, root, i), t, SYS_LETTERS);
        strcat(b, t);
    }
}

static void chord_en(int type, nt_t root, char *b)
{
    int i;
    char t[8];
    b[0] = 0;
    for (i = 0; i < CHORDS[type].n; i++) {
        if (i)
            strcat(b, " ");
        note_name_s(chord_note(type, root, i), t, SYS_LETTERS);
        strcat(b, t);
    }
}

static void degrees_en(const scale_t *s, nt_t root, int n, char *b)
{
    int d;
    char t[16];
    b[0] = 0;
    for (d = 0; d < 7; d++) {
        int ty = degree_chord(s, d, n);
        if (d)
            strcat(b, " ");
        if (ty < 0) {
            strcat(b, "?");
            continue;
        }
        chord_symbol(ty, scale_note(s, root, d), t);
        strcat(b, t);
    }
}

static void romans(const scale_t *s, int n, char *b)
{
    int d;
    char t[16], *p;
    b[0] = 0;
    for (d = 0; d < 7; d++) {
        roman(d, degree_chord(s, d, n), t);
        for (p = t; *p; p++)              /* the circles as ASCII for the check */
            if (*p == 1)
                *p = 'o';
            else if (*p == 2)
                *p = '%';
        if (d)
            strcat(b, " ");
        strcat(b, t);
    }
}

int main(void)
{
    char b[128], t[32];
    int i, r, m;
    scale_t ms;

    /* scales */
    scale_en(&SCALES[SC_MAJOR], N("C"), b), check("C major", b, "C D E F G A B");
    scale_en(&SCALES[SC_MAJOR], N("F"), b), check("F major", b, "F G A Bb C D E");
    scale_en(&SCALES[SC_MAJOR], N("D"), b), check("D major", b, "D E F# G A B C#");
    scale_en(&SCALES[SC_MAJOR], N("Gb"), b), check("Gb major", b, "Gb Ab Bb Cb Db Eb F");
    scale_en(&SCALES[SC_MAJOR], N("C#"), b), check("C# major", b, "C# D# E# F# G# A# B#");
    scale_en(&SCALES[SC_NATMINOR], N("A"), b), check("A minor", b, "A B C D E F G");
    scale_en(&SCALES[SC_NATMINOR], N("D"), b), check("D minor", b, "D E F G A Bb C");
    scale_en(&SCALES[SC_HARMMINOR], N("A"), b), check("A harmonic", b, "A B C D E F G#");
    scale_en(&SCALES[SC_HARMMINOR], N("G#"), b), check("G# harmonic", b, "G# A# B C# D# E F##");
    scale_en(&SCALES[SC_MELMINOR], N("C"), b), check("C melodic", b, "C D Eb F G A B");
    scale_en(&SCALES[SC_PENTA_MAJ], N("G"), b), check("G penta", b, "G A B D E");
    scale_en(&SCALES[SC_PENTA_MIN], N("E"), b), check("E minor penta", b, "E G A B D");
    scale_en(&SCALES[SC_BLUES], N("A"), b), check("A blues", b, "A C D Eb E G");
    scale_en(&SCALES[SC_WHOLE], N("C"), b), check("C whole tone", b, "C D E F# G# Bb");
    scale_en(&SCALES[SC_CHROMATIC], N("C"), b), check("C chromatic", b, "C C# D D# E F F# G G# A A# B");
    scale_steps(&SCALES[SC_MAJOR], b), check("major steps", b, "T  T  S  T  T  T  S");
    scale_steps(&SCALES[SC_HARMMINOR], b), check("harmonic steps", b, "T  S  T  T  S  T+S  S");
    degree_labels(&SCALES[SC_NATMINOR], b), check("minor degrees", b, "1 2 b3 4 5 b6 b7");

    /* modes */
    mode_scale(1, &ms), scale_en(&ms, N("D"), b), check("D dorian", b, "D E F G A B C");
    mode_scale(1, &ms), scale_en(&ms, N("E"), b), check("E dorian", b, "E F# G A B C# D");
    mode_scale(2, &ms), scale_en(&ms, N("E"), b), check("E phrygian", b, "E F G A B C D");
    mode_scale(3, &ms), scale_en(&ms, N("F"), b), check("F lydian", b, "F G A B C D E");
    mode_scale(3, &ms), scale_en(&ms, N("C"), b), check("C lydian", b, "C D E F# G A B");
    mode_scale(4, &ms), scale_en(&ms, N("G"), b), check("G mixolydian", b, "G A B C D E F");
    mode_scale(6, &ms), scale_en(&ms, N("B"), b), check("B locrian", b, "B C D E F G A");
    mode_scale(1, &ms), degree_labels(&ms, b), check("dorian degrees", b, "1 2 b3 4 5 6 b7");
    mode_scale(6, &ms), degree_labels(&ms, b), check("locrian degrees", b, "1 b2 b3 4 b5 b6 b7");
    note_name_s(mode_parent(N("D"), 1), t, SYS_LETTERS), check("D dorian parent", t, "C");
    note_name_s(mode_parent(N("F#"), 2), t, SYS_LETTERS), check("F# phrygian parent", t, "D");
    note_name_s(mode_parent(N("Bb"), 3), t, SYS_LETTERS), check("Bb lydian parent", t, "F");
    note_name_s(mode_parent(N("A"), 5), t, SYS_LETTERS), check("A aeolian parent", t, "C");
    /* every mode, from every root, is its parent major's notes */
    for (m = 0; m < 7; m++)
        for (r = 0; r < NROOTS; r++) {
            int k;
            uint32_t a = 0, p = 0;
            mode_scale(m, &ms);
            for (k = 0; k < 7; k++) {
                a |= 1u << pc_of(scale_note(&ms, ROOTS[r], k));
                p |= 1u << pc_of(scale_note(&SCALES[SC_MAJOR], mode_parent(ROOTS[r], m), k));
            }
            checks++;
            if (a != p)
                fails++, printf("FAIL mode %d root %d not its parent's notes\n", m, r);
        }

    /* chords */
    chord_en(CH_MAJ, N("C"), b), check("C", b, "C E G");
    chord_en(CH_MIN, N("F#"), b), check("F#m", b, "F# A C#");
    chord_en(CH_DIM, N("B"), b), check("Bdim", b, "B D F");
    chord_en(CH_AUG, N("C"), b), check("Caug", b, "C E G#");
    chord_en(CH_SUS4, N("D"), b), check("Dsus4", b, "D G A");
    chord_en(CH_SUS2, N("Eb"), b), check("Ebsus2", b, "Eb F Bb");
    chord_en(CH_DOM7, N("G"), b), check("G7", b, "G B D F");
    chord_en(CH_MAJ7, N("Eb"), b), check("Ebmaj7", b, "Eb G Bb D");
    chord_en(CH_HALFDIM, N("B"), b), check("Bm7b5", b, "B D F A");
    chord_en(CH_DIM7, N("G#"), b), check("G#dim7", b, "G# B D F");
    chord_en(CH_DIM7, N("C"), b), check("Cdim7", b, "C Eb Gb Bbb");
    chord_symbol(CH_MIN7, N("Bb"), t), check("Bbm7 symbol", t, "Bbm7");

    /* chords on the degrees */
    degrees_en(&SCALES[SC_MAJOR], N("C"), 3, b), check("C major triads", b, "C Dm Em F G Am Bdim");
    degrees_en(&SCALES[SC_MAJOR], N("C"), 4, b), check("C major 7ths", b, "Cmaj7 Dm7 Em7 Fmaj7 G7 Am7 Bm7b5");
    degrees_en(&SCALES[SC_MAJOR], N("Bb"), 3, b), check("Bb major triads", b, "Bb Cm Dm Eb F Gm Adim");
    degrees_en(&SCALES[SC_NATMINOR], N("A"), 3, b), check("A minor triads", b, "Am Bdim C Dm Em F G");
    degrees_en(&SCALES[SC_HARMMINOR], N("A"), 3, b), check("A harmonic triads", b, "Am Bdim Caug Dm E F G#dim");
    degrees_en(&SCALES[SC_HARMMINOR], N("A"), 4, b),
        check("A harmonic 7ths", b, "Am(maj7) Bm7b5 Cmaj7#5 Dm7 E7 Fmaj7 G#dim7");
    degrees_en(&SCALES[SC_MELMINOR], N("C"), 3, b), check("C melodic triads", b, "Cm Dm Ebaug F G Adim Bdim");
    romans(&SCALES[SC_MAJOR], 3, b), check("major romans", b, "I ii iii IV V vi viio");
    romans(&SCALES[SC_MAJOR], 4, b), check("major 7th romans", b, "Imaj7 ii7 iii7 IVmaj7 V7 vi7 vii%7");
    romans(&SCALES[SC_HARMMINOR], 3, b), check("harmonic romans", b, "i iio III+ iv V VI viio");
    /* every heptatonic scale and mode has a chord on every degree */
    for (i = 0; i < 4; i++)
        for (m = 0; m < 7; m++) {
            checks += 2;
            if (degree_chord(&SCALES[i], m, 3) < 0 || degree_chord(&SCALES[i], m, 4) < 0)
                fails++, printf("FAIL scale %d degree %d has no chord\n", i, m);
        }
    check("degree name 7 minor", degree_name(&SCALES[SC_NATMINOR], 6), "Sottotonica");
    check("degree name 7 harmonic", degree_name(&SCALES[SC_HARMMINOR], 6), "Sensibile");

    /* intervals */
    note_name_s(interval_top(N("D"), 4), t, SYS_LETTERS), check("M3 above D", t, "F#");
    note_name_s(interval_top(N("E"), 3), t, SYS_LETTERS), check("m3 above E", t, "G");
    note_name_s(interval_top(N("F"), 6), t, SYS_LETTERS), check("tritone above F", t, "B");
    note_name_s(interval_top(N("Bb"), 7), t, SYS_LETTERS), check("P5 above Bb", t, "F");
    note_name_s(interval_top(N("A"), 12), t, SYS_LETTERS), check("octave above A", t, "A");

    /* key signatures: the table agrees with the scales, and the relative minors */
    for (i = 0; i < NKEYSIG; i++) {
        checks++;
        if (key_count(KEYSIGS[i].root) != KEYSIGS[i].count)
            fails++, printf("FAIL key %d: %d accidentals in the scale, %d in the table\n", i,
                            key_count(KEYSIGS[i].root), KEYSIGS[i].count);
    }
    th_names = NAMES_LETTERS;
    key_alterations(3, b), check("3 sharps", b, "F# C# G#");
    key_alterations(-4, b), check("4 flats", b, "Bb Eb Ab Db");
    th_names = NAMES_GERMAN;
    key_alterations(-4, b), check("4 flats German", b, "B Es As Des");
    key_alterations(6, b), check("6 sharps German", b, "Fis Cis Gis Dis Ais Eis");
    note_name(N("B"), t), check("German B", t, "H");
    note_name(N("Cb"), t), check("German Cb", t, "Ces");
    scale_en(&SCALES[SC_HARMMINOR], N("G#"), b);
    {   /* (scale_en writes letters: the German names by hand) */
        int k;
        b[0] = 0;
        for (k = 0; k < 7; k++) {
            if (k)
                strcat(b, " ");
            note_name(scale_note(&SCALES[SC_HARMMINOR], N("G#"), k), t);
            strcat(b, t);
        }
        check("G# harmonic German", b, "Gis Ais H Cis Dis E Fisis");
    }
    chord_symbol(CH_MIN7, N("B"), t), check("German Hm7", t, "Hm7");
    chord_symbol(CH_MAJ, N("Bb"), t), check("German B (Bb)", t, "B");
    th_names = NAMES_BOTH;
    note_full(N("F#"), t), check("both F#", t, "Fa# (F#)");
    chord_name(CH_MIN, N("D"), b), check("both Dm name", b, "Re (D) minore");
    th_ui = L_DE;
    chord_name(CH_MIN, N("D"), b), check("German UI Dm name", b, "Re (D) Moll");
    scale_steps(&SCALES[SC_MAJOR], b), check("German steps", b, "G  G  H  G  G  G  H");
    check("German degree 5", degree_name(&SCALES[SC_MAJOR], 4), "Dominante");
    th_ui = L_ES;
    check("Spanish degree 7", degree_name(&SCALES[SC_MAJOR], 6), "Sensible");
    th_ui = L_FR;
    note_full(N("D"), t), check("French D", t, "Ré (D)");
    scale_steps(&SCALES[SC_HARMMINOR], b), check("French steps", b, "T  dT  T  T  dT  T+dT  dT");
    check("French degree 6", degree_name(&SCALES[SC_MAJOR], 5), "Sus-dominante");
    chord_name(CH_DIM, N("B"), b), check("French Bdim name", b, "Si (B) diminué");
    th_ui = L_IT;
    th_names = NAMES_SOLFEGE;
    key_alterations(2, b), check("2 diesis", b, "Fa# Do#");
    note_name_s(relative_minor(N("C")), t, SYS_LETTERS), check("rel C", t, "A");
    note_name_s(relative_minor(N("Eb")), t, SYS_LETTERS), check("rel Eb", t, "C");
    note_name_s(relative_minor(N("F#")), t, SYS_LETTERS), check("rel F#", t, "D#");
    note_name(N("F#"), t), check("Italian F#", t, "Fa#");
    note_name(N("Bb"), t), check("Italian Bb", t, "Sib");

    /* recognition: every chord in every root and inversion is found again */
    for (i = 0; i < NCHORDS; i++)
        for (r = 0; r < 12; r++) {
            int k, ty, rp, inv;
            for (k = 0; k < CHORDS[i].n; k++) {
                int bass = (r + CHORDS[i].semi[k]) % 12, ok = chord_find(chord_mask(i, r), bass, &ty, &rp, &inv);
                checks++;
                /* symmetric chords (aug, dim7) and sus2/sus4 read from their bass: the same notes, another name */
                if (!ok || chord_mask(ty, rp) != chord_mask(i, r) || (rp == r && ty == i && inv != k))
                    fails++, printf("FAIL find %s on %d inv %d\n", CHORDS[i].sym, r, k);
            }
        }
    {
        int ty, rp, inv;
        chord_find(1u << 4 | 1u << 7 | 1u << 0, 4, &ty, &rp, &inv);
        checks++;
        if (ty != CH_MAJ || rp != 0 || inv != 1)
            fails++, printf("FAIL E-G-C should be C major, first inversion\n");
    }

    for (r = 0; r < NLANGS; r++) {          /* every name and text, in every language */
        int ok = 1;
        for (i = 0; i < NINTERVALS; i++)
            ok &= INTERVALS[i].name[r] && INTERVALS[i].text[r] && INTERVALS[i].name[r][0];
        for (i = 0; i < NSCALES; i++)
            ok &= SCALES[i].name[r] && SCALES[i].text[r] && SCALES[i].name[r][0];
        for (i = 0; i < NMODES; i++)
            ok &= MODES[i].name[r] && MODES[i].text[r] && MODES[i].name[r][0];
        for (i = 0; i < NCHORDS; i++)
            ok &= CHORDS[i].name[r] && CHORDS[i].text[r] && CHORDS[i].name[r][0];
        for (i = 0; i < 8; i++)
            ok &= DEGREE_NAME[r][i] != 0;
        for (i = 0; i < 3; i++)
            ok &= FN_NAME[r][i] != 0;
        checks++;
        if (!ok)
            fails++, printf("FAIL language %d: a name or text missing\n", r);
    }
    printf("theory: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
