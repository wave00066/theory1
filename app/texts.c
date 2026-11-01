/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* Every word on the screen that is not theory (theory.c has those), in Italian, English, Spanish and German.
 * UTF-8: the app draws the accents (app.c tx). %1, %2, %3 are filled in by fmt(). The hints (the line above the
 * keyboard) are in the small capitals font. */

enum {
    S_TAGLINE, S_M_LESSONS, S_M_QUIZ, S_M_LAB, S_M_SETTINGS, S_M_LESSONS_SUB, S_M_QUIZ_SUB, S_M_LAB_SUB,
    S_M_SETTINGS_SUB, S_H_MENU, S_H_TOPICS,
    S_H_BASICS, S_H_INTERVALS, S_H_SCALES, S_H_MODES, S_H_CHORDS, S_H_HARMONY3, S_H_HARMONY4, S_H_KEYS,
    S_SEMITONE, S_SEMITONES, S_ABOVE, S_SYMBOL, S_STEPS, S_CHROM_STEPS, S_MODE_PARENT, S_STRUCTURE, S_FUNCTION,
    S_MODE_CHORDS, S_KEY_MAJ, S_NO_ALT, S_SHARP_1, S_SHARP_N, S_FLAT_1, S_FLAT_N, S_RELATIVE, S_CW, S_CCW,
    S_SHARPS, S_FLATS, S_FIFTHS,
    S_RECORD, S_LEVEL_K1, S_H_QSETUP, S_Q_NOTE, S_Q_SAMEKEY, S_Q_INT1, S_Q_INT2, S_Q_SCALE, S_Q_MODE, S_Q_CHORD,
    S_Q_DEG_MAJ, S_Q_DEG_MIN, S_Q_DEG_TRI, S_Q_DEG_SEV, S_Q_EAR_INT, S_Q_EAR_CHORD, S_Q_REPLAY, S_Q_HOWMANY,
    S_MAJOR_Q, S_Q_NONE, S_Q_REL, S_MINOR_OF, S_MAJOR_OF, S_Q_WHICH, S_Q_NOALT_Q,
    S_F_RIGHT, S_F_SKIPPED, S_F_WRONG, S_F_START, S_F_PLAYED, S_F_GOON, S_F_YES, S_F_FROM_TONIC, S_F_NOT_NEXT,
    S_F_IN_CHORD, S_F_NOT_CHORD, S_F_LOOK, S_F_HINT, S_F_ANSWER, S_F_ERRORS, S_F_HINTS, S_F_HELP_SEQ, S_F_HELP_SET,
    S_F_HELP_ONE, S_H_QCHOICE, S_H_QPLAY, S_H_NEXT, S_Q_HEADER, S_Q_SCORE,
    S_END, S_END_3, S_END_2, S_END_1, S_END_0, S_NEW_BEST, S_BEST, S_LEVEL, S_H_END,
    S_NO_SCALE, S_PLAY_SOMETHING, S_LAB_INTRO, S_OCTAVE_MIDI, S_DEGREE_OF, S_OUT_OF_SCALE, S_PLUS_OCTAVE, S_NOTES,
    S_DEGREE, S_UNKNOWN_CHORD, S_UNKNOWN_TIP, S_OUT_RED, S_LAST, S_LAB_SEL1, S_LAB_SEL0, S_H_LAB,
    S_SET_LANG, S_SET_NAMES, S_SET_SOUND, S_SET_SPEED, S_SET_EXT, S_SET_BLE, S_SET_DEVICE, S_SET_RESET,
    S_EXT_BOTH, S_EXT_MUTE, S_ON, S_OFF, S_NOT_BUILT, S_CONNECTED, S_SEARCHING, S_REC_SEARCH, S_SEL_RESET,
    S_H_SETTINGS, S_SAY_RECORDS, S_SAY_RESET, S_SAY_OCTAVE, S_SAY_SAVED, S_SAY_ERROR, S_SAY_SEARCHING,
    S_SAY_CONNECTING, S_SAY_FOUND,
    NSTR
};

