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
       SC_CHROMATIC, NBASESCALES,
       /* the scales of the world and the symmetric ones (the "World scales" lesson) */
       SC_OCTATONIC = NBASESCALES, SC_ENIGMATIC, SC_DOUBLEHARM, SC_HUNGARIAN, SC_PURVI, SC_MARWA, SC_TODI, SC_INSEN,
       SC_YO, SC_HIRAJOSHI, SC_KUMOI, SC_IWATO, SC_RYUKYU, NSCALES };
#define NWORLD (NSCALES - NBASESCALES)
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
    {{"ottatonica", "octatonic", "octatónica", "oktatonisch", "octatonique"}, 8, {0, 2, 3, 5, 6, 8, 9, 11},
     {0, 1, 2, 3, 4, 5, 5, 6},
     {"8 note: tono, semitono, tono... Simmetrica: esiste in sole 3 versioni. Messiaen, jazz, musica di suspense.",
      "8 notes: tone, semitone, tone... Symmetrical: only 3 versions exist. Messiaen, jazz, suspense film music.",
      "8 notas: tono, semitono, tono... Simétrica: solo existen 3 versiones. Messiaen, jazz, música de suspense.",
      "8 Töne: Ganzton, Halbton, Ganzton... Symmetrisch: nur 3 Versionen. Messiaen, Jazz, Spannungs-Filmmusik.",
      "8 notes : ton, demi-ton, ton... Symétrique : seulement 3 versions. Messiaen, jazz, musiques de suspense."}},
    {{"enigmatica", "enigmatic", "enigmática", "enigmatisch", "énigmatique"}, 7, {0, 1, 4, 6, 8, 10, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"La scala 'enigmatica' di Verdi (1888): S, T+S, T, T, T, S, S. Il centro è a toni interi, le estremità a semitoni.",
      "Verdi's 'enigmatic' scale (1888): S, T+S, T, T, T, S, S. Whole tones in the middle, semitones at the ends.",
      "La escala 'enigmática' de Verdi (1888): S, T+S, T, T, T, S, S. Tonos enteros en el centro, semitonos en los extremos.",
      "Verdis 'rätselhafte' Tonleiter (1888): H, G+H, G, G, G, H, H. Ganztöne in der Mitte, Halbtöne an den Enden.",
      "La gamme 'énigmatique' de Verdi (1888) : dT, T+dT, T, T, T, dT, dT. Des tons au centre, des demi-tons aux bords."}},
    {{"doppia armonica", "double harmonic", "doble armónica", "doppelharmonisch", "double harmonique"}, 7,
     {0, 1, 4, 5, 7, 8, 11}, {0, 1, 2, 3, 4, 5, 6},
     {"1 b2 3 4 5 b6 7: due salti di tono e mezzo. Detta bizantina; nel mondo arabo è Hijaz Kar, in India il thaat Bhairav.",
      "1 b2 3 4 5 b6 7: two leaps of a tone and a half. Also Byzantine; Hijaz Kar in Arabic music, thaat Bhairav in India.",
      "1 b2 3 4 5 b6 7: dos saltos de tono y medio. También bizantina; Hijaz Kar en la música árabe, thaat Bhairav en India.",
      "1 b2 3 4 5 b6 7: zwei Sprünge von anderthalb Tönen. Auch byzantinisch; arabisch Hijaz Kar, in Indien Thaat Bhairav.",
      "1 b2 3 4 5 b6 7 : deux sauts d'un ton et demi. Dite byzantine ; Hijaz Kar en musique arabe, thaat Bhairav en Inde."}},
    {{"ungherese minore", "Hungarian minor", "húngara menor", "ungarisch Moll", "hongroise mineure"}, 7,
     {0, 2, 3, 6, 7, 8, 11}, {0, 1, 2, 3, 4, 5, 6},
     {"1 2 b3 #4 5 b6 7: la minore armonica con la 4a alzata. Due salti di tono e mezzo: Liszt, Brahms, Bartók.",
      "1 2 b3 #4 5 b6 7: harmonic minor with a raised 4th. Two tone-and-a-half leaps: Liszt, Brahms, Bartók.",
      "1 2 b3 #4 5 b6 7: la menor armónica con la cuarta elevada. Dos saltos de tono y medio: Liszt, Brahms, Bartók.",
      "1 2 b3 #4 5 b6 7: harmonisches Moll mit erhöhter Quarte. Zwei Sprünge von anderthalb Tönen: Liszt, Brahms, Bartók.",
      "1 2 b3 #4 5 b6 7 : le mineur harmonique à quarte haussée. Deux sauts d'un ton et demi : Liszt, Brahms, Bartók."}},
    {{"thaat Purvi", "thaat Purvi", "thaat Purvi", "Thaat Purvi", "thaat Purvi"}, 7, {0, 1, 4, 6, 7, 8, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"Uno dei 10 thaat indiani: 1 b2 3 #4 5 b6 7. In India la scala è lo scheletro del raga, che ha frasi e un'ora del giorno.",
      "One of India's 10 thaats: 1 b2 3 #4 5 b6 7. In India a scale is a raga's skeleton; a raga has phrases and a time of day.",
      "Uno de los 10 thaat indios: 1 b2 3 #4 5 b6 7. En India la escala es el esqueleto del raga, con frases y hora del día.",
      "Einer der 10 indischen Thaats: 1 b2 3 #4 5 b6 7. Die Tonleiter ist das Gerüst des Raga, mit Phrasen und Tageszeit.",
      "Un des 10 thaats indiens : 1 b2 3 #4 5 b6 7. En Inde, la gamme est le squelette du raga, avec phrases et heure."}},
    {{"thaat Marwa", "thaat Marwa", "thaat Marwa", "Thaat Marwa", "thaat Marwa"}, 7, {0, 1, 4, 6, 7, 9, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"Thaat indiano: 1 b2 3 #4 5 6 7. Inquieto, senza una 'casa' stabile: il raga Marwa si canta al tramonto.",
      "Indian thaat: 1 b2 3 #4 5 6 7. Restless, with no settled home: raga Marwa is sung at sunset.",
      "Thaat indio: 1 b2 3 #4 5 6 7. Inquieto, sin 'casa' estable: el raga Marwa se canta al atardecer.",
      "Indischer Thaat: 1 b2 3 #4 5 6 7. Unruhig, ohne festes Zuhause: Raga Marwa singt man bei Sonnenuntergang.",
      "Thaat indien : 1 b2 3 #4 5 6 7. Inquiet, sans 'maison' stable : le raga Marwa se chante au coucher du soleil."}},
    {{"thaat Todi", "thaat Todi", "thaat Todi", "Thaat Todi", "thaat Todi"}, 7, {0, 1, 3, 6, 7, 8, 11},
     {0, 1, 2, 3, 4, 5, 6},
     {"Thaat indiano: 1 b2 b3 #4 5 b6 7, serio, un raga del mattino. Altri thaat sono i nostri modi: Kafi è il dorico.",
      "Indian thaat: 1 b2 b3 #4 5 b6 7, serious, a morning raga. Other thaats are our modes: Kafi is the Dorian.",
      "Thaat indio: 1 b2 b3 #4 5 b6 7, serio, un raga de la mañana. Otros thaat son nuestros modos: Kafi es el dórico.",
      "Indischer Thaat: 1 b2 b3 #4 5 b6 7, ernst, ein Morgen-Raga. Andere Thaats sind unsere Modi: Kafi ist Dorisch.",
      "Thaat indien : 1 b2 b3 #4 5 b6 7, grave, un raga du matin. D'autres thaats sont nos modes : Kafi est le dorien."}},
    {{"in sen", "in sen", "in sen", "In Sen", "in sen"}, 5, {0, 1, 5, 7, 10}, {0, 1, 3, 4, 6},
     {"Pentatonica giapponese: 1 b2 4 5 b7. Scura e intima, senza sensibile. Si suona sullo shakuhachi e sul koto.",
      "Japanese pentatonic: 1 b2 4 5 b7. Dark and intimate, no leading tone. Played on shakuhachi and koto.",
      "Pentatónica japonesa: 1 b2 4 5 b7. Oscura e íntima, sin sensible. Se toca en el shakuhachi y el koto.",
      "Japanische Pentatonik: 1 b2 4 5 b7. Dunkel und innig, ohne Leitton. Gespielt auf Shakuhachi und Koto.",
      "Pentatonique japonaise : 1 b2 4 5 b7. Sombre et intime, sans sensible. Jouée au shakuhachi et au koto."}},
    {{"yo", "yo", "yo", "Yo", "yo"}, 5, {0, 2, 5, 7, 9}, {0, 1, 3, 4, 5},
     {"Pentatonica giapponese: 1 2 4 5 6. Senza semitoni: aperta e festosa, la scala dei canti popolari.",
      "Japanese pentatonic: 1 2 4 5 6. No semitones: open and festive, the scale of folk songs.",
      "Pentatónica japonesa: 1 2 4 5 6. Sin semitonos: abierta y festiva, la escala de las canciones populares.",
      "Japanische Pentatonik: 1 2 4 5 6. Ohne Halbtöne: offen und festlich, die Tonleiter der Volkslieder.",
      "Pentatonique japonaise : 1 2 4 5 6. Sans demi-tons : ouverte et festive, la gamme des chants populaires."}},
    {{"hirajoshi", "hirajoshi", "hirajoshi", "Hirajoshi", "hirajoshi"}, 5, {0, 2, 3, 7, 8}, {0, 1, 2, 4, 5},
     {"Pentatonica giapponese: 1 2 b3 5 b6. Malinconica, è la scala classica del koto.",
      "Japanese pentatonic: 1 2 b3 5 b6. Melancholic, the classic koto scale.",
      "Pentatónica japonesa: 1 2 b3 5 b6. Melancólica, la escala clásica del koto.",
      "Japanische Pentatonik: 1 2 b3 5 b6. Melancholisch, die klassische Koto-Tonleiter.",
      "Pentatonique japonaise : 1 2 b3 5 b6. Mélancolique, la gamme classique du koto."}},
    {{"kumoi", "kumoi", "kumoi", "Kumoi", "kumoi"}, 5, {0, 2, 3, 7, 9}, {0, 1, 2, 4, 5},
     {"Pentatonica giapponese: 1 2 b3 5 6. Dolce e malinconica, come un minore con la 6a maggiore.",
      "Japanese pentatonic: 1 2 b3 5 6. Sweet and wistful, like a minor with a major 6th.",
      "Pentatónica japonesa: 1 2 b3 5 6. Dulce y melancólica, como un menor con la sexta mayor.",
      "Japanische Pentatonik: 1 2 b3 5 6. Süß und wehmütig, wie Moll mit großer Sexte.",
      "Pentatonique japonaise : 1 2 b3 5 6. Douce et mélancolique, comme un mineur à sixte majeure."}},
    {{"iwato", "iwato", "iwato", "Iwato", "iwato"}, 5, {0, 1, 5, 6, 10}, {0, 1, 3, 4, 6},
     {"Pentatonica giapponese: 1 b2 4 b5 b7. Cupa e tesa, dal suono teatrale.",
      "Japanese pentatonic: 1 b2 4 b5 b7. Dark and tense, with a theatrical sound.",
      "Pentatónica japonesa: 1 b2 4 b5 b7. Sombría y tensa, de sonido teatral.",
      "Japanische Pentatonik: 1 b2 4 b5 b7. Düster und gespannt, mit theatralischem Klang.",
      "Pentatonique japonaise : 1 b2 4 b5 b7. Sombre et tendue, au son théâtral."}},
    {{"ryukyu", "Ryukyu", "ryukyu", "Ryukyu", "ryukyu"}, 5, {0, 4, 5, 7, 11}, {0, 2, 3, 4, 6},
     {"Pentatonica delle isole di Okinawa: 1 3 4 5 7. Luminosa, l'unica giapponese con la sensibile.",
      "Pentatonic of the Okinawa islands: 1 3 4 5 7. Bright, the only Japanese one with a leading tone.",
      "Pentatónica de las islas de Okinawa: 1 3 4 5 7. Luminosa, la única japonesa con sensible.",
      "Pentatonik der Okinawa-Inseln: 1 3 4 5 7. Hell, die einzige japanische mit Leitton.",
      "Pentatonique des îles d'Okinawa : 1 3 4 5 7. Lumineuse, la seule japonaise avec une sensible."}},
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
                   d == 3 ? (de ? "G+H" : fr ? "T+dT" : "T+S") : d == 4 ? (de ? "2G" : "2T") : "?");
    }
}

