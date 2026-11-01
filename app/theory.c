/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* The theory engine: note spelling and naming, intervals, scales, modes, chords, chords on the degrees of a scale,
 * key signatures and chord recognition, with their names and explanations in five languages (Italian, English,
 * Spanish, German, French). Pure C (no platform, no floats): part of the app's unity build, and compiled alone on a
 * computer by tests/theory_test.c, which checks every scale, mode and chord in every key.
 *
 * Spelling is real spelling, not just pitch: each degree of a heptatonic scale takes the next letter, so F major has
 * Bb (not A#), G# harmonic minor has F## and D minor's sixth is Bb. A note is a letter (C..B) and an accidental.
 * Text is UTF-8; the app draws its accents (app.c tx). */
#include <stdint.h>

/* the language of the text (a setting): Italian, English, Spanish, German, French */
enum { L_IT, L_EN, L_ES, L_DE, L_FR, NLANGS };
static uint8_t th_ui;
#define TR(a) ((a)[th_ui])

/* how notes are named (a setting): Do with C beside it, Do only, C only, or the German way (H, Fis, Es) */
enum { NAMES_BOTH, NAMES_SOLFEGE, NAMES_LETTERS, NAMES_GERMAN, NNAMES };
static uint8_t th_names;
enum { SYS_SOLFEGE, SYS_LETTERS, SYS_GERMAN };

/* ------------------------------------------------------------------- notes --- */
typedef struct {
    int8_t letter;                      /* 0..6: C D E F G A B */
    int8_t acc;                         /* -2 bb, -1 b, 0, +1 #, +2 ## */
} nt_t;

static const uint8_t LETTER_PC[7] = {0, 2, 4, 5, 7, 9, 11};
static const char *const LETTER_SOL[7] = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si"};
static const char *const LETTER_EN[7] = {"C", "D", "E", "F", "G", "A", "B"};

static int pc_of(nt_t n) { return ((LETTER_PC[n.letter] + n.acc) % 12 + 12) % 12; }

/* the note semis above root, written on the letter letteroff letters above root's */
static nt_t spell(nt_t root, int semis, int letteroff)
{
    nt_t r;
    int target, d;
    r.letter = (int8_t)((root.letter + letteroff) % 7);
    target = (pc_of(root) + semis) % 12;
    d = (target - LETTER_PC[r.letter] + 12) % 12;
    if (d > 6)
        d -= 12;
    r.acc = (int8_t)d;
    return r;
}

static void str_cat(char *d, const char *s)
{
    while (*d)
        d++;
    while (*s)
        *d++ = *s++;
    *d = 0;
}

static const char *acc_text(int acc)
{
    switch (acc) {
    case -2: return "bb";
    case -1: return "b";
    case 1: return "#";
    case 2: return "##";
    default: return "";
    }
}

/* the German names: H for B, B for Bb; -is sharpens, -es flattens (Es, As: no e before -s) */
static void note_german(nt_t n, char *b)
{
    static const char *const L[7] = {"C", "D", "E", "F", "G", "A", "H"};
    int i;
    b[0] = 0;
    if (n.letter == 6 && n.acc == -1) {
        str_cat(b, "B");
        return;
    }
    if (n.letter == 6 && n.acc == -2) {
        str_cat(b, "Heses");
        return;
    }
    str_cat(b, L[n.letter]);
    for (i = 0; i < n.acc; i++)
        str_cat(b, "is");
    for (i = 0; i < -n.acc; i++)
        str_cat(b, (n.letter == 2 || n.letter == 5) && i == 0 ? "s" : "es");
}

static void note_name_s(nt_t n, char *b, int sys)
{
    if (sys == SYS_GERMAN) {
        note_german(n, b);
        return;
    }
    b[0] = 0;
    str_cat(b, sys == SYS_LETTERS ? LETTER_EN[n.letter] : th_ui == L_FR && n.letter == 1 ? "Ré" : LETTER_SOL[n.letter]);
    str_cat(b, acc_text(n.acc));
}
static int names_main(void)
{
    return th_names == NAMES_LETTERS ? SYS_LETTERS : th_names == NAMES_GERMAN ? SYS_GERMAN : SYS_SOLFEGE;
}
/* the note's name: the main one; the other one beside it ("C" with NAMES_BOTH, else ""); both: "Do (C)" */
static void note_name(nt_t n, char *b) { note_name_s(n, b, names_main()); }
static void note_alt(nt_t n, char *b)
{
    b[0] = 0;
    if (th_names == NAMES_BOTH)
        note_name_s(n, b, SYS_LETTERS);
}
static void note_full(nt_t n, char *b)
{
    char t[8];
    note_name(n, b);
    note_alt(n, t);
    if (t[0]) {
        str_cat(b, " (");
        str_cat(b, t);
        str_cat(b, ")");
    }
}

/* a pitch class spelled the usual way when nothing says otherwise (a key played in the lab) */
static nt_t nt_of_pc(int pc)
{
    static const int8_t L[12] = {0, 0, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6}, A[12] = {0, 1, 0, -1, 0, 0, 1, 0, -1, 0, -1, 0};
    nt_t n;
    pc = ((pc % 12) + 12) % 12;
    n.letter = L[pc];
    n.acc = A[pc];
    return n;
}

/* the 17 roots a lesson or a question can start from: every natural, sharp and flat with one accidental that is
 * not a white key (no E#, Fb, B#, Cb), in keyboard order */
#define NROOTS 17
static const nt_t ROOTS[NROOTS] = {{0, 0}, {0, 1}, {1, -1}, {1, 0}, {1, 1}, {2, -1}, {2, 0}, {3, 0}, {3, 1},
                                   {4, -1}, {4, 0}, {4, 1}, {5, -1}, {5, 0}, {5, 1}, {6, -1}, {6, 0}};
/* the roots of the usual keys (no D#, G#, A# major: those need double sharps) */
static const uint8_t COMMON_MAJOR_ROOTS[] = {0, 2, 3, 5, 6, 7, 8, 10, 12, 13, 15, 16};
/* the roots of the usual minor keys (no Db, Gb minor) */
static const uint8_t COMMON_MINOR_ROOTS[] = {0, 1, 3, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16};

/* --------------------------------------------------------------- intervals --- */
typedef struct {
    const char *name[NLANGS];
    const char *abbr;                   /* "3M" */
    uint8_t semis, letters;
    const char *text[NLANGS];
} interval_t;
#define NINTERVALS 13
static const interval_t INTERVALS[NINTERVALS] = {
    {{"Unisono", "Unison", "Unísono", "Prime", "Unisson"}, "1", 0, 0,
     {"La stessa nota due volte. Distanza zero: il punto di partenza di ogni intervallo.",
      "The same note twice. Distance zero: where every interval starts.",
      "La misma nota dos veces. Distancia cero: el punto de partida de todo intervalo.",
      "Zweimal derselbe Ton. Abstand null: der Ausgangspunkt jedes Intervalls.", "La même note deux fois. Distance zéro : le point de départ de tout intervalle."}},
    {{"2a minore", "Minor 2nd", "Segunda menor", "Kleine Sekunde", "Seconde mineure"}, "2m", 1, 1,
     {"Un semitono: due tasti vicini. Suona teso, come la musica dello squalo.",
      "One semitone: two neighbouring keys. It sounds tense, like the shark music.",
      "Un semitono: dos teclas vecinas. Suena tenso, como la música del tiburón.",
      "Ein Halbton: zwei benachbarte Tasten. Klingt gespannt, wie die Haifisch-Musik.", "Un demi-ton : deux touches voisines. Son tendu, comme la musique du requin."}},
    {{"2a maggiore", "Major 2nd", "Segunda mayor", "Große Sekunde", "Seconde majeure"}, "2M", 2, 1,
     {"Un tono (2 semitoni). È il passo più comune nelle scale.",
      "A whole tone (2 semitones). The most common step in scales.",
      "Un tono (2 semitonos). Es el paso más común en las escalas.",
      "Ein Ganzton (2 Halbtöne). Der häufigste Schritt in Tonleitern.", "Un ton (2 demi-tons). Le pas le plus courant dans les gammes."}},
    {{"3a minore", "Minor 3rd", "Tercera menor", "Kleine Terz", "Tierce mineure"}, "3m", 3, 2,
     {"3 semitoni. Dà il colore triste o malinconico agli accordi minori.",
      "3 semitones. It gives minor chords their sad, wistful colour.",
      "3 semitonos. Da el color triste o melancólico a los acordes menores.",
      "3 Halbtöne. Sie gibt Moll-Akkorden ihre traurige Farbe.", "3 demi-tons. Elle donne aux accords mineurs leur couleur triste."}},
    {{"3a maggiore", "Major 3rd", "Tercera mayor", "Große Terz", "Tierce majeure"}, "3M", 4, 2,
     {"4 semitoni. Dà il colore luminoso e allegro agli accordi maggiori.",
      "4 semitones. It gives major chords their bright, happy colour.",
      "4 semitonos. Da el color luminoso y alegre a los acordes mayores.",
      "4 Halbtöne. Sie gibt Dur-Akkorden ihre helle, fröhliche Farbe.", "4 demi-tons. Elle donne aux accords majeurs leur couleur claire et joyeuse."}},
    {{"4a giusta", "Perfect 4th", "Cuarta justa", "Reine Quarte", "Quarte juste"}, "4", 5, 3,
     {"5 semitoni. Suona aperta e stabile: è l'inizio di molti inni.",
      "5 semitones. Open and stable: many anthems begin with it.",
      "5 semitonos. Suena abierta y estable: así empiezan muchos himnos.",
      "5 Halbtöne. Offen und stabil: so beginnen viele Hymnen.", "5 demi-tons. Ouverte et stable : beaucoup d'hymnes commencent ainsi."}},
    {{"Tritono", "Tritone", "Tritono", "Tritonus", "Triton"}, "4+/5-", 6, 3,
     {"6 semitoni: 4a aumentata o 5a diminuita. Il suono più instabile: vuole risolvere.",
      "6 semitones: augmented 4th or diminished 5th. The most unstable sound: it wants to resolve.",
      "6 semitonos: cuarta aumentada o quinta disminuida. El sonido más inestable: quiere resolver.",
      "6 Halbtöne: übermäßige Quarte oder verminderte Quinte. Der unruhigste Klang: er will sich auflösen.", "6 demi-tons : quarte augmentée ou quinte diminuée. Le son le plus instable : il veut se résoudre."}},
    {{"5a giusta", "Perfect 5th", "Quinta justa", "Reine Quinte", "Quinte juste"}, "5", 7, 4,
     {"7 semitoni. Stabile e potente: è la base del 'power chord' e del circolo delle quinte.",
      "7 semitones. Stable and strong: the base of the power chord and of the circle of fifths.",
      "7 semitonos. Estable y potente: la base del 'power chord' y del círculo de quintas.",
      "7 Halbtöne. Stabil und kräftig: Grundlage des Powerchords und des Quintenzirkels.", "7 demi-tons. Stable et puissante : la base du 'power chord' et du cycle des quintes."}},
    {{"6a minore", "Minor 6th", "Sexta menor", "Kleine Sexte", "Sixte mineure"}, "6m", 8, 5,
     {"8 semitoni. Dolce e un po' malinconica.", "8 semitones. Sweet and a little sad.",
      "8 semitonos. Dulce y un poco melancólica.", "8 Halbtöne. Weich und ein wenig wehmütig.", "8 demi-tons. Douce et un peu triste."}},
    {{"6a maggiore", "Major 6th", "Sexta mayor", "Große Sexte", "Sixte majeure"}, "6M", 9, 5,
     {"9 semitoni. Calda e cantabile.", "9 semitones. Warm and singing.", "9 semitonos. Cálida y cantable.",
      "9 Halbtöne. Warm und sanglich.", "9 demi-tons. Chaude et chantante."}},
    {{"7a minore", "Minor 7th", "Séptima menor", "Kleine Septime", "Septième mineure"}, "7m", 10, 6,
     {"10 semitoni. È la nota che rende 'di dominante' l'accordo di settima (es. G7).",
      "10 semitones. The note that makes a dominant seventh chord (e.g. G7).",
      "10 semitonos. Es la nota que hace 'de dominante' el acorde de séptima (ej. G7).",
      "10 Halbtöne. Der Ton, der einen Dominantseptakkord macht (z. B. G7).", "10 demi-tons. La note qui fait un accord de septième de dominante (ex. G7)."}},
    {{"7a maggiore", "Major 7th", "Séptima mayor", "Große Septime", "Septième majeure"}, "7M", 11, 6,
     {"11 semitoni. Un semitono sotto l'ottava: sognante, tipica del jazz (maj7).",
      "11 semitones. A semitone below the octave: dreamy, typical of jazz (maj7).",
      "11 semitonos. Un semitono bajo la octava: soñadora, típica del jazz (maj7).",
      "11 Halbtöne. Ein Halbton unter der Oktave: verträumt, typisch für Jazz (maj7).", "11 demi-tons. Un demi-ton sous l'octave : rêveuse, typique du jazz (maj7)."}},
    {{"Ottava", "Octave", "Octava", "Oktave", "Octave"}, "8", 12, 7,
     {"12 semitoni. La stessa nota più in alto: ha lo stesso nome della prima.",
      "12 semitones. The same note higher up: it has the same name.",
      "12 semitonos. La misma nota más aguda: tiene el mismo nombre.",
      "12 Halbtöne. Derselbe Ton höher: er hat denselben Namen.", "12 demi-tons. La même note plus haut : elle a le même nom."}},
};
static nt_t interval_top(nt_t root, int i) { return spell(root, INTERVALS[i].semis % 12, INTERVALS[i].letters % 7); }

/* ----------------------------------------------------------------- scales --- */
#define SCALE_MAX 12
typedef struct {
    const char *name[NLANGS];           /* as it follows a note: "Re maggiore", "D natural minor" */
    uint8_t n;
    uint8_t semi[SCALE_MAX];
    uint8_t let[SCALE_MAX];             /* each degree's letter above the root's */
    const char *text[NLANGS];
} scale_t;
enum { SC_MAJOR, SC_NATMINOR, SC_HARMMINOR, SC_MELMINOR, SC_PENTA_MAJ, SC_PENTA_MIN, SC_BLUES, SC_WHOLE,
       SC_CHROMATIC, NSCALES };
static const scale_t SCALES[NSCALES] = {
    {{"maggiore", "major", "mayor", "Dur", "majeur"}, 7, {0, 2, 4, 5, 7, 9, 11}, {0, 1, 2, 3, 4, 5, 6},
     {"La scala di base: T T S T T T S. Su Do usa solo i tasti bianchi. Suono luminoso, 'allegro'.",
      "The basic scale: T T S T T T S. On C it uses only the white keys. Bright and 'happy'.",
      "La escala básica: T T S T T T S. Sobre Do usa solo teclas blancas. Sonido luminoso, 'alegre'.",
      "Die Grundtonleiter: G G H G G G H. Auf C nur weiße Tasten. Heller, 'fröhlicher' Klang.", "La gamme de base : T T dT T T T dT. Sur Do, seulement les touches blanches. Son clair, 'joyeux'."}},
    {{"minore naturale", "natural minor", "menor natural", "Moll (natürlich)", "mineur naturel"}, 7, {0, 2, 3, 5, 7, 8, 10},
     {0, 1, 2, 3, 4, 5, 6},
     {"T S T T S T T. Ha la 3a, 6a e 7a minori. La minore ha le stesse note di Do maggiore: è la sua relativa.",
      "T S T T S T T. Minor 3rd, 6th and 7th. A minor has the notes of C major: it is its relative.",
      "T S T T S T T. Tercera, sexta y séptima menores. La menor tiene las notas de Do mayor: es su relativa.",
      "G H G G H G G. Kleine Terz, Sexte und Septime. a-Moll hat die Töne von C-Dur: ihre Paralleltonart.", "T dT T T dT T T. Tierce, sixte et septième mineures. La mineur a les notes de Do majeur : c'est sa relative."}},
    {{"minore armonica", "harmonic minor", "menor armónica", "Moll (harmonisch)", "mineur harmonique"}, 7, {0, 2, 3, 5, 7, 8, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"Come la naturale ma con la 7a alzata: la 'sensibile' porta alla tonica. Tra 6a e 7a c'è un salto di 3 semitoni.",
      "Like the natural minor with a raised 7th: the leading tone pulls to the tonic. 6th to 7th is 3 semitones.",
      "Como la natural con la séptima elevada: la 'sensible' lleva a la tónica. De la 6.ª a la 7.ª hay 3 semitonos.",
      "Wie natürliches Moll mit erhöhter 7. Stufe: der Leitton führt zum Grundton. Von 6 nach 7: 3 Halbtöne.", "Comme le mineur naturel avec la 7e haussée : la sensible mène à la tonique. De la 6e à la 7e : 3 demi-tons."}},
    {{"minore melodica", "melodic minor", "menor melódica", "Moll (melodisch)", "mineur mélodique"}, 7, {0, 2, 3, 5, 7, 9, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"Salendo alza 6a e 7a (scendendo si usa spesso la naturale). Nel jazz è uguale in salita e discesa.",
      "Going up it raises the 6th and 7th (going down, often the natural minor). In jazz it is the same both ways.",
      "Al subir eleva la 6.ª y la 7.ª (al bajar se usa a menudo la natural). En jazz es igual en ambos sentidos.",
      "Aufwärts mit erhöhter 6. und 7. Stufe (abwärts oft natürlich). Im Jazz in beide Richtungen gleich.", "En montant, 6e et 7e haussées (en descendant, souvent le naturel). En jazz, pareil dans les deux sens."}},
    {{"pentatonica maggiore", "major pentatonic", "pentatónica mayor", "Dur-Pentatonik", "pentatonique majeure"}, 5, {0, 2, 4, 7, 9},
     {0, 1, 2, 4, 5},
     {"5 note: la maggiore senza 4a e 7a. Niente semitoni, quindi nessuna nota 'stona': perfetta per improvvisare.",
      "5 notes: the major scale without the 4th and 7th. No semitones, so no note clashes: perfect to improvise.",
      "5 notas: la mayor sin 4.ª ni 7.ª. Sin semitonos, ninguna nota choca: perfecta para improvisar.",
      "5 Töne: Dur ohne 4. und 7. Stufe. Keine Halbtöne, nichts reibt: ideal zum Improvisieren.", "5 notes : la gamme majeure sans 4e ni 7e. Pas de demi-tons, aucune note ne frotte : idéale pour improviser."}},
    {{"pentatonica minore", "minor pentatonic", "pentatónica menor", "Moll-Pentatonik", "pentatonique mineure"}, 5, {0, 3, 5, 7, 10},
     {0, 2, 3, 4, 6},
     {"5 note: 1 b3 4 5 b7. La scala del rock e del blues. Ha le note della pentatonica maggiore relativa.",
      "5 notes: 1 b3 4 5 b7. The scale of rock and blues. Same notes as its relative major pentatonic.",
      "5 notas: 1 b3 4 5 b7. La escala del rock y del blues. Mismas notas que su pentatónica mayor relativa.",
      "5 Töne: 1 b3 4 5 b7. Die Tonleiter von Rock und Blues. Dieselben Töne wie die parallele Dur-Pentatonik.", "5 notes : 1 b3 4 5 b7. La gamme du rock et du blues. Mêmes notes que sa pentatonique majeure relative."}},
    {{"blues", "blues", "blues", "Blues", "blues"}, 6, {0, 3, 5, 6, 7, 10}, {0, 2, 3, 4, 4, 6},
     {"La pentatonica minore più la 'blue note': la 5a diminuita. Quella nota in più dà il sapore blues.",
      "The minor pentatonic plus the 'blue note', the flat 5th. That extra note gives the blues flavour.",
      "La pentatónica menor más la 'blue note', la quinta disminuida. Esa nota extra da el sabor blues.",
      "Moll-Pentatonik plus 'Blue Note', die verminderte Quinte. Dieser Ton gibt den Blues-Geschmack.", "La pentatonique mineure plus la 'blue note', la quinte diminuée. Cette note en plus donne le goût du blues."}},
    {{"esatona", "whole tone", "hexátona", "Ganztonleiter", "par tons"}, 6, {0, 2, 4, 6, 8, 10}, {0, 1, 2, 3, 4, 6},
     {"Solo toni interi (6 note). Senza centro tonale: suona sospesa, da sogno. Usata da Debussy.",
      "Only whole tones (6 notes). No tonal centre: floating, dreamlike. Loved by Debussy.",
      "Solo tonos enteros (6 notas). Sin centro tonal: suena suspendida, de sueño. La usaba Debussy.",
      "Nur Ganztöne (6 Töne). Ohne tonales Zentrum: schwebend, traumhaft. Von Debussy geliebt.", "Que des tons entiers (6 notes). Sans centre tonal : flottante, rêveuse. Chère à Debussy."}},
    {{"cromatica", "chromatic", "cromática", "chromatisch", "chromatique"}, 12, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
     {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6},
     {"Tutti i 12 semitoni dell'ottava. Non è una tonalità: è il materiale di tutte le scale.",
      "All 12 semitones of the octave. Not a key: the raw material of every scale.",
      "Los 12 semitonos de la octava. No es una tonalidad: es el material de todas las escalas.",
      "Alle 12 Halbtöne der Oktave. Keine Tonart: das Material aller Tonleitern.", "Les 12 demi-tons de l'octave. Pas une tonalité : le matériau de toutes les gammes."}},
};

static nt_t scale_note(const scale_t *s, nt_t root, int i)
{
    return spell(root, s->semi[i % s->n], s->let[i % s->n]);
}

/* the scale's steps: "T  T  S  T  T  T  S" (German: G for Ganzton, H for Halbton; French: T and dT, demi-ton) */
static void scale_steps(const scale_t *s, char *b)
{
    int i, de = th_ui == L_DE, fr = th_ui == L_FR;
    b[0] = 0;
    for (i = 0; i < s->n; i++) {
        int next = i + 1 < s->n ? s->semi[i + 1] : 12, d = next - s->semi[i];
        if (i)
            str_cat(b, "  ");
        str_cat(b, d == 1 ? (de ? "H" : fr ? "dT" : "S") : d == 2 ? (de ? "G" : "T") :
                   d == 3 ? (de ? "G+H" : fr ? "T+dT" : "T+S") : "?");
    }
}

/* ------------------------------------------------------------------- modes --- */
#define NMODES 7
typedef struct {
    const char *name[NLANGS];
    uint8_t ch;                         /* its characteristic degree (0-based) */
    const char *text[NLANGS];
} mode_t_;
static const mode_t_ MODES[NMODES] = {
    {{"ionico", "Ionian", "jónico", "Ionisch", "ionien"}, 3,
     {"È la scala maggiore. Luminoso e stabile.", "It is the major scale. Bright and stable.",
      "Es la escala mayor. Luminoso y estable.", "Das ist die Dur-Tonleiter. Hell und stabil.", "C'est la gamme majeure. Clair et stable."}},
    {{"dorico", "Dorian", "dórico", "Dorisch", "dorien"}, 5,
     {"Minore, ma con la 6a maggiore: un minore 'sereno'. Funk, jazz, musica celtica.",
      "Minor, but with a major 6th: a 'calm' minor. Funk, jazz, Celtic music.",
      "Menor, pero con la sexta mayor: un menor 'sereno'. Funk, jazz, música celta.",
      "Moll, aber mit großer Sexte: ein 'gelassenes' Moll. Funk, Jazz, keltische Musik.", "Mineur, mais avec la sixte majeure : un mineur 'serein'. Funk, jazz, musique celtique."}},
    {{"frigio", "Phrygian", "frigio", "Phrygisch", "phrygien"}, 1,
     {"Minore con la 2a minore: suono spagnolo, flamenco, metal.",
      "Minor with a minor 2nd: a Spanish sound, flamenco, metal.",
      "Menor con la segunda menor: sonido español, flamenco, metal.",
      "Moll mit kleiner Sekunde: spanischer Klang, Flamenco, Metal.", "Mineur avec la seconde mineure : son espagnol, flamenco, metal."}},
    {{"lidio", "Lydian", "lidio", "Lydisch", "lydien"}, 3,
     {"Maggiore con la 4a aumentata: sognante, magico. Tipico delle colonne sonore.",
      "Major with a raised 4th: dreamy, magical. A film-music favourite.",
      "Mayor con la cuarta aumentada: soñador, mágico. Típico de las bandas sonoras.",
      "Dur mit übermäßiger Quarte: verträumt, magisch. Typisch für Filmmusik.", "Majeur avec la quarte augmentée : rêveur, magique. Typique des musiques de film."}},
    {{"misolidio", "Mixolydian", "mixolidio", "Mixolydisch", "mixolydien"}, 6,
     {"Maggiore con la 7a minore: rock, blues, musica popolare.",
      "Major with a minor 7th: rock, blues, folk music.",
      "Mayor con la séptima menor: rock, blues, música popular.",
      "Dur mit kleiner Septime: Rock, Blues, Volksmusik.", "Majeur avec la septième mineure : rock, blues, musique folk."}},
    {{"eolio", "Aeolian", "eólico", "Äolisch", "éolien"}, 5,
     {"È la scala minore naturale. Malinconico.", "It is the natural minor scale. Melancholic.",
      "Es la escala menor natural. Melancólico.", "Das ist das natürliche Moll. Melancholisch.", "C'est la gamme mineure naturelle. Mélancolique."}},
    {{"locrio", "Locrian", "locrio", "Lokrisch", "locrien"}, 4,
     {"Ha la 5a diminuita: instabile, il modo più 'scuro'. Si usa poco.",
      "It has a diminished 5th: unstable, the 'darkest' mode. Rarely used.",
      "Tiene la quinta disminuida: inestable, el modo más 'oscuro'. Se usa poco.",
      "Mit verminderter Quinte: instabil, der 'dunkelste' Modus. Selten benutzt.", "Avec la quinte diminuée : instable, le mode le plus 'sombre'. Peu utilisé."}},
};
/* mode m on root as a scale: the major scale's steps from its m-th degree */
static void mode_scale(int m, scale_t *out)
{
    int i, l;
    const scale_t *maj = &SCALES[SC_MAJOR];
    for (l = 0; l < NLANGS; l++) {
        out->name[l] = MODES[m].name[l];
        out->text[l] = MODES[m].text[l];
    }
    out->n = 7;
    for (i = 0; i < 7; i++) {
        out->semi[i] = (uint8_t)((maj->semi[(m + i) % 7] - maj->semi[m] + 12) % 12);
        out->let[i] = (uint8_t)i;
    }
}
/* the major scale the mode comes from: D dorian -> C major */
static nt_t mode_parent(nt_t root, int m) { return spell(root, (12 - SCALES[SC_MAJOR].semi[m]) % 12, (7 - m) % 7); }

/* a scale's degrees as numbers against the major scale: "1 2 b3 4 5 6 b7" */
static void degree_labels(const scale_t *s, char *b)
{
    int i;
    b[0] = 0;
    for (i = 0; i < s->n; i++) {
        int major = SCALES[SC_MAJOR].semi[s->let[i] % 7], d = (int)s->semi[i] - major;
        char t[4] = {0, 0, 0, 0};
        if (i)
            str_cat(b, " ");
        str_cat(b, d < 0 ? (d == -2 ? "bb" : "b") : d > 0 ? "#" : "");
        t[0] = (char)('1' + s->let[i]);
        str_cat(b, t);
    }
}

/* ------------------------------------------------------------------ chords --- */
typedef struct {
    const char *name[NLANGS];           /* as it follows a note: "minore settima", "minor seventh" */
    const char *sym;                    /* added to the root: "", "m", "dim", "maj7" */
    uint8_t n;
    uint8_t semi[4], let[4];
    const char *formula;                /* "1 3 5" */
    const char *text[NLANGS];
} chord_t;
enum { CH_MAJ, CH_MIN, CH_DIM, CH_AUG, CH_SUS2, CH_SUS4, CH_MAJ7, CH_DOM7, CH_MIN7, CH_HALFDIM, CH_DIM7, CH_MINMAJ7,
       CH_AUGMAJ7, NCHORDS };
#define NTRIADS 6
static const chord_t CHORDS[NCHORDS] = {
    {{"maggiore", "major", "mayor", "Dur", "majeur"}, "", 3, {0, 4, 7}, {0, 2, 4}, "1 3 5",
     {"3a maggiore + 3a minore (4+3 semitoni). Suono luminoso.", "Major 3rd + minor 3rd (4+3 semitones). Bright.",
      "Tercera mayor + tercera menor (4+3 semitonos). Luminoso.", "Große Terz + kleine Terz (4+3 Halbtöne). Hell.", "Tierce majeure + tierce mineure (4+3 demi-tons). Clair."}},
    {{"minore", "minor", "menor", "Moll", "mineur"}, "m", 3, {0, 3, 7}, {0, 2, 4}, "1 b3 5",
     {"3a minore + 3a maggiore (3+4 semitoni). Suono malinconico.", "Minor 3rd + major 3rd (3+4 semitones). Sad.",
      "Tercera menor + tercera mayor (3+4 semitonos). Melancólico.", "Kleine Terz + große Terz (3+4 Halbtöne). Traurig.", "Tierce mineure + tierce majeure (3+4 demi-tons). Triste."}},
    {{"diminuito", "diminished", "disminuido", "vermindert", "diminué"}, "dim", 3, {0, 3, 6}, {0, 2, 4}, "1 b3 b5",
     {"Due 3e minori (3+3). Teso e instabile: vuole risolvere.", "Two minor 3rds (3+3). Tense: it wants to resolve.",
      "Dos terceras menores (3+3). Tenso e inestable: quiere resolver.",
      "Zwei kleine Terzen (3+3). Gespannt: er will sich auflösen.", "Deux tierces mineures (3+3). Tendu : il veut se résoudre."}},
    {{"aumentato", "augmented", "aumentado", "übermäßig", "augmenté"}, "aug", 3, {0, 4, 8}, {0, 2, 4}, "1 3 #5",
     {"Due 3e maggiori (4+4). Misterioso, sospeso.", "Two major 3rds (4+4). Mysterious, floating.",
      "Dos terceras mayores (4+4). Misterioso, suspendido.", "Zwei große Terzen (4+4). Geheimnisvoll, schwebend.", "Deux tierces majeures (4+4). Mystérieux, flottant."}},
    {{"sus2", "sus2", "sus2", "sus2", "sus2"}, "sus2", 3, {0, 2, 7}, {0, 1, 4}, "1 2 5",
     {"La 3a è sostituita dalla 2a: né maggiore né minore.", "The 3rd is replaced by the 2nd: neither major nor minor.",
      "La tercera se cambia por la segunda: ni mayor ni menor.", "Die Terz wird durch die Sekunde ersetzt: weder Dur noch Moll.", "La tierce est remplacée par la seconde : ni majeur ni mineur."}},
    {{"sus4", "sus4", "sus4", "sus4", "sus4"}, "sus4", 3, {0, 5, 7}, {0, 3, 4}, "1 4 5",
     {"La 3a è sostituita dalla 4a: sospeso, vuole tornare alla 3a.",
      "The 3rd is replaced by the 4th: suspended, it wants to fall back to the 3rd.",
      "La tercera se cambia por la cuarta: suspendido, quiere volver a la tercera.",
      "Die Terz wird durch die Quarte ersetzt: schwebend, will zur Terz zurück.", "La tierce est remplacée par la quarte : suspendu, il veut revenir à la tierce."}},
    {{"settima maggiore", "major seventh", "séptima mayor", "Major-Septakkord", "septième majeure"}, "maj7", 4, {0, 4, 7, 11},
     {0, 2, 4, 6}, "1 3 5 7",
     {"Accordo maggiore + 7a maggiore. Morbido e sognante (jazz, pop).",
      "Major chord + major 7th. Soft and dreamy (jazz, pop).",
      "Acorde mayor + séptima mayor. Suave y soñador (jazz, pop).",
      "Dur-Akkord + große Septime. Weich und verträumt (Jazz, Pop).", "Accord majeur + septième majeure. Doux et rêveur (jazz, pop)."}},
    {{"settima di dominante", "dominant seventh", "séptima de dominante", "Dominantseptakkord", "septième de dominante"}, "7", 4,
     {0, 4, 7, 10}, {0, 2, 4, 6}, "1 3 5 b7",
     {"Maggiore + 7a minore. Si forma sul V grado e spinge forte verso la tonica.",
      "Major + minor 7th. Built on the 5th degree, it pulls hard to the tonic.",
      "Mayor + séptima menor. Se forma sobre el V grado y empuja hacia la tónica.",
      "Dur + kleine Septime. Steht auf der V. Stufe und drängt zur Tonika.", "Majeur + septième mineure. Sur le Ve degré, il tire fort vers la tonique."}},
    {{"minore settima", "minor seventh", "menor séptima", "Moll-Septakkord", "mineur septième"}, "m7", 4, {0, 3, 7, 10}, {0, 2, 4, 6},
     "1 b3 5 b7",
     {"Minore + 7a minore. Morbido, molto usato.", "Minor + minor 7th. Soft and very common.",
      "Menor + séptima menor. Suave, muy usado.", "Moll + kleine Septime. Weich und sehr häufig.", "Mineur + septième mineure. Doux, très courant."}},
    {{"semidiminuito", "half-diminished", "semidisminuido", "halbvermindert", "demi-diminué"}, "m7b5", 4, {0, 3, 6, 10},
     {0, 2, 4, 6}, "1 b3 b5 b7",
     {"Diminuito + 7a minore. Si forma sul VII grado della scala maggiore.",
      "Diminished + minor 7th. Built on the 7th degree of the major scale.",
      "Disminuido + séptima menor. Se forma sobre el VII grado de la escala mayor.",
      "Vermindert + kleine Septime. Steht auf der VII. Stufe der Dur-Tonleiter.", "Diminué + septième mineure. Sur le VIIe degré de la gamme majeure."}},
    {{"settima diminuita", "diminished seventh", "séptima disminuida", "verminderter Septakkord", "septième diminuée"}, "dim7", 4,
     {0, 3, 6, 9}, {0, 2, 4, 6}, "1 b3 b5 bb7",
     {"Tre 3e minori sovrapposte. Si forma sul VII grado della minore armonica.",
      "Three stacked minor 3rds. Built on the 7th degree of the harmonic minor.",
      "Tres terceras menores. Se forma sobre el VII grado de la menor armónica.",
      "Drei kleine Terzen übereinander. Steht auf der VII. Stufe von harmonisch Moll.", "Trois tierces mineures empilées. Sur le VIIe degré du mineur harmonique."}},
    {{"minore settima magg.", "minor-major seventh", "menor (7 mayor)", "Moll mit großer Septime", "mineur (7e majeure)"}, "m(maj7)", 4,
     {0, 3, 7, 11}, {0, 2, 4, 6}, "1 b3 5 7",
     {"Minore + 7a maggiore. Si forma sul I grado della minore armonica: suono da film giallo.",
      "Minor + major 7th. Built on the 1st degree of the harmonic minor: a spy-film sound.",
      "Menor + séptima mayor. Se forma sobre el I grado de la menor armónica: sonido de película de espías.",
      "Moll + große Septime. Steht auf der I. Stufe von harmonisch Moll: Agentenfilm-Klang.", "Mineur + septième majeure. Sur le 1er degré du mineur harmonique : un son de film d'espionnage."}},
    {{"aumentato settima magg.", "augmented major seventh", "aumentado (7 mayor)", "übermäßig mit großer Septime", "augmenté (7e majeure)"},
     "maj7#5", 4, {0, 4, 8, 11}, {0, 2, 4, 6}, "1 3 #5 7",
     {"Aumentato + 7a maggiore. Si forma sul III grado della minore armonica.",
      "Augmented + major 7th. Built on the 3rd degree of the harmonic minor.",
      "Aumentado + séptima mayor. Se forma sobre el III grado de la menor armónica.",
      "Übermäßig + große Septime. Steht auf der III. Stufe von harmonisch Moll.", "Augmenté + septième majeure. Sur le IIIe degré du mineur harmonique."}},
};
static nt_t chord_note(int type, nt_t root, int i) { return spell(root, CHORDS[type].semi[i], CHORDS[type].let[i]); }
static uint32_t chord_mask(int type, int root_pc)
{
    uint32_t m = 0;
    int i;
    for (i = 0; i < CHORDS[type].n; i++)
        m |= 1u << ((root_pc + CHORDS[type].semi[i]) % 12);
    return m;
}
/* "Do (C) minore" / "D minor" */
static void chord_name(int type, nt_t root, char *b)
{
    note_full(root, b);
    str_cat(b, " ");
    str_cat(b, TR(CHORDS[type].name));
}
/* the symbol, in letters ("Cm7"); the German way names B natural H and Bb B */
static void chord_symbol(int type, nt_t root, char *b)
{
    b[0] = 0;
    if (th_names == NAMES_GERMAN && root.letter == 6 && root.acc >= -1) {
        str_cat(b, root.acc == -1 ? "B" : "H");
        str_cat(b, acc_text(root.acc > 0 ? root.acc : 0));
    } else {
        note_name_s(root, b, SYS_LETTERS);
    }
    str_cat(b, CHORDS[type].sym);
}

/* -------------------------------------------------- chords on the degrees --- */
/* the chord on degree d of a heptatonic scale (thirds stacked in the scale), n 3 or 4 notes: its type, -1 none */
static int degree_chord(const scale_t *s, int d, int n)
{
    int t, i;
    for (t = 0; t < NCHORDS; t++) {
        if (CHORDS[t].n != n || CHORDS[t].let[1] != 2)
            continue;
        for (i = 0; i < n; i++)
            if ((s->semi[(d + 2 * i) % 7] - s->semi[d] + 12) % 12 != CHORDS[t].semi[i])
                break;
        if (i == n)
            return t;
    }
    return -1;
}

/* the roman numeral: case from the third, then a mark; in the text, '\x01' is a small circle (diminished) and
 * '\x02' a crossed one (half-diminished), drawn by the UI */
static void roman(int d, int type, char *b)
{
    static const char *const UP[7] = {"I", "II", "III", "IV", "V", "VI", "VII"},
                      *const LO[7] = {"i", "ii", "iii", "iv", "v", "vi", "vii"};
    int minor = type >= 0 && CHORDS[type].semi[1] == 3;
    b[0] = 0;
    str_cat(b, minor ? LO[d] : UP[d]);
    switch (type) {
    case CH_DIM: str_cat(b, "\x01"); break;
    case CH_AUG: str_cat(b, "+"); break;
    case CH_MAJ7: str_cat(b, "maj7"); break;
    case CH_DOM7: case CH_MIN7: str_cat(b, "7"); break;
    case CH_HALFDIM: str_cat(b, "\x02" "7"); break;
    case CH_DIM7: str_cat(b, "\x01" "7"); break;
    case CH_MINMAJ7: str_cat(b, "(maj7)"); break;
    case CH_AUGMAJ7: str_cat(b, "+maj7"); break;
    default: break;
    }
}

/* the degrees' names (the 8th: the 7th a whole step below the tonic) and their function */
static const char *const DEGREE_NAME[NLANGS][8] = {
    {"Tonica", "Sopratonica", "Mediante", "Sottodominante", "Dominante", "Sopradominante", "Sensibile", "Sottotonica"},
    {"Tonic", "Supertonic", "Mediant", "Subdominant", "Dominant", "Submediant", "Leading tone", "Subtonic"},
    {"Tónica", "Supertónica", "Mediante", "Subdominante", "Dominante", "Superdominante", "Sensible", "Subtónica"},
    {"Tonika", "Supertonika", "Mediante", "Subdominante", "Dominante", "Submediante", "Leitton", "Subtonika"},
    {"Tonique", "Sus-tonique", "Médiante", "Sous-dominante", "Dominante", "Sus-dominante", "Sensible", "Sous-tonique"},
};
static const char *const DEGREE_FN = "TSTSDTD";
static const char *const FN_NAME[NLANGS][3] = {
    {"riposo (tonica)", "movimento (sottodominante)", "tensione (dominante)"},
    {"rest (tonic)", "movement (subdominant)", "tension (dominant)"},
    {"reposo (tónica)", "movimiento (subdominante)", "tensión (dominante)"},
    {"Ruhe (Tonika)", "Bewegung (Subdominante)", "Spannung (Dominante)"},
    {"repos (tonique)", "mouvement (sous-dominante)", "tension (dominante)"},
};
static int degree_fn(int d) { return DEGREE_FN[d] == 'T' ? 0 : DEGREE_FN[d] == 'S' ? 1 : 2; }
static const char *degree_name(const scale_t *s, int d)
{
    return DEGREE_NAME[th_ui][d == 6 && s->semi[6] == 10 ? 7 : d];
}

/* ------------------------------------------------------- key signatures --- */
/* the 15 major keys round the circle of fifths: C, then the sharps, then the flats */
#define NKEYSIG 15
typedef struct {
    nt_t root;
    int8_t count;                       /* + sharps, - flats */
} keysig_t;
static const keysig_t KEYSIGS[NKEYSIG] = {
    {{0, 0}, 0}, {{4, 0}, 1}, {{1, 0}, 2}, {{5, 0}, 3}, {{2, 0}, 4}, {{6, 0}, 5}, {{3, 1}, 6}, {{0, 1}, 7},
    {{3, 0}, -1}, {{6, -1}, -2}, {{2, -1}, -3}, {{5, -1}, -4}, {{1, -1}, -5}, {{4, -1}, -6}, {{0, -1}, -7},
};
static const nt_t SHARP_ORDER[7] = {{3, 1}, {0, 1}, {4, 1}, {1, 1}, {5, 1}, {2, 1}, {6, 1}};
static const nt_t FLAT_ORDER[7] = {{6, -1}, {2, -1}, {5, -1}, {1, -1}, {4, -1}, {0, -1}, {3, -1}};
static nt_t relative_minor(nt_t major_root) { return spell(major_root, 9, 5); }
/* the alterations of a key, as the major scale has them (count from the scale itself, so it always agrees) */
static int key_count(nt_t root)
{
    int i, c = 0;
    for (i = 0; i < 7; i++)
        c += scale_note(&SCALES[SC_MAJOR], root, i).acc;
    return c;
}
/* the key's alterations as text: "Fa# Do# Sol#" */
static void key_alterations(int count, char *b)
{
    int i, n = count < 0 ? -count : count;
    char t[8];
    b[0] = 0;
    for (i = 0; i < n && i < 7; i++) {
        if (i)
            str_cat(b, " ");
        note_name(count > 0 ? SHARP_ORDER[i] : FLAT_ORDER[i], t);
        str_cat(b, t);
    }
}

/* -------------------------------------------------------- recognition --- */
/* the chord the pitch classes in mask make (bass_pc: the lowest note, -1 unknown): 1 = found, with its type, its
 * root's pitch class and the inversion (0 root position, 1 first, 2 second, 3 third). A reading with the bass as
 * the root wins: C E G A is read as Am7 in its first inversion (there are no 6th chords here), and the symmetric
 * chords (augmented, diminished 7th) are named from the bass */
static int chord_find(uint32_t mask, int bass_pc, int *type, int *root_pc, int *inv)
{
    int pass, r, t;
    for (pass = 0; pass < 2; pass++)
        for (r = 0; r < 12; r++) {
            if (!(mask >> r & 1u) || (pass == 0 && r != bass_pc) || (pass == 1 && r == bass_pc))
                continue;
            for (t = 0; t < NCHORDS; t++)
                if (chord_mask(t, r) == mask) {
                    int i;
                    *type = t;
                    *root_pc = r;
                    *inv = -1;
                    for (i = 0; i < CHORDS[t].n; i++)
                        if ((r + CHORDS[t].semi[i]) % 12 == bass_pc)
                            *inv = i;
                    return 1;
                }
        }
    return 0;
}