static const char *const STR[NSTR][NLANGS] = {
    [S_TAGLINE] = {"teoria musicale", "music theory", "teoría musical", "Musiktheorie"},
    [S_M_LESSONS] = {"LEZIONI", "LESSONS", "LECCIONES", "LEKTIONEN"},
    [S_M_QUIZ] = {"QUIZ", "QUIZ", "QUIZ", "QUIZ"},
    [S_M_LAB] = {"LABORATORIO", "LAB", "LABORATORIO", "LABOR"},
    [S_M_SETTINGS] = {"IMPOSTAZIONI", "SETTINGS", "AJUSTES", "EINSTELLUNGEN"},
    [S_M_LESSONS_SUB] = {"scale, modi, accordi, armonia...", "scales, modes, chords, harmony...",
                         "escalas, modos, acordes, armonía...", "Tonleitern, Modi, Akkorde, Harmonik..."},
    [S_M_QUIZ_SUB] = {"mettiti alla prova sui tasti", "test yourself on the keys", "ponte a prueba en las teclas",
                      "teste dich auf den Tasten"},
    [S_M_LAB_SUB] = {"suona: ti dico che note e accordi sono", "play: I name the notes and chords",
                     "toca: te digo qué notas y acordes son", "spiele: ich nenne Töne und Akkorde"},
    [S_M_SETTINGS_SUB] = {"lingua, nomi delle note, suono, Bluetooth", "language, note names, sound, Bluetooth",
                          "idioma, nombres de notas, sonido", "Sprache, Notennamen, Klang, Bluetooth"},
    [S_H_MENU] = {"SELECT SCEGLI   SEL ENTRA", "SELECT CHOOSE   SEL ENTER", "SELECT ELIGE   SEL ENTRA",
                  "SELECT WÄHLEN   SEL ÖFFNEN"},
    [S_H_TOPICS] = {"SELECT SCEGLI   SEL ENTRA   HOME MENU", "SELECT CHOOSE   SEL ENTER   HOME MENU",
                    "SELECT ELIGE   SEL ENTRA   HOME MENÚ", "SELECT WÄHLEN   SEL ÖFFNEN   HOME MENÜ"},

    [S_H_BASICS] = {"SELECT PAGINA   PLAY ASCOLTA   HOME INDIETRO", "SELECT PAGE   PLAY LISTEN   HOME BACK",
                    "SELECT PÁGINA   PLAY ESCUCHA   HOME ATRÁS", "SELECT SEITE   PLAY HÖREN   HOME ZURÜCK"},
    [S_H_INTERVALS] = {"SELECT INTERVALLO   K1 NOTA   PLAY ASCOLTA", "SELECT INTERVAL   K1 NOTE   PLAY LISTEN",
                       "SELECT INTERVALO   K1 NOTA   PLAY ESCUCHA", "SELECT INTERVALL   K1 TON   PLAY HÖREN"},
    [S_H_SCALES] = {"SELECT SCALA   K1 TONICA   PLAY ASCOLTA", "SELECT SCALE   K1 ROOT   PLAY LISTEN",
                    "SELECT ESCALA   K1 TÓNICA   PLAY ESCUCHA", "SELECT TONLEITER   K1 GRUNDTON   PLAY HÖREN"},
    [S_H_MODES] = {"SELECT MODO   K1 TONICA   PLAY ASCOLTA", "SELECT MODE   K1 ROOT   PLAY LISTEN",
                   "SELECT MODO   K1 TÓNICA   PLAY ESCUCHA", "SELECT MODUS   K1 GRUNDTON   PLAY HÖREN"},
    [S_H_CHORDS] = {"SELECT ACCORDO   K1 TONICA   PLAY ASCOLTA", "SELECT CHORD   K1 ROOT   PLAY LISTEN",
                    "SELECT ACORDE   K1 TÓNICA   PLAY ESCUCHA", "SELECT AKKORD   K1 GRUNDTON   PLAY HÖREN"},
    [S_H_HARMONY3] = {"SELECT SCALA K1 TONICA K2 GRADO K3 SETTIME ARP TUTTI",
                      "SELECT SCALE K1 ROOT K2 DEGREE K3 SEVENTHS ARP ALL",
                      "SELECT ESCALA K1 TÓNICA K2 GRADO K3 SÉPTIMAS ARP TODO",
                      "SELECT TONL. K1 GRUNDT. K2 STUFE K3 SEPT. ARP ALLE"},
    [S_H_HARMONY4] = {"SELECT SCALA K1 TONICA K2 GRADO K3 TRIADI ARP TUTTI",
                      "SELECT SCALE K1 ROOT K2 DEGREE K3 TRIADS ARP ALL",
                      "SELECT ESCALA K1 TÓNICA K2 GRADO K3 TRÍADAS ARP TODO",
                      "SELECT TONL. K1 GRUNDT. K2 STUFE K3 DREIKL. ARP ALLE"},
    [S_H_KEYS] = {"SELECT TONALITÀ   PLAY SCALA   HOME INDIETRO", "SELECT KEY   PLAY SCALE   HOME BACK",
                  "SELECT TONALIDAD   PLAY ESCALA   HOME ATRÁS", "SELECT TONART   PLAY TONLEITER   HOME ZURÜCK"},

    [S_SEMITONE] = {"semitono", "semitone", "semitono", "Halbton"},
    [S_SEMITONES] = {"semitoni", "semitones", "semitonos", "Halbtöne"},
    [S_ABOVE] = {"(sopra)", "(above)", "(arriba)", "(oben)"},
    [S_SYMBOL] = {"sigla", "symbol", "cifrado", "Symbol"},
    [S_STEPS] = {"Passi: ", "Steps: ", "Pasos: ", "Schritte: "},
    [S_CHROM_STEPS] = {"Passi: 12 semitoni, tutti uguali", "Steps: 12 semitones, all equal",
                       "Pasos: 12 semitonos, todos iguales", "Schritte: 12 Halbtöne, alle gleich"},
    [S_MODE_PARENT] = {"= %1 maggiore da %2.  Nota tipica: %3", "= %1 major from %2.  Typical note: %3",
                       "= %1 mayor desde %2.  Nota típica: %3", "= %1 Dur ab %2.  Typischer Ton: %3"},
    [S_STRUCTURE] = {"Struttura: %1 semitoni", "Structure: %1 semitones", "Estructura: %1 semitonos",
                     "Aufbau: %1 Halbtöne"},
    [S_FUNCTION] = {"Funzione: %1", "Function: %1", "Función: %1", "Funktion: %1"},
    [S_MODE_CHORDS] = {"Accordi del modo: suonali per sentirne il colore", "The mode's chords: play them to hear it",
                       "Acordes del modo: tócalos para oír su color", "Akkorde des Modus: spiele sie, hör die Farbe"},
    [S_KEY_MAJ] = {"%1 magg.", "%1 major", "%1 mayor", "%1 Dur"},
    [S_NO_ALT] = {"nessuna alterazione", "no sharps or flats", "sin alteraciones", "keine Vorzeichen"},
    [S_SHARP_1] = {"diesis", "sharp", "sostenido", "Kreuz"},
    [S_SHARP_N] = {"diesis", "sharps", "sostenidos", "Kreuze"},
    [S_FLAT_1] = {"bemolle", "flat", "bemol", "b-Vorzeichen"},
    [S_FLAT_N] = {"bemolli", "flats", "bemoles", "b-Vorzeichen"},
    [S_RELATIVE] = {"Relativa: %1 min.", "Relative: %1 minor", "Relativa: %1 menor", "Parallele: %1 Moll"},
    [S_CW] = {"Ogni passo in senso orario: una 5a sopra, un # in più.",
              "Each step clockwise: a 5th up, one more sharp.",
              "En sentido horario: una quinta arriba, un # más.",
              "Im Uhrzeigersinn: eine Quinte höher, ein # mehr."},
    [S_CCW] = {"In senso antiorario: una 5a sotto (4a sopra), un b in più.",
               "Counter-clockwise: a 5th down (a 4th up), one more flat.",
               "Antihorario: una quinta abajo (cuarta arriba), un b más.",
               "Gegen den Uhrzeigersinn: eine Quinte tiefer, ein b mehr."},
    [S_SHARPS] = {"Diesis:", "Sharps:", "Sostenidos:", "Kreuze:"},
    [S_FLATS] = {"Bemolli:", "Flats:", "Bemoles:", "b:"},
    [S_FIFTHS] = {"QUINTE", "FIFTHS", "QUINTAS", "QUINTEN"},

    [S_RECORD] = {"record", "best", "récord", "Rekord"},
    [S_LEVEL_K1] = {"Livello (K1):", "Level (K1):", "Nivel (K1):", "Niveau (K1):"},
    [S_H_QSETUP] = {"SELECT TIPO   K1 LIVELLO   SEL INIZIA", "SELECT TYPE   K1 LEVEL   SEL START",
                    "SELECT TIPO   K1 NIVEL   SEL EMPIEZA", "SELECT ART   K1 NIVEAU   SEL START"},
    [S_Q_NOTE] = {"Suona la nota:", "Play the note:", "Toca la nota:", "Spiele den Ton:"},
    [S_Q_SAMEKEY] = {"(stesso tasto di %1)", "(same key as %1)", "(misma tecla que %1)", "(gleiche Taste wie %1)"},
    [S_Q_INT1] = {"Suona la nota a una", "Play the note a", "Toca la nota a una", "Spiele den Ton eine"},
    [S_Q_INT2] = {"%1 sopra %2", "%1 above %2", "%1 sobre %2", "%1 über %2"},
    [S_Q_SCALE] = {"Suona la scala di", "Play the scale of", "Toca la escala de", "Spiele die Tonleiter"},
    [S_Q_MODE] = {"Suona il modo", "Play the mode", "Toca el modo", "Spiele den Modus"},
    [S_Q_CHORD] = {"Suona l'accordo di", "Play the chord", "Toca el acorde de", "Spiele den Akkord"},
    [S_Q_DEG_MAJ] = {"In %1 maggiore suona", "In %1 major, play", "En %1 mayor toca", "In %1 Dur spiele"},
    [S_Q_DEG_MIN] = {"In %1 minore armonica suona", "In %1 harmonic minor, play", "En %1 menor armónica toca",
                     "In %1 Moll (harmonisch) spiele"},
    [S_Q_DEG_TRI] = {"l'accordo del %1 grado", "the chord on degree %1", "el acorde del %1 grado",
                     "den Akkord der %1. Stufe"},
    [S_Q_DEG_SEV] = {"la settima sul %1 grado", "the seventh on degree %1", "la séptima del %1 grado",
                     "den Septakkord der %1. Stufe"},
    [S_Q_EAR_INT] = {"Ascolta. Che intervallo è?", "Listen. Which interval is it?", "Escucha. Qué intervalo es?",
                     "Hör zu. Welches Intervall ist das?"},
    [S_Q_EAR_CHORD] = {"Ascolta. Che accordo è?", "Listen. Which chord is it?", "Escucha. Qué acorde es?",
                       "Hör zu. Welcher Akkord ist das?"},
    [S_Q_REPLAY] = {"PLAY per riascoltare", "PLAY to hear it again", "PLAY para volver a oír", "PLAY: noch einmal"},
    [S_Q_HOWMANY] = {"Quante alterazioni ha", "How many sharps or flats in", "Cuántas alteraciones tiene",
                     "Wie viele Vorzeichen hat"},
    [S_MAJOR_Q] = {"%1 maggiore?", "%1 major?", "%1 mayor?", "%1 Dur?"},
    [S_Q_NONE] = {"nessuna", "none", "ninguna", "keine"},
    [S_Q_REL] = {"Qual è la relativa minore di", "What is the relative minor of", "Cuál es la relativa menor de",
                 "Welches Moll ist die Parallele von"},
    [S_MINOR_OF] = {"%1 minore", "%1 minor", "%1 menor", "%1 Moll"},
    [S_MAJOR_OF] = {"%1 maggiore", "%1 major", "%1 mayor", "%1 Dur"},
    [S_Q_WHICH] = {"Quale tonalità maggiore ha", "Which major key has", "Qué tonalidad mayor tiene",
                   "Welche Dur-Tonart hat"},
    [S_Q_NOALT_Q] = {"nessuna alterazione?", "no sharps or flats?", "ninguna alteración?", "keine Vorzeichen?"},

    [S_F_RIGHT] = {"Giusto! +%1", "Right! +%1", "Correcto! +%1", "Richtig! +%1"},
    [S_F_SKIPPED] = {"Saltata", "Skipped", "Saltada", "Übersprungen"},
    [S_F_WRONG] = {"No...", "No...", "No...", "Leider nein..."},
    [S_F_START] = {"Questa è la nota di partenza", "That is the starting note", "Esa es la nota de partida",
                   "Das ist der Startton"},
    [S_F_PLAYED] = {"No: hai suonato %1", "No: you played %1", "No: tocaste %1", "Nein: das war %1"},
    [S_F_GOON] = {"Bene, continua", "Good, go on", "Bien, sigue", "Gut, weiter"},
    [S_F_YES] = {"Sì!", "Yes!", "Sí!", "Ja!"},
    [S_F_FROM_TONIC] = {"%1: si parte dalla tonica", "%1: start from the root", "%1: se empieza por la tónica",
                        "%1: fang beim Grundton an"},
    [S_F_NOT_NEXT] = {"%1: non è la nota dopo", "%1: not the next note", "%1: no es la siguiente",
                      "%1: nicht der nächste Ton"},
    [S_F_IN_CHORD] = {"Sì, fa parte dell'accordo", "Yes, it is in the chord", "Sí, está en el acorde",
                      "Ja, gehört zum Akkord"},
    [S_F_NOT_CHORD] = {"%1: non fa parte dell'accordo", "%1: not in the chord", "%1: no está en el acorde",
                       "%1: gehört nicht dazu"},
    [S_F_LOOK] = {" - guarda i tasti", " - look at the keys", " - mira las teclas", " - schau auf die Tasten"},
    [S_F_HINT] = {"Aiuto: guarda i tasti accesi", "Hint: look at the lit keys", "Ayuda: mira las teclas encendidas",
                  "Tipp: schau auf die hellen Tasten"},
    [S_F_ANSWER] = {"Risposta: %1", "Answer: %1", "Respuesta: %1", "Antwort: %1"},
    [S_F_ERRORS] = {"Errori: %1", "Mistakes: %1", "Errores: %1", "Fehler: %1"},
    [S_F_HINTS] = {"   Aiuti: %1", "   Hints: %1", "   Ayudas: %1", "   Tipps: %1"},
    [S_F_HELP_SEQ] = {"Suona le note una alla volta, in salita, in qualsiasi ottava.",
                      "Play the notes one at a time, going up, in any octave.",
                      "Toca las notas una a una, subiendo, en cualquier octava.",
                      "Spiele die Töne einzeln, aufwärts, in jeder Oktave."},
    [S_F_HELP_SET] = {"Suona le note insieme o una alla volta, in qualsiasi ottava.",
                      "Play the notes together or one at a time, in any octave.",
                      "Toca las notas juntas o una a una, en cualquier octava.",
                      "Spiele die Töne zusammen oder einzeln, in jeder Oktave."},
    [S_F_HELP_ONE] = {"Suona la nota in qualsiasi ottava.", "Play the note in any octave.",
                      "Toca la nota en cualquier octava.", "Spiele den Ton in jeder Oktave."},
    [S_H_QCHOICE] = {"SELECT SCEGLI  SEL RISPONDI  PLAY ASCOLTA  EDIT AIUTO",
                     "SELECT CHOOSE  SEL ANSWER  PLAY LISTEN  EDIT HINT",
                     "SELECT ELIGE  SEL RESPONDE  PLAY ESCUCHA  EDIT AYUDA",
                     "SELECT WÄHLEN  SEL ANTWORT  PLAY HÖREN  EDIT TIPP"},
    [S_H_QPLAY] = {"PLAY NOTA DI RIFERIMENTO  EDIT AIUTO  FX SALTA", "PLAY REFERENCE NOTE  EDIT HINT  FX SKIP",
                   "PLAY NOTA DE REFERENCIA  EDIT AYUDA  FX SALTA", "PLAY BEZUGSTON  EDIT TIPP  FX ÜBERSPRINGEN"},
    [S_H_NEXT] = {"SEL AVANTI", "SEL NEXT", "SEL SIGUIENTE", "SEL WEITER"},
    [S_Q_HEADER] = {"QUIZ %1/%2", "QUIZ %1/%2", "QUIZ %1/%2", "QUIZ %1/%2"},
    [S_Q_SCORE] = {"Punti %1", "Score %1", "Puntos %1", "Punkte %1"},

    [S_END] = {"QUIZ FINITO", "QUIZ OVER", "QUIZ TERMINADO", "QUIZ BEENDET"},
    [S_END_3] = {"Eccellente!", "Excellent!", "Excelente!", "Ausgezeichnet!"},
    [S_END_2] = {"Molto bene!", "Very good!", "Muy bien!", "Sehr gut!"},
    [S_END_1] = {"Bene, continua così!", "Good, keep it up!", "Bien, sigue así!", "Gut, weiter so!"},
    [S_END_0] = {"Ripassa le lezioni e riprova!", "Review the lessons and try again!",
                 "Repasa las lecciones y vuelve a probar!", "Wiederhole die Lektionen!"},
    [S_NEW_BEST] = {"NUOVO RECORD!", "NEW BEST!", "NUEVO RÉCORD!", "NEUER REKORD!"},
    [S_BEST] = {"Record: %1", "Best: %1", "Récord: %1", "Rekord: %1"},
    [S_LEVEL] = {"Livello %1", "Level %1", "Nivel %1", "Niveau %1"},
    [S_H_END] = {"SEL RICOMINCIA   HOME MENU", "SEL AGAIN   HOME MENU", "SEL OTRA VEZ   HOME MENÚ",
                 "SEL NOCHMAL   HOME MENÜ"},

    [S_NO_SCALE] = {"nessuna scala", "no scale", "sin escala", "keine Tonleiter"},
    [S_PLAY_SOMETHING] = {"Suona qualcosa!", "Play something!", "Toca algo!", "Spiel etwas!"},
    [S_LAB_INTRO] = {"Una nota: ti dico il nome. Due: l'intervallo. Tre o più: l'accordo e il rivolto. Con una "
                     "scala (SELECT) vedi i gradi e le note fuori scala.",
                     "One note: I name it. Two: the interval. Three or more: the chord and its inversion. Pick a "
                     "scale (SELECT) to see its degrees and the notes outside it.",
                     "Una nota: te digo su nombre. Dos: el intervalo. Tres o más: el acorde y su inversión. Con una "
                     "escala (SELECT) ves los grados y las notas fuera de ella.",
                     "Ein Ton: ich nenne ihn. Zwei: das Intervall. Drei oder mehr: Akkord und Umkehrung. Mit einer "
                     "Tonleiter (SELECT) siehst du die Stufen und fremde Töne."},
    [S_OCTAVE_MIDI] = {"Ottava %1  (MIDI %2)", "Octave %1  (MIDI %2)", "Octava %1  (MIDI %2)", "Oktave %1  (MIDI %2)"},
    [S_DEGREE_OF] = {"%1o grado della scala", "Degree %1 of the scale", "Grado %1 de la escala",
                     "%1. Stufe der Tonleiter"},
    [S_OUT_OF_SCALE] = {"Fuori scala!", "Not in the scale!", "Fuera de la escala!", "Nicht in der Tonleiter!"},
    [S_PLUS_OCTAVE] = {" + ottava", " + octave", " + octava", " + Oktave"},
    [S_NOTES] = {"Note: %1  (%2)", "Notes: %1  (%2)", "Notas: %1  (%2)", "Töne: %1  (%2)"},
    [S_DEGREE] = {"Grado ", "Degree ", "Grado ", "Stufe "},
    [S_UNKNOWN_CHORD] = {"Accordo non riconosciuto", "Unknown chord", "Acorde no reconocido", "Unbekannter Akkord"},
    [S_UNKNOWN_TIP] = {"Prova a togliere una nota, o a suonarle per terze (una sì, una no).",
                       "Try leaving out a note, or play them in thirds (skip one each time).",
                       "Prueba a quitar una nota, o tócalas por terceras (una sí, una no).",
                       "Lass einen Ton weg oder spiele in Terzen (jeden zweiten Ton)."},
    [S_OUT_RED] = {"Note fuori scala in rosso", "Notes outside the scale in red", "Fuera de la escala en rojo",
                   "Fremde Töne in Rot"},
    [S_LAST] = {"Ultime: ", "Last: ", "Últimas: ", "Zuletzt: "},
    [S_LAB_SEL1] = {"SELECT cambia scala, K1 la tonica", "SELECT changes the scale, K1 the root",
                    "SELECT cambia la escala, K1 la tónica", "SELECT: Tonleiter, K1: Grundton"},
    [S_LAB_SEL0] = {"SELECT: scegli una scala da vedere", "SELECT: pick a scale to show",
                    "SELECT: elige una escala para ver", "SELECT: wähle eine Tonleiter"},
    [S_H_LAB] = {"SELECT SCALA   K1 TONICA   HOME MENU", "SELECT SCALE   K1 ROOT   HOME MENU",
                 "SELECT ESCALA   K1 TÓNICA   HOME MENÚ", "SELECT TONLEITER   K1 GRUNDTON   HOME MENÜ"},

    [S_SET_LANG] = {"Lingua", "Language", "Idioma", "Sprache"},
    [S_SET_NAMES] = {"Nomi delle note", "Note names", "Nombres de notas", "Notennamen"},
    [S_SET_SOUND] = {"Suono", "Sound", "Sonido", "Klang"},
    [S_SET_SPEED] = {"Velocità esempi", "Example speed", "Velocidad ejemplos", "Tempo der Beispiele"},
    [S_SET_EXT] = {"Tastiera esterna", "External keyboard", "Teclado externo", "Externe Tastatur"},
    [S_SET_BLE] = {"Bluetooth", "Bluetooth", "Bluetooth", "Bluetooth"},
    [S_SET_DEVICE] = {"Piano Bluetooth", "Bluetooth piano", "Piano Bluetooth", "Bluetooth-Klavier"},
    [S_SET_RESET] = {"Azzera i record", "Reset best scores", "Borrar récords", "Rekorde löschen"},
    [S_EXT_BOTH] = {"suona anche qui", "also sounds here", "suena también aquí", "klingt auch hier"},
    [S_EXT_MUTE] = {"suona solo lei", "only it sounds", "solo suena él", "nur sie klingt"},
    [S_ON] = {"Acceso", "On", "Encendido", "An"},
    [S_OFF] = {"Spento", "Off", "Apagado", "Aus"},
    [S_NOT_BUILT] = {"non in questa build", "not in this build", "no en esta versión", "nicht in diesem Build"},
    [S_CONNECTED] = {"connesso %1", "connected %1", "conectado %1", "verbunden %1"},
    [S_SEARCHING] = {"cerco... (REC)", "searching... (REC)", "buscando... (REC)", "suche... (REC)"},
    [S_REC_SEARCH] = {"REC per cercare", "REC to search", "REC para buscar", "REC zum Suchen"},
    [S_SEL_RESET] = {"SEL per azzerare", "SEL to reset", "SEL para borrar", "SEL zum Löschen"},
    [S_H_SETTINGS] = {"SELECT VOCE   SEL O K1 CAMBIA   HOME ESCE E SALVA",
                      "SELECT ITEM   SEL OR K1 CHANGE   HOME SAVE AND EXIT",
                      "SELECT OPCIÓN   SEL O K1 CAMBIA   HOME GUARDA Y SALE",
                      "SELECT EINTRAG   SEL/K1 ÄNDERN   HOME SPEICHERN"},
    [S_SAY_RECORDS] = {"RECORD", "BEST SCORES", "RÉCORDS", "REKORDE"},
    [S_SAY_RESET] = {"AZZERATI", "RESET", "BORRADOS", "GELÖSCHT"},
    [S_SAY_OCTAVE] = {"OTTAVA", "OCTAVE", "OCTAVA", "OKTAVE"},
    [S_SAY_SAVED] = {"SALVATO", "SAVED", "GUARDADO", "GESPEICHERT"},
    [S_SAY_ERROR] = {"ERRORE", "ERROR", "ERROR", "FEHLER"},
    [S_SAY_SEARCHING] = {"CERCO...", "SEARCHING...", "BUSCANDO...", "SUCHE..."},
    [S_SAY_CONNECTING] = {"CONNETTO", "CONNECTING", "CONECTANDO", "VERBINDE"},
    [S_SAY_FOUND] = {"TROVATO", "FOUND", "ENCONTRADO", "GEFUNDEN"},
};
#define S(k) (STR[k][th_ui])