/* ------------------------------------------------------------------- modes --- */
/* the modes: the rotations of the major scale (the 7 diatonic modes), of the melodic minor and of the harmonic minor */
#define NMODES 21
#define NDIATONIC 7
typedef struct {
    const char *name[NLANGS];
    uint8_t ch;                         /* its characteristic degree (0-based) */
    const char *text[NLANGS];
    uint8_t fam, rot;                   /* the scale it turns (SC_MAJOR, SC_MELMINOR, SC_HARMMINOR), from which degree */
} mode_t_;
static const mode_t_ MODES[NMODES] = {
    {{"ionico", "Ionian", "jónico", "Ionisch", "ionien"}, 3,
     {"È la scala maggiore. Luminoso e stabile.", "It is the major scale. Bright and stable.",
      "Es la escala mayor. Luminoso y estable.", "Das ist die Dur-Tonleiter. Hell und stabil.", "C'est la gamme majeure. Clair et stable."}, SC_MAJOR, 0},
    {{"dorico", "Dorian", "dórico", "Dorisch", "dorien"}, 5,
     {"Minore, ma con la 6a maggiore: un minore 'sereno'. Funk, jazz, musica celtica.",
      "Minor, but with a major 6th: a 'calm' minor. Funk, jazz, Celtic music.",
      "Menor, pero con la sexta mayor: un menor 'sereno'. Funk, jazz, música celta.",
      "Moll, aber mit großer Sexte: ein 'gelassenes' Moll. Funk, Jazz, keltische Musik.", "Mineur, mais avec la sixte majeure : un mineur 'serein'. Funk, jazz, musique celtique."}, SC_MAJOR, 1},
    {{"frigio", "Phrygian", "frigio", "Phrygisch", "phrygien"}, 1,
     {"Minore con la 2a minore: suono spagnolo, flamenco, metal.",
      "Minor with a minor 2nd: a Spanish sound, flamenco, metal.",
      "Menor con la segunda menor: sonido español, flamenco, metal.",
      "Moll mit kleiner Sekunde: spanischer Klang, Flamenco, Metal.", "Mineur avec la seconde mineure : son espagnol, flamenco, metal."}, SC_MAJOR, 2},
    {{"lidio", "Lydian", "lidio", "Lydisch", "lydien"}, 3,
     {"Maggiore con la 4a aumentata: sognante, magico. Tipico delle colonne sonore.",
      "Major with a raised 4th: dreamy, magical. A film-music favourite.",
      "Mayor con la cuarta aumentada: soñador, mágico. Típico de las bandas sonoras.",
      "Dur mit übermäßiger Quarte: verträumt, magisch. Typisch für Filmmusik.", "Majeur avec la quarte augmentée : rêveur, magique. Typique des musiques de film."}, SC_MAJOR, 3},
    {{"misolidio", "Mixolydian", "mixolidio", "Mixolydisch", "mixolydien"}, 6,
     {"Maggiore con la 7a minore: rock, blues, musica popolare.",
      "Major with a minor 7th: rock, blues, folk music.",
      "Mayor con la séptima menor: rock, blues, música popular.",
      "Dur mit kleiner Septime: Rock, Blues, Volksmusik.", "Majeur avec la septième mineure : rock, blues, musique folk."}, SC_MAJOR, 4},
    {{"eolio", "Aeolian", "eólico", "Äolisch", "éolien"}, 5,
     {"È la scala minore naturale. Malinconico.", "It is the natural minor scale. Melancholic.",
      "Es la escala menor natural. Melancólico.", "Das ist das natürliche Moll. Melancholisch.", "C'est la gamme mineure naturelle. Mélancolique."}, SC_MAJOR, 5},
    {{"locrio", "Locrian", "locrio", "Lokrisch", "locrien"}, 4,
     {"Ha la 5a diminuita: instabile, il modo più 'scuro'. Si usa poco.",
      "It has a diminished 5th: unstable, the 'darkest' mode. Rarely used.",
      "Tiene la quinta disminuida: inestable, el modo más 'oscuro'. Se usa poco.",
      "Mit verminderter Quinte: instabil, der 'dunkelste' Modus. Selten benutzt.", "Avec la quinte diminuée : instable, le mode le plus 'sombre'. Peu utilisé."}, SC_MAJOR, 6},
    {{"minore melodica", "melodic minor", "menor melódica", "Moll (melodisch)", "mineur mélodique"}, 6,
     {"Minore con 6a e 7a maggiori, uguale in salita e discesa: la base dell'armonia jazz. Accordo: m(maj7).",
      "Minor with major 6th and 7th, the same up and down: the root of jazz harmony. Chord: m(maj7).",
      "Menor con sexta y séptima mayores, igual al subir y al bajar: la base del jazz. Acorde: m(maj7).",
      "Moll mit großer Sexte und Septime, auf- und abwärts gleich: Basis der Jazzharmonik. Akkord: m(maj7).",
      "Mineur à sixte et septième majeures, pareil en montant et en descendant : la base du jazz. Accord : m(maj7)."},
     SC_MELMINOR, 0},
    {{"dorico b2", "Dorian b2", "dórico b2", "Dorisch b2", "dorien b2"}, 1,
     {"2° modo della melodica: un frigio con la 6a maggiore, più dolce. Accordo: m7.",
      "2nd mode of melodic minor: a Phrygian with a major 6th, softer. Chord: m7.",
      "2.º modo de la melódica: un frigio con la sexta mayor, más suave. Acorde: m7.",
      "2. Modus von melodisch Moll: Phrygisch mit großer Sexte, weicher. Akkord: m7.",
      "2e mode du mineur mélodique : un phrygien à sixte majeure, plus doux. Accord : m7."}, SC_MELMINOR, 1},
    {{"lidio aumentato", "Lydian augmented", "lidio aumentado", "Lydisch übermäßig", "lydien augmenté"}, 4,
     {"3° modo della melodica: lidio con la 5a aumentata. Colore etereo, sospeso. Accordo: maj7(#5).",
      "3rd mode of melodic minor: Lydian with a raised 5th. Ethereal, floating. Chord: maj7(#5).",
      "3.er modo de la melódica: lidio con la quinta aumentada. Color etéreo, suspendido. Acorde: maj7(#5).",
      "3. Modus von melodisch Moll: Lydisch mit übermäßiger Quinte. Ätherisch, schwebend. Akkord: maj7(#5).",
      "3e mode du mineur mélodique : lydien à quinte augmentée. Couleur éthérée, suspendue. Accord : maj7(#5)."},
     SC_MELMINOR, 2},
    {{"lidio dominante", "Lydian dominant", "lidio dominante", "Lydisch-dominant", "lydien dominant"}, 3,
     {"4° modo della melodica: misolidio con la 4a aumentata (#4 e b7). Brillante, jazz. Accordo: 7(#11).",
      "4th mode of melodic minor: Mixolydian with a raised 4th (#4 and b7). Bright and jazzy. Chord: 7(#11).",
      "4.º modo de la melódica: mixolidio con la cuarta aumentada (#4 y b7). Brillante, jazz. Acorde: 7(#11).",
      "4. Modus von melodisch Moll: Mixolydisch mit übermäßiger Quarte (#4, b7). Hell, jazzig. Akkord: 7(#11).",
      "4e mode du mineur mélodique : mixolydien à quarte augmentée (#4 et b7). Brillant, jazzy. Accord : 7(#11)."},
     SC_MELMINOR, 3},
    {{"misolidio b6", "Mixolydian b6", "mixolidio b6", "Mixolydisch b6", "mixolydien b6"}, 5,
     {"5° modo della melodica: maggiore con 6a e 7a minori, detto anche eolio dominante. Accordo: 7(b13).",
      "5th mode of melodic minor: major with minor 6th and 7th, also Aeolian dominant. Chord: 7(b13).",
      "5.º modo de la melódica: mayor con sexta y séptima menores, o eólico dominante. Acorde: 7(b13).",
      "5. Modus von melodisch Moll: Dur mit kleiner Sexte und Septime, auch äolisch-dominant. Akkord: 7(b13).",
      "5e mode du mineur mélodique : majeur à sixte et septième mineures, ou éolien dominant. Accord : 7(b13)."},
     SC_MELMINOR, 4},
    {{"locrio #2", "Locrian #2", "locrio #2", "Lokrisch #2", "locrien #2"}, 1,
     {"6° modo della melodica: locrio con la 2a maggiore, meno cupo. Sul II grado del minore (ii-V). Accordo: m7b5.",
      "6th mode of melodic minor: Locrian with a major 2nd, less dark. On the II of a minor key (ii-V). Chord: m7b5.",
      "6.º modo de la melódica: locrio con la segunda mayor, menos oscuro. En el II del menor (ii-V). Acorde: m7b5.",
      "6. Modus von melodisch Moll: Lokrisch mit großer Sekunde, heller. Auf der II. Stufe in Moll. Akkord: m7b5.",
      "6e mode du mineur mélodique : locrien à seconde majeure, moins sombre. Sur le IIe du mineur. Accord : m7b5."},
     SC_MELMINOR, 5},
    {{"superlocrio", "super-Locrian", "superlocrio", "Superlokrisch", "superlocrien"}, 3,
     {"7° modo della melodica, la scala alterata: tutto abbassato tranne la tonica. Massima tensione sul V7.",
      "7th mode of melodic minor, the altered scale: all flattened but the root. Maximum tension on a V7.",
      "7.º modo de la melódica, la escala alterada: todo bajado salvo la tónica. Máxima tensión sobre el V7.",
      "7. Modus von melodisch Moll, die alterierte Skala: alles erniedrigt außer dem Grundton. Höchste Spannung auf V7.",
      "7e mode du mineur mélodique, la gamme altérée : tout baissé sauf la tonique. Tension maximale sur le V7."},
     SC_MELMINOR, 6},
    {{"minore armonica", "harmonic minor", "menor armónica", "Moll (harmonisch)", "mineur harmonique"}, 6,
     {"Minore con la 7a alzata, la sensibile: è la minore della musica classica. Accordo: m(maj7).",
      "Minor with a raised 7th, the leading tone: the minor of classical music. Chord: m(maj7).",
      "Menor con la séptima elevada, la sensible: el menor de la música clásica. Acorde: m(maj7).",
      "Moll mit erhöhter 7. Stufe, dem Leitton: das Moll der Klassik. Akkord: m(maj7).",
      "Mineur à septième haussée, la sensible : le mineur de la musique classique. Accord : m(maj7)."},
     SC_HARMMINOR, 0},
    {{"locrio #6", "Locrian #6", "locrio #6", "Lokrisch #6", "locrien #6"}, 5,
     {"2° modo dell'armonica: locrio con la 6a maggiore. Colore dolce-amaro. Accordo: m7b5.",
      "2nd mode of harmonic minor: Locrian with a major 6th. Bittersweet. Chord: m7b5.",
      "2.º modo de la armónica: locrio con la sexta mayor. Color agridulce. Acorde: m7b5.",
      "2. Modus von harmonisch Moll: Lokrisch mit großer Sexte. Bittersüß. Akkord: m7b5.",
      "2e mode du mineur harmonique : locrien à sixte majeure. Aigre-doux. Accord : m7b5."}, SC_HARMMINOR, 1},
    {{"ionico #5", "Ionian #5", "jónico #5", "Ionisch #5", "ionien #5"}, 4,
     {"3° modo dell'armonica: maggiore con la 5a aumentata. Raro, luminoso e strano. Accordo: maj7(#5).",
      "3rd mode of harmonic minor: major with a raised 5th. Rare, bright and strange. Chord: maj7(#5).",
      "3.er modo de la armónica: mayor con la quinta aumentada. Raro, luminoso y extraño. Acorde: maj7(#5).",
      "3. Modus von harmonisch Moll: Dur mit übermäßiger Quinte. Selten, hell und seltsam. Akkord: maj7(#5).",
      "3e mode du mineur harmonique : majeur à quinte augmentée. Rare, lumineux et étrange. Accord : maj7(#5)."},
     SC_HARMMINOR, 2},
    {{"dorico #4", "Dorian #4", "dórico #4", "Dorisch #4", "dorien #4"}, 3,
     {"4° modo dell'armonica, detto ucraino o rumeno: dorico con la 4a aumentata. Klezmer, Europa dell'Est.",
      "4th mode of harmonic minor, Ukrainian or Romanian: Dorian with a raised 4th. Klezmer, Eastern Europe.",
      "4.º modo de la armónica, ucraniano o rumano: dórico con la cuarta aumentada. Klezmer, Europa del Este.",
      "4. Modus von harmonisch Moll, ukrainisch oder rumänisch: Dorisch mit erhöhter Quarte. Klezmer, Osteuropa.",
      "4e mode du mineur harmonique, ukrainien ou roumain : dorien à quarte augmentée. Klezmer, Europe de l'Est."},
     SC_HARMMINOR, 3},
    {{"frigio dominante", "Phrygian dominant", "frigio dominante", "Phrygisch-dominant", "phrygien dominant"}, 2,
     {"5° modo dell'armonica: frigio con la 3a maggiore, un salto di tono e mezzo. Flamenco, klezmer, maqam Hijaz.",
      "5th mode of harmonic minor: Phrygian with a major 3rd, a tone-and-a-half leap. Flamenco, klezmer, maqam Hijaz.",
      "5.º modo de la armónica: frigio con la tercera mayor, un salto de tono y medio. Flamenco, klezmer, maqam Hijaz.",
      "5. Modus von harmonisch Moll: Phrygisch mit großer Terz, anderthalb Töne Sprung. Flamenco, Klezmer, Maqam Hijaz.",
      "5e mode du mineur harmonique : phrygien à tierce majeure, saut d'un ton et demi. Flamenco, klezmer, maqam Hijaz."},
     SC_HARMMINOR, 4},
    {{"lidio #2", "Lydian #2", "lidio #2", "Lydisch #2", "lydien #2"}, 1,
     {"6° modo dell'armonica: lidio con la 2a aumentata. Colore misterioso, da film fantasy.",
      "6th mode of harmonic minor: Lydian with a raised 2nd. Mysterious, a fantasy-film colour.",
      "6.º modo de la armónica: lidio con la segunda aumentada. Color misterioso, de película fantástica.",
      "6. Modus von harmonisch Moll: Lydisch mit übermäßiger Sekunde. Geheimnisvoll, wie im Fantasy-Film.",
      "6e mode du mineur harmonique : lydien à seconde augmentée. Couleur mystérieuse, de film fantastique."},
     SC_HARMMINOR, 5},
    {{"superlocrio bb7", "super-Locrian bb7", "superlocrio bb7", "Superlokrisch bb7", "superlocrien bb7"}, 6,
     {"7° modo dell'armonica: tutto abbassato, la 7a due volte (bb7). Le note del suo accordo fanno un dim7.",
      "7th mode of harmonic minor: all flattened, the 7th twice (bb7). Its chord tones make a dim7 chord.",
      "7.º modo de la armónica: todo bajado, la séptima dos veces (bb7). Sus notas de acorde forman un dim7.",
      "7. Modus von harmonisch Moll: alles erniedrigt, die Septime doppelt (bb7). Seine Akkordtöne bilden dim7.",
      "7e mode du mineur harmonique : tout baissé, la septième deux fois (bb7). Ses notes forment un accord dim7."},
     SC_HARMMINOR, 6},
};

/* mode m on root as a scale: its parent scale's steps from its rot-th degree */
static void mode_scale(int m, scale_t *out)
{
    int i, l;
    const scale_t *par = &SCALES[MODES[m].fam];
    const int r = MODES[m].rot;
    for (l = 0; l < NLANGS; l++) {
        out->name[l] = MODES[m].name[l];
        out->text[l] = MODES[m].text[l];
    }
    out->n = 7;
    for (i = 0; i < 7; i++) {
        out->semi[i] = (uint8_t)((par->semi[(r + i) % 7] - par->semi[r] + 12) % 12);
        out->let[i] = (uint8_t)i;
    }
}
/* the root of the scale the mode comes from: D dorian -> C (major), C altered -> Db (melodic minor) */
static nt_t mode_parent(nt_t root, int m)
{
    const int r = MODES[m].rot;
    return spell(root, (12 - SCALES[MODES[m].fam].semi[r]) % 12, (7 - r) % 7);
}

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

/* ------------------------------------------------------------ the staff --- */
/* The seven clefs (the Italian setticlavio): a sign (G, F or C) on a line (1 the bottom line .. 5 the top), and the
 * note that line is then. A note on the staff is a position: 0 the bottom line, 1 the space above it, ... 8 the top
 * line; -2 and -4 the ledger lines below, 10 and 12 above (two ledger lines at most). Notes are counted
 * diatonically: d = octave * 7 + letter, C4 (middle C) = 28. */