/* the lists */
static const char *const TOPIC_NAME[NLANGS][7] = {
    {"Le basi", "Intervalli", "Scale", "Modi", "Accordi", "Armonia", "Tonalità"},
    {"Basics", "Intervals", "Scales", "Modes", "Chords", "Harmony", "Keys"},
    {"Lo básico", "Intervalos", "Escalas", "Modos", "Acordes", "Armonía", "Tonalidades"},
    {"Grundlagen", "Intervalle", "Tonleitern", "Modi", "Akkorde", "Harmonik", "Tonarten"},
};
static const char *const TOPIC_SUB[NLANGS][7] = {
    {"note, toni, alterazioni", "distanze tra due note", "maggiore, minori, blues...", "i 7 modi greci",
     "triadi e settime", "accordi su ogni grado", "circolo delle quinte"},
    {"notes, steps, sharps", "distance between notes", "major, minors, blues...", "the 7 church modes",
     "triads and sevenths", "a chord on each degree", "circle of fifths"},
    {"notas, tonos, alteraciones", "distancia entre notas", "mayor, menores, blues...", "los 7 modos griegos",
     "tríadas y séptimas", "acordes en cada grado", "círculo de quintas"},
    {"Töne, Schritte, Vorzeichen", "Abstand zweier Töne", "Dur, Moll, Blues...", "die 7 Kirchentonarten",
     "Drei- und Vierklänge", "Akkorde jeder Stufe", "Quintenzirkel"},
};
static const char *const QT_NAMES[NLANGS][10] = {
    {"Misto", "Nomi delle note", "Intervalli", "Scale", "Modi", "Accordi", "Gradi della scala",
     "Orecchio: intervalli", "Orecchio: accordi", "Tonalità"},
    {"Mixed", "Note names", "Intervals", "Scales", "Modes", "Chords", "Scale degrees", "Ear: intervals",
     "Ear: chords", "Keys"},
    {"Mixto", "Nombres de notas", "Intervalos", "Escalas", "Modos", "Acordes", "Grados de la escala",
     "Oído: intervalos", "Oído: acordes", "Tonalidades"},
    {"Gemischt", "Notennamen", "Intervalle", "Tonleitern", "Modi", "Akkorde", "Stufen", "Gehör: Intervalle",
     "Gehör: Akkorde", "Tonarten"},
};
static const char *const LEVEL_NAMES[NLANGS][3] = {
    {"1 Facile", "2 Medio", "3 Difficile"}, {"1 Easy", "2 Medium", "3 Hard"},
    {"1 Fácil", "2 Medio", "3 Difícil"}, {"1 Leicht", "2 Mittel", "3 Schwer"},
};
static const char *const INV_NAME[NLANGS][4] = {
    {"stato fondamentale", "1o rivolto", "2o rivolto", "3o rivolto"},
    {"root position", "1st inversion", "2nd inversion", "3rd inversion"},
    {"estado fundamental", "1.a inversión", "2.a inversión", "3.a inversión"},
    {"Grundstellung", "1. Umkehrung", "2. Umkehrung", "3. Umkehrung"},
};
static const char *const INV_WHY[NLANGS][4] = {
    {"la fondamentale è al basso", "la 3a è al basso", "la 5a è al basso", "la 7a è al basso"},
    {"the root is in the bass", "the 3rd is in the bass", "the 5th is in the bass", "the 7th is in the bass"},
    {"la fundamental está en el bajo", "la tercera está en el bajo", "la quinta está en el bajo",
     "la séptima está en el bajo"},
    {"der Grundton liegt im Bass", "die Terz liegt im Bass", "die Quinte liegt im Bass", "die Septime liegt im Bass"},
};
static const char *const SOUND_NAMES[NLANGS][3] = {
    {"Piano elettrico", "Organo", "Campane"}, {"Electric piano", "Organ", "Bells"},
    {"Piano eléctrico", "Órgano", "Campanas"}, {"E-Piano", "Orgel", "Glocken"},
};
static const char *const SPEED_NAMES[NLANGS][3] = {
    {"Lenta", "Media", "Veloce"}, {"Slow", "Medium", "Fast"}, {"Lenta", "Media", "Rápida"},
    {"Langsam", "Mittel", "Schnell"},
};
/* each language by its own name, and the note-name systems */
static const char *const LANG_SELF[NLANGS] = {"Italiano", "English", "Español", "Deutsch"};
static const char *const NAMES_SELF[NNAMES] = {"Do (C)", "Do Re Mi", "C D E", "C D E ... H"};