enum { SIGN_G, SIGN_F, SIGN_C };
enum { CLEF_TREBLE, CLEF_BASS, CLEF_SOPRANO, CLEF_MEZZO, CLEF_ALTO, CLEF_TENOR, CLEF_BARITONE, NCLEFS };
#define STAFF_LO (-4)
#define STAFF_HI 12
typedef struct {
    uint8_t sign, line;
    int8_t ref;                         /* the sign's note, diatonically: G4 32, F3 24, C4 28 */
    const char *name[NLANGS];
} clef_t;
static const clef_t CLEFS[NCLEFS] = {
    {SIGN_G, 2, 32, {"Chiave di violino", "Treble clef", "Clave de sol", "Violinschlüssel", "Clé de sol"}},
    {SIGN_F, 4, 24, {"Chiave di basso", "Bass clef", "Clave de fa", "Bassschlüssel", "Clé de fa"}},
    {SIGN_C, 1, 28, {"Chiave di soprano", "Soprano clef", "Clave de do en 1.a", "Sopranschlüssel", "Clé d'ut 1re"}},
    {SIGN_C, 2, 28, {"Chiave di mezzosoprano", "Mezzo-soprano clef", "Clave de do en 2.a", "Mezzosopranschlüssel",
                     "Clé d'ut 2e"}},
    {SIGN_C, 3, 28, {"Chiave di contralto", "Alto clef", "Clave de do en 3.a", "Altschlüssel", "Clé d'ut 3e"}},
    {SIGN_C, 4, 28, {"Chiave di tenore", "Tenor clef", "Clave de do en 4.a", "Tenorschlüssel", "Clé d'ut 4e"}},
    {SIGN_F, 3, 24, {"Chiave di baritono", "Baritone clef", "Clave de fa en 3.a", "Baritonschlüssel", "Clé de fa 3e"}},
};
/* the clefs a player has chosen: the treble only, treble and bass, or all seven */
enum { CLEFSET_TREBLE, CLEFSET_TWO, CLEFSET_ALL, NCLEFSETS };
static int clefs_in(int set) { return set == CLEFSET_TREBLE ? 1 : set == CLEFSET_TWO ? 2 : NCLEFS; }

/* the note at a position on a clef's staff (acc: its accidental), and its MIDI number */
static nt_t staff_note(int clef, int pos, int acc, int *midi)
{
    int d = CLEFS[clef].ref - 2 * (CLEFS[clef].line - 1) + pos, oct = d / 7;
    nt_t n;
    n.letter = (int8_t)(d % 7);
    n.acc = (int8_t)acc;
    if (midi)
        *midi = 12 * (oct + 1) + LETTER_PC[n.letter] + acc;
    return n;
}
static int staff_octave(int clef, int pos) { return (CLEFS[clef].ref - 2 * (CLEFS[clef].line - 1) + pos) / 7; }

/* "Sol4" / "Sol4 (G4)": a note with its octave (scientific: middle C is C4) */
static void note_oct(nt_t n, int oct, char *b)
{
    char t[8], o[4] = {0, 0, 0, 0};
    o[0] = (char)('0' + (oct < 0 ? 0 : oct > 9 ? 9 : oct));
    note_name(n, b);
    str_cat(b, o);
    note_alt(n, t);
    if (t[0]) {
        str_cat(b, " (");
        str_cat(b, t);
        str_cat(b, o);
        str_cat(b, ")");
    }
}