/* the basics, a page each: its title and text in every language, the notes it shows, what PLAY plays */
typedef struct {
    const char *title[NLANGS], *text[NLANGS];
    uint16_t mask, root;
    uint8_t play[8], nplay, together;
} page_t;
#define PCB(x) (1u << (x))
#define WHITE_PCS (PCB(0) | PCB(2) | PCB(4) | PCB(5) | PCB(7) | PCB(9) | PCB(11))
#define BLACK_PCS (PCB(1) | PCB(3) | PCB(6) | PCB(8) | PCB(10))
static const page_t BASICS[8] = {
    {{"Le 7 note", "The 7 notes", "Las 7 notas", "Die 7 Stammtöne"},
     {"Le note sono sette: Do Re Mi Fa Sol La Si (in lettere: C D E F G A B). Sono i tasti bianchi. Dopo il Si "
      "ricomincia il Do, più in alto: è l'ottava. PLAY: la scala di Do.",
      "There are seven notes: C D E F G A B (in solfège: Do Re Mi Fa Sol La Si). They are the white keys. After B "
      "comes C again, higher up: the octave. PLAY: the C scale.",
      "Las notas son siete: Do Re Mi Fa Sol La Si (en letras: C D E F G A B). Son las teclas blancas. Después del Si "
      "vuelve el Do, más agudo: es la octava. PLAY: la escala de Do.",
      "Es gibt sieben Stammtöne: C D E F G A H (Do Re Mi Fa Sol La Si). Das sind die weißen Tasten. Nach H kommt "
      "wieder C, höher: die Oktave. PLAY: die C-Dur-Tonleiter."},
     WHITE_PCS, PCB(0), {60, 62, 64, 65, 67, 69, 71, 72}, 8, 0},
    {{"Tono e semitono", "Tone and semitone", "Tono y semitono", "Ganzton und Halbton"},
     {"Il semitono è la distanza più piccola: da un tasto a quello subito accanto, bianco o nero. Un tono = 2 "
      "semitoni. Tra Mi-Fa e Si-Do non c'è il tasto nero: sono semitoni.",
      "A semitone is the smallest step: from one key to the very next, white or black. A tone = 2 semitones. "
      "Between E-F and B-C there is no black key: they are semitones.",
      "El semitono es la distancia más pequeña: de una tecla a la de al lado, blanca o negra. Un tono = 2 "
      "semitonos. Entre Mi-Fa y Si-Do no hay tecla negra: son semitonos.",
      "Der Halbton ist der kleinste Schritt: von einer Taste zur nächsten, weiß oder schwarz. Ein Ganzton = 2 "
      "Halbtöne. Zwischen E-F und H-C gibt es keine schwarze Taste: das sind Halbtöne."},
     PCB(4) | PCB(5) | PCB(11) | PCB(0), PCB(4), {64, 65, 71, 72}, 4, 0},
    {{"Diesis e bemolle", "Sharps and flats", "Sostenidos y bemoles", "Kreuz und b"},
     {"Il diesis (#) alza una nota di un semitono, il bemolle (b) la abbassa. Ogni tasto nero ha due nomi: Do# = "
      "Reb. Sono note 'enarmoniche': stesso suono, nome diverso.",
      "A sharp (#) raises a note by a semitone, a flat (b) lowers it. Every black key has two names: C# = Db. They "
      "are 'enharmonic': same sound, different name.",
      "El sostenido (#) sube una nota un semitono, el bemol (b) la baja. Cada tecla negra tiene dos nombres: Do# = "
      "Reb. Son notas 'enarmónicas': mismo sonido, distinto nombre.",
      "Ein Kreuz (#) erhöht einen Ton um einen Halbton (C wird Cis), ein b erniedrigt ihn (D wird Des). Jede "
      "schwarze Taste hat zwei Namen: Cis = Des. Das heißt 'enharmonisch'."},
     BLACK_PCS, PCB(1), {60, 61, 62, 63, 64}, 5, 0},
    {{"La tastiera dell'FM-1", "The FM-1 keyboard", "El teclado del FM-1", "Die FM-1-Tastatur"},
     {"27 tasti: 16 bianchi e 11 neri, da Fa a Sol (più di 2 ottave). Il Do è il quinto tasto bianco. OCT- e OCT+ "
      "spostano il suono di un'ottava. Funziona anche una tastiera MIDI (USB, jack, Bluetooth).",
      "27 keys: 16 white and 11 black, from F to G (over 2 octaves). C is the 5th white key. OCT- and OCT+ shift "
      "the sound by an octave. A MIDI keyboard works too (USB, jack, Bluetooth).",
      "27 teclas: 16 blancas y 11 negras, de Fa a Sol (más de 2 octavas). El Do es la quinta tecla blanca. OCT- y "
      "OCT+ mueven el sonido una octava. También funciona un teclado MIDI (USB, jack, Bluetooth).",
      "27 Tasten: 16 weiße und 11 schwarze, von F bis G (über 2 Oktaven). C ist die 5. weiße Taste. OCT- und OCT+ "
      "verschieben den Klang um eine Oktave. Auch ein MIDI-Keyboard geht (USB, Klinke, Bluetooth)."},
     PCB(0), PCB(0), {48, 60, 72}, 3, 0},
    {{"La scala", "The scale", "La escala", "Die Tonleiter"},
     {"Una serie di note in ordine, con una sequenza fissa di toni (T) e semitoni (S). La maggiore è: T T S T T T "
      "S. Le note della scala sono i 'gradi': I II III IV V VI VII.",
      "A row of notes in order, with a fixed pattern of tones (T) and semitones (S). The major scale is: T T S T T "
      "T S. The notes of a scale are its 'degrees': I II III IV V VI VII.",
      "Una serie de notas en orden, con una secuencia fija de tonos (T) y semitonos (S). La mayor es: T T S T T T "
      "S. Las notas de la escala son los 'grados': I II III IV V VI VII.",
      "Eine Reihe von Tönen mit festem Muster aus Ganztönen (G) und Halbtönen (H). Dur ist: G G H G G G H. Die "
      "Töne heißen 'Stufen': I II III IV V VI VII."},
     WHITE_PCS, PCB(0), {60, 62, 64, 65, 67, 69, 71, 72}, 8, 0},
    {{"L'intervallo", "The interval", "El intervalo", "Das Intervall"},
     {"La distanza tra due note. Ha un numero (2a, 3a, 5a: quante note si contano) e una qualità (maggiore, "
      "minore, giusta). Do-Mi = 3a maggiore, 4 semitoni.",
      "The distance between two notes. It has a number (2nd, 3rd, 5th: how many letters you count) and a quality "
      "(major, minor, perfect). C-E = major 3rd, 4 semitones.",
      "La distancia entre dos notas. Tiene un número (segunda, tercera, quinta: cuántas notas se cuentan) y una "
      "calidad (mayor, menor, justa). Do-Mi = tercera mayor, 4 semitonos.",
      "Der Abstand zweier Töne. Es hat eine Zahl (Sekunde, Terz, Quinte: wie viele Stufen man zählt) und eine Art "
      "(groß, klein, rein). C-E = große Terz, 4 Halbtöne."},
     PCB(0) | PCB(4), PCB(0), {60, 64}, 2, 0},
    {{"L'accordo", "The chord", "El acorde", "Der Akkord"},
     {"Tre o più note insieme. La triade si fa 'saltando' una nota della scala: 1-3-5 (Do Mi Sol). Se la 3a è "
      "maggiore l'accordo è maggiore, se è minore è minore.",
      "Three or more notes together. A triad skips every other note of the scale: 1-3-5 (C E G). If the 3rd is "
      "major the chord is major; if it is minor, the chord is minor.",
      "Tres o más notas juntas. La tríada se forma 'saltando' una nota de la escala: 1-3-5 (Do Mi Sol). Si la "
      "tercera es mayor el acorde es mayor; si es menor, es menor.",
      "Drei oder mehr Töne zusammen. Den Dreiklang baut man aus jedem zweiten Ton der Tonleiter: 1-3-5 (C E G). "
      "Große Terz: Dur. Kleine Terz: Moll."},
     PCB(0) | PCB(4) | PCB(7), PCB(0), {60, 64, 67}, 3, 1},
    {{"La tonalità", "The key", "La tonalidad", "Die Tonart"},
     {"La scala 'di casa' di un brano. La tonica è il punto di riposo: la melodia ci torna alla fine. Le "
      "alterazioni in chiave dicono quali note sono # o b (lezione Tonalità).",
      "The 'home' scale of a piece. The tonic is the point of rest: the melody comes back to it at the end. The key "
      "signature tells which notes are # or b (lesson Keys).",
      "La escala 'de casa' de una pieza. La tónica es el punto de reposo: la melodía vuelve a ella al final. La "
      "armadura dice qué notas son # o b (lección Tonalidades).",
      "Die 'Heimat'-Tonleiter eines Stücks. Der Grundton ist der Ruhepunkt: die Melodie kehrt am Ende zu ihm "
      "zurück. Die Vorzeichen zeigen, welche Töne # oder b sind (Lektion Tonarten)."},
     WHITE_PCS, PCB(0), {67, 71, 74, 72}, 4, 0},
};

/* a template with %1 %2 %3 filled in */
static void t1_fmt(char *d, const char *t, const char *a, const char *b, const char *c)
{
    char *o = d;
    for (; *t; t++) {
        if (t[0] == '%' && t[1] >= '1' && t[1] <= '3') {
            const char *s = t[1] == '1' ? a : t[1] == '2' ? b : c;
            while (s && *s)
                *o++ = *s++;
            t++;
        } else {
            *o++ = *t;
        }
    }
    *o = 0;
}
