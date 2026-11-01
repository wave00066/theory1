#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""THEORY-1's SYNTH section: its chapters, pages, steps and words in five languages, written into
app/synth_lessons.h (and the host test that visits every page, app/tests/synth.txt).

    python3 app/tools/gen_synth.py

A chapter starts from a patch (INIT and its changes) and has pages:
  T  a page to read: a picture and a text
  P  a page to try: the text says what K1..K4 move, the picture follows them
  S  a step: like P, with a goal on one knob (a parameter, a comparison, a value); reached, it turns green
Every page may set parameters when it is shown ("set"). In a chapter that builds a sound ("build"), a step also
sets the goals of the steps before it, so a page reached directly sounds as it would have.
"""
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
APP = os.path.dirname(HERE)
LANGS = 5   # it en es de fr


def t(ms):
    """a time knob's position for a time in ms (synthmath.h syn_ms: 10^(x/25) ms)"""
    return max(0, min(100, round(25 * math.log10(ms))))


def cut(hz):
    """the cutoff knob for a frequency (syn_cutoff: 30 * 2^(0.094 x))"""
    return max(0, min(100, round(math.log2(hz / 30) / 0.094)))


def lfo(hz):
    """the LFO's rate knob (syn_lfo_hz: 0.05 * 2^(0.086439 x))"""
    return max(0, min(100, round(math.log2(hz / 0.05) / 0.086439)))


PARAMS = ["ENGINE", "WAVE", "PW", "OCT", "OSC2", "WAVE2", "DETUNE", "NOISE", "CUTOFF", "RESO", "FTYPE", "KTRACK",
          "A", "D", "S", "R", "FA", "FD", "FS", "FR", "FAMT", "PAMT", "PDEC", "LRATE", "LDEPTH", "LDEST", "LSHAPE",
          "RATIO", "INDEX", "MDEC", "FB", "H1", "H2", "H3", "H4", "H5", "H6", "H7", "H8", "WTPOS", "POLY", "GLIDE",
          "DRIVE", "CHORUS", "VOL", "VELAMT"]
VIRTUAL = ["HSEL", "HLVL"]           # additive: the harmonic chosen, its level
SINE, TRI, SAW, SQUARE, PULSE, NOISE = range(6)
LP, BP, HP = range(3)
L_PITCH, L_CUTOFF, L_AMP, L_PW, L_WT = range(5)
SH = 4
E_SUB, E_FM, E_ADD, E_WT = range(4)
RATIO = [0.5, 1, 1.41, 2, 3, 3.5, 4, 5, 7, 14]

INIT = dict(ENGINE=E_SUB, WAVE=SAW, PW=50, OCT=0, OSC2=0, WAVE2=SAW, DETUNE=0, NOISE=0, CUTOFF=100, RESO=0,
            FTYPE=LP, KTRACK=50, A=0, D=t(300), S=100, R=t(20), FA=0, FD=t(300), FS=0, FR=t(300), FAMT=0, PAMT=0,
            PDEC=t(40), LRATE=lfo(5.5), LDEPTH=0, LDEST=L_PITCH, LSHAPE=0, RATIO=1, INDEX=0, MDEC=100, FB=0,
            H1=100, H2=0, H3=0, H4=0, H5=0, H6=0, H7=0, H8=0, WTPOS=0, POLY=0, GLIDE=0, DRIVE=0, CHORUS=0, VOL=80,
            VELAMT=0)

# ------------------------------------------------------------------ words ---
# each: (it, en, es, de, fr)
MODULES = [
    ("Le fondamenta", "The basics", "Los fundamentos", "Die Grundlagen", "Les bases"),
    ("Inviluppi e modulazioni", "Envelopes and modulation", "Envolventes y modulación", "Hüllkurven und Modulation",
     "Enveloppes et modulations"),
    ("Tecniche di sintesi", "Synthesis techniques", "Técnicas de síntesis", "Synthesearten", "Techniques de synthèse"),
    ("Voci ed effetti", "Voices and effects", "Voces y efectos", "Stimmen und Effekte", "Voix et effets"),
    ("Creiamo i suoni insieme", "Let's make sounds together", "Creemos sonidos juntos", "Klänge gemeinsam bauen",
     "Créons des sons ensemble"),
    ("Il tuo synth", "Your synth", "Tu sintetizador", "Dein Synth", "Ton synthé"),
]

PNAME = {
    "ENGINE": ("Motore", "Engine", "Motor", "Synthese", "Moteur"),
    "WAVE": ("Onda", "Wave", "Onda", "Welle", "Onde"),
    "PW": ("PW", "PW", "PW", "PW", "PW"),
    "OCT": ("Ottava", "Octave", "Octava", "Oktave", "Octave"),
    "OSC2": ("Osc 2", "Osc 2", "Osc 2", "Osz 2", "Osc 2"),
    "WAVE2": ("Onda 2", "Wave 2", "Onda 2", "Welle 2", "Onde 2"),
    "DETUNE": ("Detune", "Detune", "Detune", "Detune", "Detune"),
    "NOISE": ("Rumore", "Noise", "Ruido", "Rauschen", "Bruit"),
    "CUTOFF": ("Cutoff", "Cutoff", "Cutoff", "Cutoff", "Cutoff"),
    "RESO": ("Risonanza", "Resonance", "Resonancia", "Resonanz", "Résonance"),
    "FTYPE": ("Tipo", "Type", "Tipo", "Typ", "Type"),
    "KTRACK": ("Key track", "Key track", "Key track", "Key track", "Key track"),
    "A": ("Attack", "Attack", "Attack", "Attack", "Attack"),
    "D": ("Decay", "Decay", "Decay", "Decay", "Decay"),
    "S": ("Sustain", "Sustain", "Sustain", "Sustain", "Sustain"),
    "R": ("Release", "Release", "Release", "Release", "Release"),
    "FA": ("F.Attack", "F.Attack", "F.Attack", "F.Attack", "F.Attack"),
    "FD": ("F.Decay", "F.Decay", "F.Decay", "F.Decay", "F.Decay"),
    "FS": ("F.Sustain", "F.Sustain", "F.Sustain", "F.Sustain", "F.Sustain"),
    "FR": ("F.Release", "F.Release", "F.Release", "F.Release", "F.Release"),
    "FAMT": ("Amount", "Amount", "Amount", "Amount", "Amount"),
    "PAMT": ("Pitch amt", "Pitch amt", "Pitch amt", "Pitch amt", "Pitch amt"),
    "PDEC": ("Pitch dec", "Pitch dec", "Pitch dec", "Pitch dec", "Pitch dec"),
    "LRATE": ("Velocità", "Rate", "Velocidad", "Tempo", "Vitesse"),
    "LDEPTH": ("Profondità", "Depth", "Profund.", "Tiefe", "Profond."),
    "LDEST": ("Destinaz.", "Target", "Destino", "Ziel", "Cible"),
    "LSHAPE": ("Forma", "Shape", "Forma", "Form", "Forme"),
    "RATIO": ("Ratio", "Ratio", "Ratio", "Ratio", "Ratio"),
    "INDEX": ("Indice", "Index", "Índice", "Index", "Indice"),
    "MDEC": ("Decay mod", "Mod decay", "Decay mod", "Mod-Decay", "Decay mod"),
    "FB": ("Feedback", "Feedback", "Feedback", "Feedback", "Feedback"),
    "WTPOS": ("Posizione", "Position", "Posición", "Position", "Position"),
    "POLY": ("Voci", "Voices", "Voces", "Stimmen", "Voix"),
    "GLIDE": ("Glide", "Glide", "Glide", "Glide", "Glide"),
    "DRIVE": ("Drive", "Drive", "Drive", "Drive", "Drive"),
    "CHORUS": ("Chorus", "Chorus", "Chorus", "Chorus", "Chorus"),
    "VOL": ("Volume", "Volume", "Volumen", "Lautst.", "Volume"),
    "VELAMT": ("Vel>amt", "Vel>amt", "Vel>amt", "Vel>amt", "Vél>amt"),
    "HSEL": ("Armonica", "Harmonic", "Armónico", "Oberton", "Harmon."),
    "HLVL": ("Livello", "Level", "Nivel", "Pegel", "Niveau"),
}
for h in range(1, 9):
    PNAME["H%d" % h] = tuple(n + " %d" % h for n in ("Arm.", "Harm.", "Arm.", "Ober.", "Harm."))

ENUMS = {
    "WAVE": [("Sinus", "Sine", "Seno", "Sinus", "Sinus"), ("Triangolo", "Triangle", "Triángulo", "Dreieck", "Triangle"),
             ("Dente sega", "Saw", "Sierra", "Sägezahn", "Scie"), ("Quadra", "Square", "Cuadrada", "Rechteck", "Carré"),
             ("Impulso", "Pulse", "Pulso", "Puls", "Impulsion"), ("Rumore", "Noise", "Ruido", "Rauschen", "Bruit")],
    "FTYPE": [("LP", "LP", "LP", "LP", "LP"), ("BP", "BP", "BP", "BP", "BP"), ("HP", "HP", "HP", "HP", "HP")],
    "LDEST": [("Altezza", "Pitch", "Altura", "Tonhöhe", "Hauteur"), ("Filtro", "Filter", "Filtro", "Filter", "Filtre"),
              ("Volume", "Volume", "Volumen", "Lautst.", "Volume"), ("PW", "PW", "PW", "PW", "PW"),
              ("Posizione", "Position", "Posición", "Position", "Position")],
    "LSHAPE": [("Sinus", "Sine", "Seno", "Sinus", "Sinus"), ("Triang.", "Tri", "Triáng.", "Dreieck", "Triang."),
               ("Sega", "Saw", "Sierra", "Säge", "Scie"), ("Quadra", "Square", "Cuadr.", "Recht.", "Carré"),
               ("S&H", "S&H", "S&H", "S&H", "S&H")],
    "ENGINE": [("Sottrattiva", "Subtract.", "Sustractiva", "Subtraktiv", "Soustract."), ("FM", "FM", "FM", "FM", "FM"),
               ("Additiva", "Additive", "Aditiva", "Additiv", "Additive"),
               ("Wavetable", "Wavetable", "Wavetable", "Wavetable", "Wavetable")],
    "POLY": [("Poly", "Poly", "Poly", "Poly", "Poly"), ("Mono", "Mono", "Mono", "Mono", "Mono")],
}

# the screen's other words
SS = [
    ("MENU", ("SINTESI", "SYNTH", "SÍNTESIS", "SYNTHESE", "SYNTHÈSE")),
    ("MENU_SUB", ("come funziona un synth: gira e ascolta", "how a synth works: turn and listen",
                  "cómo funciona un sintetizador", "wie ein Synth funktioniert", "comment marche un synthé")),
    ("H_PAGE", ("SELECT PAGINA  SEL AVANTI  PLAY ESEMPIO  FX DA CAPO", "SELECT PAGE  SEL NEXT  PLAY EXAMPLE  FX RESTART",
                "SELECT PÁGINA  SEL SIGUE  PLAY EJEMPLO  FX REINICIA", "SELECT SEITE  SEL WEITER  PLAY BEISPIEL  FX NEU",
                "SELECT PAGE  SEL SUITE  PLAY EXEMPLE  FX REPART")),
    ("GOAL", ("Obiettivo", "Goal", "Objetivo", "Ziel", "Objectif")),
    ("DONE", ("Fatto! SEL: avanti", "Done! SEL: next", "Hecho. SEL: sigue", "Geschafft! SEL: weiter", "Bravo ! SEL : suite")),
    ("RESTART", ("Da capo", "Restart", "De nuevo", "Neustart", "On repart")),
    ("END", ("Fine del capitolo", "End of chapter", "Fin del capítulo", "Kapitelende", "Fin du chapitre")),
    ("PERIOD", ("periodo", "period", "periodo", "Periode", "période")),
    ("AMPL", ("ampiezza", "amplitude", "amplitud", "Amplitude", "amplitude")),
    ("TIME", ("tempo", "time", "tiempo", "Zeit", "temps")),
    ("FUND", ("fondamentale", "fundamental", "fundamental", "Grundton", "fondamentale")),
    ("HARMS", ("armoniche", "harmonics", "armónicos", "Obertöne", "harmoniques")),
    ("OSC", ("OSC", "OSC", "OSC", "OSZ", "OSC")),
    ("FILTER", ("FILTRO", "FILTER", "FILTRO", "FILTER", "FILTRE")),
    ("AMP", ("AMP", "AMP", "AMP", "VERST.", "AMP")),
    ("OUT", ("USCITA", "OUT", "SALIDA", "AUSGANG", "SORTIE")),
    ("ENV", ("ENV", "ENV", "ENV", "ENV", "ENV")),
    ("LFO", ("LFO", "LFO", "LFO", "LFO", "LFO")),
    ("KEYS", ("TASTI", "KEYS", "TECLAS", "TASTEN", "TOUCHES")),
    ("CARRIER", ("portante", "carrier", "portadora", "Träger", "porteuse")),
    ("MODULATOR", ("modulante", "modulator", "modulador", "Modulator", "modulateur")),
    ("STACK", ("pila", "stack", "pila", "Stapel", "pile")),
    ("PARALLEL", ("parallelo", "parallel", "paralelo", "parallel", "parallèle")),
    ("FEEDBACK", ("feedback", "feedback", "feedback", "Feedback", "feedback")),
    ("KEYDOWN", ("tasto premuto", "key held", "tecla pulsada", "Taste gedrückt", "touche enfoncée")),
    ("ORGAN", ("Organo", "Organ", "Órgano", "Orgel", "Orgue")),
    ("PIANO", ("Pianoforte", "Piano", "Piano", "Klavier", "Piano")),
    ("STRINGS", ("Archi", "Strings", "Cuerdas", "Streicher", "Cordes")),
    ("DRUM", ("Percussione", "Drum", "Percusión", "Schlagzeug", "Percussion")),
    ("SUM", ("somma", "sum", "suma", "Summe", "somme")),
    ("POSITION", ("posizione", "position", "posición", "Position", "position")),
    ("NOGLIDE", ("senza glide", "no glide", "sin glide", "ohne Glide", "sans glide")),
    ("GLIDE", ("con glide", "with glide", "con glide", "mit Glide", "avec glide")),
    ("PITCH", ("altezza", "pitch", "altura", "Tonhöhe", "hauteur")),
    ("HOLD", ("fisso", "hold", "fijo", "halten", "tenu")),
    ("OFF", ("off", "off", "off", "aus", "off")),
    ("IN", ("ingresso", "in", "entrada", "Eingang", "entrée")),
    ("LEVEL", ("livello", "level", "nivel", "Pegel", "niveau")),
    ("SYNTH", ("SYNTH", "SYNTH", "SYNTH", "SYNTH", "SYNTH")),
    ("DELAY", ("DELAY", "DELAY", "DELAY", "DELAY", "DELAY")),
    ("REVERB", ("REVERB", "REVERB", "REVERB", "HALL", "REVERB")),
]

# -------------------------------------------------------------- chapters ---
# page: (kind, picture, knobs, goal, text, set)   goal: (param, op, value, tolerance) op: eq ge le
def T(diag, text, set=None):
    return ("T", diag, [], None, text, set or {})


def P(diag, knobs, text, set=None):
    return ("P", diag, knobs, None, text, set or {})


def S(diag, knobs, goal, text, set=None):
    return ("S", diag, knobs, goal, text, set or {})


CHAPTERS = []


def chapter(module, title, demo, patch, pages, build=False, keep=False):
    CHAPTERS.append(dict(module=module, title=title, demo=demo, patch=patch, pages=pages, build=build, keep=keep))


# 1 --- sound
chapter(0, ("Il suono", "Sound", "El sonido", "Der Klang", "Le son"), "note", dict(WAVE=SINE), [
    T("SOUND", (
        "Il suono è aria che vibra. Un altoparlante spinge e tira l'aria avanti e indietro, e l'orecchio sente queste "
        "vibrazioni. Tre cose descrivono ogni suono: altezza, volume e timbro.",
        "Sound is air that vibrates. A speaker pushes and pulls the air back and forth, and your ear feels these "
        "vibrations. Three things describe every sound: pitch, volume and timbre.",
        "El sonido es aire que vibra. Un altavoz empuja y tira del aire adelante y atrás, y el oído siente estas "
        "vibraciones. Tres cosas describen cada sonido: altura, volumen y timbre.",
        "Schall ist Luft, die schwingt. Ein Lautsprecher drückt und zieht die Luft hin und her, und das Ohr spürt diese "
        "Schwingungen. Drei Dinge beschreiben jeden Klang: Tonhöhe, Lautstärke und Klangfarbe.",
        "Le son, c'est de l'air qui vibre. Un haut-parleur pousse et tire l'air d'avant en arrière, et l'oreille sent "
        "ces vibrations. Trois choses décrivent chaque son : la hauteur, le volume et le timbre.")),
    T("SOUND", (
        "Altezza = frequenza: quante vibrazioni al secondo, in hertz (Hz). Il La centrale fa 440 Hz; con il doppio, "
        "880 Hz, sali di un'ottava. Volume = ampiezza: quanto è grande la vibrazione.",
        "Pitch = frequency: how many vibrations per second, in hertz (Hz). The A above middle C is 440 Hz; double it, "
        "880 Hz, and you go up an octave. Volume = amplitude: how big the vibration is.",
        "Altura = frecuencia: cuántas vibraciones por segundo, en hercios (Hz). El La central tiene 440 Hz; con el "
        "doble, 880 Hz, subes una octava. Volumen = amplitud: lo grande que es la vibración.",
        "Tonhöhe = Frequenz: wie viele Schwingungen pro Sekunde, in Hertz (Hz). Das A über dem mittleren C hat 440 Hz; "
        "das Doppelte, 880 Hz, ist eine Oktave höher. Lautstärke = Amplitude: wie groß die Schwingung ist.",
        "Hauteur = fréquence : combien de vibrations par seconde, en hertz (Hz). Le La du milieu fait 440 Hz ; le "
        "double, 880 Hz, est une octave plus haut. Volume = amplitude : la taille de la vibration.")),
    T("WAVE", (
        "Il timbro è il colore del suono. Ogni nota ha una frequenza base, la fondamentale, più tante armoniche: 2, 3, "
        "4 volte più acute. Quali armoniche ci sono, e quanto forti, decide il timbro.",
        "Timbre is the colour of a sound. Every note has a base frequency, the fundamental, plus many harmonics: 2, 3, "
        "4 times higher. Which harmonics are there, and how loud, makes the timbre.",
        "El timbre es el color del sonido. Cada nota tiene una frecuencia base, la fundamental, más muchos armónicos: "
        "2, 3, 4 veces más agudos. Qué armónicos hay, y con qué fuerza, decide el timbre.",
        "Die Klangfarbe ist die Farbe des Klangs. Jeder Ton hat eine Grundfrequenz, den Grundton, und viele Obertöne: "
        "2, 3, 4 Mal so hoch. Welche Obertöne da sind und wie laut, macht die Klangfarbe.",
        "Le timbre, c'est la couleur du son. Chaque note a une fréquence de base, la fondamentale, plus beaucoup "
        "d'harmoniques : 2, 3, 4 fois plus aiguës. Lesquelles sont là, et à quel volume, fait le timbre."),
      set=dict(WAVE=SAW)),
    P("SOUND", ["WAVE", "VOL", "OCT"], (
        "Suona un tasto. K1 cambia la forma d'onda, K2 il volume, K3 l'ottava: guarda come cambia l'onda.",
        "Play a key. K1 changes the waveform, K2 the volume, K3 the octave: watch how the wave changes.",
        "Toca una tecla. K1 cambia la forma de onda, K2 el volumen, K3 la octava: mira cómo cambia la onda.",
        "Spiel eine Taste. K1 ändert die Wellenform, K2 die Lautstärke, K3 die Oktave: sieh, wie sich die Welle ändert.",
        "Joue une touche. K1 change la forme d'onde, K2 le volume, K3 l'octave : regarde comment l'onde change."),
      set=dict(WAVE=SINE)),
    S("WAVE", ["WAVE", "VOL", "OCT"], ("WAVE", "eq", SAW, 0), (
        "Gira K1 fino al dente di sega: stessa nota, ma con tutte le armoniche. Il suono diventa ronzante e pieno.",
        "Turn K1 to the saw wave: same note, but with every harmonic. The sound turns buzzy and full.",
        "Gira K1 hasta la onda de sierra: la misma nota, pero con todos los armónicos. El sonido se vuelve zumbante "
        "y lleno.",
        "Dreh K1 bis zur Sägezahnwelle: derselbe Ton, aber mit allen Obertönen. Der Klang wird summend und voll.",
        "Tourne K1 jusqu'à l'onde en dents de scie : même note, mais avec toutes les harmoniques. Le son devient "
        "bourdonnant et plein."), set=dict(WAVE=SINE)),
    S("SOUND", ["WAVE", "VOL", "OCT"], ("VOL", "le", 40, 0), (
        "Ora abbassa il volume con K2 fino al 40%: l'onda diventa più piccola, ma la nota resta la stessa.",
        "Now turn the volume down with K2 to 40%: the wave gets smaller, but the note stays the same.",
        "Ahora baja el volumen con K2 hasta el 40%: la onda se hace más pequeña, pero la nota sigue igual.",
        "Dreh jetzt mit K2 die Lautstärke auf 40%: die Welle wird kleiner, aber der Ton bleibt gleich.",
        "Baisse maintenant le volume avec K2 jusqu'à 40% : l'onde devient plus petite, mais la note reste la même."),
      set=dict(VOL=80)),
])

# 2 --- the synthesizer
chapter(0, ("Il sintetizzatore", "The synthesizer", "El sintetizador", "Der Synthesizer", "Le synthétiseur"), "scale",
        dict(CUTOFF=60), [
    T("BLOCKS", (
        "Un sintetizzatore crea il suono da zero. Il percorso è sempre lo stesso: l'OSCILLATORE crea un'onda ricca, il "
        "FILTRO toglie una parte delle armoniche, l'AMPLIFICATORE decide il volume.",
        "A synthesizer makes sound from nothing. The path is always the same: the OSCILLATOR makes a rich wave, the "
        "FILTER takes some harmonics away, the AMPLIFIER sets the volume.",
        "Un sintetizador crea el sonido desde cero. El camino es siempre el mismo: el OSCILADOR crea una onda rica, el "
        "FILTRO quita parte de los armónicos, el AMPLIFICADOR decide el volumen.",
        "Ein Synthesizer macht Klang aus dem Nichts. Der Weg ist immer gleich: der OSZILLATOR macht eine reiche Welle, "
        "der FILTER nimmt Obertöne weg, der VERSTÄRKER bestimmt die Lautstärke.",
        "Un synthétiseur crée le son à partir de rien. Le chemin est toujours le même : l'OSCILLATEUR crée une onde "
        "riche, le FILTRE enlève une partie des harmoniques, l'AMPLIFICATEUR règle le volume.")),
    T("BLOCKS2", (
        "Altri blocchi non suonano: muovono gli altri. L'INVILUPPO è una curva che parte quando premi un tasto; l'LFO è "
        "un'oscillazione lenta. La tastiera dice quale nota e quando.",
        "Other blocks make no sound: they move the others. The ENVELOPE is a curve that starts when you press a key; "
        "the LFO is a slow wobble. The keyboard says which note, and when.",
        "Otros bloques no suenan: mueven a los demás. La ENVOLVENTE es una curva que empieza al pulsar una tecla; el "
        "LFO es una oscilación lenta. El teclado dice qué nota y cuándo.",
        "Andere Blöcke klingen nicht: sie bewegen die anderen. Die HÜLLKURVE ist eine Kurve, die beim Tastendruck "
        "startet; der LFO ist ein langsames Schwingen. Die Tastatur sagt, welcher Ton und wann.",
        "D'autres blocs ne sonnent pas : ils font bouger les autres. L'ENVELOPPE est une courbe qui part quand tu "
        "appuies sur une touche ; le LFO est une oscillation lente. Le clavier dit quelle note, et quand.")),
    T("NONE", (
        "Ogni synth, dal più grande al più piccolo, è fatto con questi blocchi. Se impari a riconoscerli, sai usare "
        "qualsiasi synth. Qui l'FM-1 diventa un synth didattico: le manopole K1, K2, K3 e K4 muovono i parametri, i "
        "tasti suonano, PLAY fa sentire un esempio e FX riporta il suono all'inizio del capitolo.",
        "Every synth, from the biggest to the smallest, is built from these blocks. Learn to spot them and you can use "
        "any synth. Here the FM-1 becomes a teaching synth: the knobs K1, K2, K3 and K4 move the settings, the keys "
        "play, PLAY plays an example and FX takes the sound back to the start of the chapter.",
        "Todos los sintetizadores, del más grande al más pequeño, están hechos con estos bloques. Si aprendes a "
        "reconocerlos, sabrás usar cualquiera. Aquí el FM-1 se vuelve un sintetizador didáctico: las perillas K1, K2, "
        "K3 y K4 mueven los parámetros, las teclas suenan, PLAY toca un ejemplo y FX devuelve el sonido al inicio del "
        "capítulo.",
        "Jeder Synthesizer, vom größten bis zum kleinsten, ist aus diesen Blöcken gebaut. Wer sie erkennt, kann jeden "
        "Synth bedienen. Hier wird der FM-1 zum Lern-Synth: die Regler K1, K2, K3 und K4 bewegen die Werte, die Tasten "
        "spielen, PLAY spielt ein Beispiel und FX setzt den Klang auf den Anfang des Kapitels zurück.",
        "Tous les synthés, du plus grand au plus petit, sont faits de ces blocs. Apprends à les reconnaître et tu sauras "
        "utiliser n'importe lequel. Ici le FM-1 devient un synthé pour apprendre : les boutons K1, K2, K3 et K4 changent "
        "les réglages, les touches jouent, PLAY fait entendre un exemple et FX ramène le son au début du chapitre.")),
    P("BLOCKS", ["WAVE", "CUTOFF", "VOL"], (
        "K1 muove l'oscillatore (l'onda), K2 il filtro (il cutoff), K3 l'amplificatore (il volume). Suona e ascolta "
        "ogni blocco.",
        "K1 moves the oscillator (the wave), K2 the filter (the cutoff), K3 the amplifier (the volume). Play and listen "
        "to each block.",
        "K1 mueve el oscilador (la onda), K2 el filtro (el cutoff), K3 el amplificador (el volumen). Toca y escucha "
        "cada bloque.",
        "K1 bewegt den Oszillator (die Welle), K2 den Filter (den Cutoff), K3 den Verstärker (die Lautstärke). Spiel "
        "und hör jeden Block.",
        "K1 règle l'oscillateur (l'onde), K2 le filtre (le cutoff), K3 l'amplificateur (le volume). Joue et écoute "
        "chaque bloc.")),
])

# 3 --- the oscillator
chapter(0, ("L'oscillatore", "The oscillator", "El oscilador", "Der Oszillator", "L'oscillateur"), "note", {}, [
    T("WAVE", (
        "Sinusoide: solo la fondamentale, nessuna armonica. È il suono più puro e morbido, come un fischio. Suona un "
        "tasto per sentirla.",
        "Sine: only the fundamental, no harmonics. The purest, softest sound, like a whistle. Play a key to hear it.",
        "Senoidal: solo la fundamental, ningún armónico. Es el sonido más puro y suave, como un silbido. Toca una tecla "
        "para oírla.",
        "Sinus: nur der Grundton, keine Obertöne. Der reinste, weichste Klang, wie ein Pfeifen. Spiel eine Taste, um "
        "ihn zu hören.",
        "Sinusoïde : seulement la fondamentale, aucune harmonique. C'est le son le plus pur et le plus doux, comme un "
        "sifflement. Joue une touche pour l'entendre."), set=dict(WAVE=SINE)),
    T("WAVE", (
        "Triangolo: solo le armoniche dispari (3, 5, 7...), molto deboli. Suono dolce, simile a un flauto o a "
        "un'ocarina.",
        "Triangle: only the odd harmonics (3, 5, 7...), very soft. A gentle sound, like a flute or an ocarina.",
        "Triangular: solo los armónicos impares (3, 5, 7...), muy débiles. Sonido dulce, como una flauta o una ocarina.",
        "Dreieck: nur die ungeraden Obertöne (3, 5, 7...), sehr leise. Ein sanfter Klang, wie eine Flöte oder eine "
        "Okarina.",
        "Triangle : seulement les harmoniques impaires (3, 5, 7...), très faibles. Un son doux, comme une flûte ou un "
        "ocarina."), set=dict(WAVE=TRI)),
    T("WAVE", (
        "Quadra: armoniche dispari, più forti. Suono cavo e legnoso, che ricorda il clarinetto.",
        "Square: odd harmonics, louder. A hollow, woody sound, like a clarinet.",
        "Cuadrada: armónicos impares, más fuertes. Sonido hueco y de madera, que recuerda al clarinete.",
        "Rechteck: ungerade Obertöne, lauter. Ein hohler, holziger Klang, wie eine Klarinette.",
        "Carrée : harmoniques impaires, plus fortes. Un son creux et boisé, qui rappelle la clarinette."),
      set=dict(WAVE=SQUARE)),
    T("WAVE", (
        "Dente di sega: tutte le armoniche, ognuna un po' più debole della precedente. È l'onda più ricca e brillante: "
        "archi, ottoni e bassi nascono da qui.",
        "Saw: every harmonic, each a little softer than the one before. The richest, brightest wave: strings, brass and "
        "basses start here.",
        "Diente de sierra: todos los armónicos, cada uno un poco más débil que el anterior. Es la onda más rica y "
        "brillante: cuerdas, metales y bajos nacen de aquí.",
        "Sägezahn: alle Obertöne, jeder etwas leiser als der vorige. Die reichste, hellste Welle: Streicher, Bläser und "
        "Bässe beginnen hier.",
        "Dents de scie : toutes les harmoniques, chacune un peu plus faible que la précédente. L'onde la plus riche et "
        "brillante : cordes, cuivres et basses naissent ici."), set=dict(WAVE=SAW)),
    T("WAVE", (
        "Impulso: una quadra asimmetrica. La larghezza (PW) cambia le armoniche: più l'impulso è stretto, più il suono "
        "è sottile e nasale.",
        "Pulse: a lopsided square. Its width (PW) changes the harmonics: the narrower the pulse, the thinner and more "
        "nasal the sound.",
        "Pulso: una cuadrada asimétrica. Su anchura (PW) cambia los armónicos: cuanto más estrecho el pulso, más fino y "
        "nasal el sonido.",
        "Puls: ein schiefes Rechteck. Seine Breite (PW) ändert die Obertöne: je schmaler der Puls, desto dünner und "
        "näselnder der Klang.",
        "Impulsion : un carré déséquilibré. Sa largeur (PW) change les harmoniques : plus l'impulsion est étroite, plus "
        "le son est fin et nasal."), set=dict(WAVE=PULSE, PW=25)),
    T("WAVE", (
        "Rumore: tutte le frequenze insieme, senza una nota precisa. Serve per vento, soffi e percussioni.",
        "Noise: every frequency at once, with no clear note. Good for wind, breath and drums.",
        "Ruido: todas las frecuencias a la vez, sin una nota clara. Sirve para viento, soplos y percusión.",
        "Rauschen: alle Frequenzen zugleich, ohne klaren Ton. Gut für Wind, Hauch und Schlagzeug.",
        "Bruit : toutes les fréquences à la fois, sans note précise. Il sert pour le vent, le souffle et les "
        "percussions."), set=dict(WAVE=NOISE)),
    P("WAVE", ["WAVE", "PW", "DETUNE", "OSC2"], (
        "K1 l'onda, K2 la larghezza dell'impulso, K3 il detune del secondo oscillatore, K4 il suo volume. Prova tutto!",
        "K1 the wave, K2 the pulse width, K3 the second oscillator's detune, K4 its level. Try everything!",
        "K1 la onda, K2 la anchura del pulso, K3 el detune del segundo oscilador, K4 su nivel. Pruébalo todo.",
        "K1 die Welle, K2 die Pulsbreite, K3 die Verstimmung des zweiten Oszillators, K4 seine Lautstärke. Probier "
        "alles aus!",
        "K1 l'onde, K2 la largeur d'impulsion, K3 le désaccord du second oscillateur, K4 son niveau. Essaie tout !"),
      set=dict(WAVE=SAW, PW=50)),
    S("WAVE", ["WAVE", "PW", "DETUNE", "OSC2"], ("PW", "le", 12, 0), (
        "Onda impulso. Tieni premuto un tasto e porta K2 (PW) dal 50% verso il 10%: il suono si assottiglia e diventa "
        "nasale.",
        "Pulse wave. Hold a key and take K2 (PW) from 50% down towards 10%: the sound thins out and turns nasal.",
        "Onda de pulso. Mantén una tecla y lleva K2 (PW) del 50% hacia el 10%: el sonido se afina y se vuelve nasal.",
        "Pulswelle. Halte eine Taste und dreh K2 (PW) von 50% Richtung 10%: der Klang wird dünn und näselnd.",
        "Onde impulsion. Garde une touche enfoncée et descends K2 (PW) de 50% vers 10% : le son s'amincit et devient "
        "nasal."), set=dict(WAVE=PULSE, PW=50, OSC2=0)),
    S("WAVE", ["WAVE", "PW", "DETUNE", "OSC2"], ("DETUNE", "ge", 12, 0), (
        "Due denti di sega insieme. Alza K3 (detune) fino a circa 15 centesimi: le onde si sfasano e il suono si "
        "allarga.",
        "Two saws together. Turn K3 (detune) up to about 15 cents: the waves drift apart and the sound gets wider.",
        "Dos dientes de sierra juntos. Sube K3 (detune) hasta unos 15 cents: las ondas se desfasan y el sonido se "
        "ensancha.",
        "Zwei Sägezähne zusammen. Dreh K3 (Detune) auf etwa 15 Cent: die Wellen laufen auseinander und der Klang wird "
        "breiter.",
        "Deux dents de scie ensemble. Monte K3 (detune) vers 15 cents : les ondes se décalent et le son s'élargit."),
      set=dict(WAVE=SAW, WAVE2=SAW, OSC2=100, DETUNE=0)),
])

# 4 --- the filter
chapter(0, ("Il filtro", "The filter", "El filtro", "Der Filter", "Le filtre"), "note", {}, [
    T("FILTER", (
        "Il filtro toglie una parte delle armoniche. Il passa-basso (LP) lascia passare i suoni bassi e taglia quelli "
        "acuti: il suono diventa più scuro e morbido. È il filtro più usato.",
        "The filter takes some harmonics away. The low-pass (LP) lets the low sounds through and cuts the high ones: "
        "the sound gets darker and softer. It's the most used filter.",
        "El filtro quita parte de los armónicos. El paso bajo (LP) deja pasar los sonidos graves y corta los agudos: el "
        "sonido se vuelve más oscuro y suave. Es el filtro más usado.",
        "Der Filter nimmt Obertöne weg. Der Tiefpass (LP) lässt tiefe Klänge durch und schneidet hohe ab: der Klang "
        "wird dunkler und weicher. Er ist der meistbenutzte Filter.",
        "Le filtre enlève une partie des harmoniques. Le passe-bas (LP) laisse passer les sons graves et coupe les "
        "aigus : le son devient plus sombre et plus doux. C'est le filtre le plus utilisé."),
      set=dict(FTYPE=LP, CUTOFF=50, RESO=0)),
    T("FILTER", (
        "Il cutoff è il punto in cui il filtro comincia a tagliare. La risonanza rinforza le frequenze vicine al "
        "cutoff: compare un picco, un fischio che segue la manopola.",
        "The cutoff is the point where the filter starts to cut. Resonance boosts the frequencies near the cutoff: a "
        "peak appears, a whistle that follows the knob.",
        "El cutoff es el punto donde el filtro empieza a cortar. La resonancia refuerza las frecuencias cerca del "
        "cutoff: aparece un pico, un silbido que sigue a la perilla.",
        "Der Cutoff ist der Punkt, an dem der Filter zu schneiden beginnt. Die Resonanz verstärkt die Frequenzen nahe "
        "am Cutoff: eine Spitze entsteht, ein Pfeifen, das dem Regler folgt.",
        "Le cutoff est le point où le filtre commence à couper. La résonance renforce les fréquences proches du "
        "cutoff : un pic apparaît, un sifflement qui suit le bouton."), set=dict(FTYPE=LP, CUTOFF=50, RESO=60)),
    T("FILTER", (
        "Passa-alto (HP): il contrario, toglie le basse e il suono diventa sottile. Passa-banda (BP): lascia passare "
        "solo una fascia, con un suono nasale, come al telefono.",
        "High-pass (HP): the opposite, it removes the lows and the sound gets thin. Band-pass (BP): only a band gets "
        "through, with a nasal sound, like on the phone.",
        "Paso alto (HP): lo contrario, quita los graves y el sonido se vuelve fino. Paso banda (BP): solo deja pasar "
        "una franja, con un sonido nasal, como al teléfono.",
        "Hochpass (HP): das Gegenteil, er nimmt die Tiefen weg und der Klang wird dünn. Bandpass (BP): nur ein Band "
        "kommt durch, näselnd wie am Telefon.",
        "Passe-haut (HP) : l'inverse, il enlève les graves et le son devient fin. Passe-bande (BP) : seule une bande "
        "passe, avec un son nasal, comme au téléphone."), set=dict(FTYPE=HP, CUTOFF=55, RESO=20)),
    P("FILTER", ["CUTOFF", "RESO", "FTYPE", "WAVE"], (
        "K1 il cutoff, K2 la risonanza, K3 il tipo di filtro, K4 l'onda. Tieni una nota e muovi K1.",
        "K1 the cutoff, K2 the resonance, K3 the filter type, K4 the wave. Hold a note and move K1.",
        "K1 el cutoff, K2 la resonancia, K3 el tipo de filtro, K4 la onda. Mantén una nota y mueve K1.",
        "K1 der Cutoff, K2 die Resonanz, K3 der Filtertyp, K4 die Welle. Halte einen Ton und bewege K1.",
        "K1 le cutoff, K2 la résonance, K3 le type de filtre, K4 l'onde. Tiens une note et tourne K1."),
      set=dict(FTYPE=LP, CUTOFF=60, RESO=20)),
    S("FILTER", ["CUTOFF", "RESO", "FTYPE", "WAVE"], ("CUTOFF", "eq", 25, 4), (
        "Tieni premuto un tasto e gira K1 fino al 25%: il dente di sega diventa ovattato, come dietro una porta.",
        "Hold a key down and turn K1 to 25%: the saw gets muffled, like behind a door.",
        "Mantén una tecla pulsada y gira K1 hasta el 25%: el diente de sierra suena apagado, como detrás de una puerta.",
        "Halte eine Taste gedrückt und dreh K1 auf 25%: der Sägezahn klingt dumpf, wie hinter einer Tür.",
        "Garde une touche enfoncée et tourne K1 jusqu'à 25% : la dent de scie devient étouffée, comme derrière une "
        "porte."), set=dict(FTYPE=LP, CUTOFF=100, RESO=0, WAVE=SAW)),
    S("FILTER", ["CUTOFF", "RESO", "FTYPE", "WAVE"], ("RESO", "ge", 70, 0), (
        "Alza la risonanza (K2) al 70%, poi muovi il cutoff (K1): senti il fischio che segue la manopola?",
        "Turn resonance (K2) up to 70%, then move the cutoff (K1): can you hear the whistle following the knob?",
        "Sube la resonancia (K2) al 70% y luego mueve el cutoff (K1): se oye un silbido que sigue a la perilla.",
        "Dreh die Resonanz (K2) auf 70% und bewege dann den Cutoff (K1): hörst du das Pfeifen, das dem Regler folgt?",
        "Monte la résonance (K2) à 70%, puis tourne le cutoff (K1) : tu entends le sifflement qui suit le bouton ?"),
      set=dict(CUTOFF=45)),
])

# 5 --- the ADSR envelope
chapter(1, ("L'inviluppo ADSR", "The ADSR envelope", "La envolvente ADSR", "Die ADSR-Hüllkurve", "L'enveloppe ADSR"),
        "scale", dict(CUTOFF=70), [
    T("ADSR", (
        "L'inviluppo è una curva che parte quando premi un tasto. Sul volume decide come il suono nasce, vive e muore. "
        "Ha quattro fasi: Attack, Decay, Sustain e Release (ADSR).",
        "An envelope is a curve that starts when you press a key. On the volume, it decides how the sound is born, "
        "lives and dies. It has four stages: Attack, Decay, Sustain and Release (ADSR).",
        "La envolvente es una curva que empieza al pulsar una tecla. En el volumen decide cómo el sonido nace, vive y "
        "muere. Tiene cuatro fases: Attack, Decay, Sustain y Release (ADSR).",
        "Eine Hüllkurve ist eine Kurve, die beim Tastendruck startet. Auf der Lautstärke bestimmt sie, wie der Klang "
        "entsteht, lebt und vergeht. Sie hat vier Phasen: Attack, Decay, Sustain und Release (ADSR).",
        "L'enveloppe est une courbe qui démarre quand tu appuies sur une touche. Sur le volume, elle décide comment le "
        "son naît, vit et meurt. Elle a quatre phases : Attack, Decay, Sustain et Release (ADSR)."),
      set=dict(A=t(100), D=t(300), S=60, R=t(500))),
    T("ADSR", (
        "Attack: il tempo per arrivare al massimo. Decay: il tempo per scendere al sustain. Sustain: il volume finché "
        "tieni il tasto (è un livello, non un tempo). Release: il tempo per spegnersi dopo.",
        "Attack: the time to reach the top. Decay: the time to fall to the sustain. Sustain: the volume while you hold "
        "the key (a level, not a time). Release: the time to fade out after.",
        "Attack: el tiempo para llegar al máximo. Decay: el tiempo para bajar al sustain. Sustain: el volumen mientras "
        "mantienes la tecla (un nivel, no un tiempo). Release: el tiempo para apagarse después.",
        "Attack: die Zeit bis zum Maximum. Decay: die Zeit, um auf das Sustain zu fallen. Sustain: die Lautstärke, "
        "solange die Taste gehalten wird (ein Pegel, keine Zeit). Release: die Zeit zum Ausklingen danach.",
        "Attack : le temps pour monter au maximum. Decay : le temps pour descendre au sustain. Sustain : le volume tant "
        "que tu tiens la touche (un niveau, pas un temps). Release : le temps pour s'éteindre après.")),
    T("ADSR4", (
        "Quattro forme da ricordare. Organo: si accende e si spegne di colpo. Pianoforte: parte forte e cala. Archi: "
        "entrano piano e sfumano. Percussione: un colpo breve.",
        "Four shapes to remember. Organ: on and off at once. Piano: starts loud and fades. Strings: come in slowly and "
        "fade away. Drum: a short hit.",
        "Cuatro formas para recordar. Órgano: se enciende y se apaga de golpe. Piano: empieza fuerte y baja. Cuerdas: "
        "entran despacio y se desvanecen. Percusión: un golpe corto.",
        "Vier Formen zum Merken. Orgel: sofort an und aus. Klavier: beginnt laut und klingt ab. Streicher: setzen "
        "langsam ein und verklingen. Schlagzeug: ein kurzer Schlag.",
        "Quatre formes à retenir. Orgue : s'allume et s'éteint d'un coup. Piano : commence fort et diminue. Cordes : "
        "entrent doucement et s'estompent. Percussion : un coup bref.")),
    P("ADSR", ["A", "D", "S", "R"], (
        "K1 Attack, K2 Decay, K3 Sustain, K4 Release. Il grafico mostra la curva: suona e guardala.",
        "K1 Attack, K2 Decay, K3 Sustain, K4 Release. The graph shows the curve: play and watch it.",
        "K1 Attack, K2 Decay, K3 Sustain, K4 Release. El gráfico muestra la curva: toca y mírala.",
        "K1 Attack, K2 Decay, K3 Sustain, K4 Release. Die Grafik zeigt die Kurve: spiel und schau zu.",
        "K1 Attack, K2 Decay, K3 Sustain, K4 Release. Le graphique montre la courbe : joue et regarde-la.")),
    S("ADSR", ["A", "D", "S", "R"], ("S", "ge", 95, 0), (
        "Organo: porta il sustain (K3) al 100%. Il suono resta uguale finché tieni il tasto e si spegne quando lo "
        "lasci.",
        "Organ: turn sustain (K3) to 100%. The sound stays the same while you hold the key and stops when you let go.",
        "Órgano: lleva el sustain (K3) al 100%. El sonido sigue igual mientras mantienes la tecla y se apaga al "
        "soltarla.",
        "Orgel: dreh das Sustain (K3) auf 100%. Der Klang bleibt gleich, solange du die Taste hältst, und stoppt beim "
        "Loslassen.",
        "Orgue : monte le sustain (K3) à 100%. Le son reste pareil tant que tu tiens la touche et s'arrête quand tu la "
        "lâches."), set=dict(A=0, D=t(100), S=30, R=t(10))),
    S("ADSR", ["A", "D", "S", "R"], ("S", "le", 4, 0), (
        "Pianoforte: abbassa il sustain (K3) a zero. Ora il suono cala da solo, anche se tieni premuto il tasto.",
        "Piano: take sustain (K3) down to zero. Now the sound fades by itself, even while you hold the key.",
        "Piano: baja el sustain (K3) a cero. Ahora el sonido baja solo, aunque mantengas la tecla.",
        "Klavier: dreh das Sustain (K3) auf null. Jetzt klingt der Ton von selbst ab, auch wenn du die Taste hältst.",
        "Piano : baisse le sustain (K3) à zéro. Le son diminue maintenant tout seul, même si tu tiens la touche."),
      set=dict(A=0, D=t(1500), S=100, R=t(300))),
    S("ADSR", ["A", "D", "S", "R"], ("A", "eq", t(500), 5), (
        "Archi: alza l'attack (K1) fino a circa mezzo secondo. Il suono entra piano piano, come un arco sulla corda.",
        "Strings: turn attack (K1) up to about half a second. The sound fades in slowly, like a bow on a string.",
        "Cuerdas: sube el attack (K1) hasta medio segundo más o menos. El sonido entra poco a poco, como un arco en la "
        "cuerda.",
        "Streicher: dreh das Attack (K1) auf etwa eine halbe Sekunde. Der Klang setzt langsam ein, wie ein Bogen auf "
        "der Saite.",
        "Cordes : monte l'attack (K1) vers une demi-seconde. Le son entre tout doucement, comme un archet sur la "
        "corde."), set=dict(A=0, D=t(300), S=80, R=t(500))),
    S("ADSR", ["A", "D", "S", "R"], ("D", "eq", t(200), 4), (
        "Percussione: attack 0 e sustain 0. Accorcia il decay (K2) a circa 200 ms: un colpo breve, come un tamburo.",
        "Drum: attack 0 and sustain 0. Shorten the decay (K2) to about 200 ms: a short hit, like a drum.",
        "Percusión: attack 0 y sustain 0. Acorta el decay (K2) a unos 200 ms: un golpe corto, como un tambor.",
        "Schlagzeug: Attack 0 und Sustain 0. Verkürze das Decay (K2) auf etwa 200 ms: ein kurzer Schlag, wie eine "
        "Trommel.",
        "Percussion : attack 0 et sustain 0. Raccourcis le decay (K2) vers 200 ms : un coup bref, comme un tambour."),
      set=dict(A=0, D=t(1000), S=0, R=t(200))),
])

# 6 --- the filter envelope
chapter(1, ("L'inviluppo del filtro", "The filter envelope", "La envolvente del filtro", "Die Filterhüllkurve",
            "L'enveloppe du filtre"), "pluck", dict(CUTOFF=20, RESO=30, FD=t(250), FR=t(250), R=t(250)), [
    T("FENV", (
        "Negli strumenti veri il colore cambia nel tempo: più brillante all'inizio, più scuro nella coda. Un secondo "
        "inviluppo che muove il cutoff fa proprio questo.",
        "In real instruments the colour changes over time: brighter at the start, darker as it fades. A second "
        "envelope moving the cutoff does exactly that.",
        "En los instrumentos reales el color cambia con el tiempo: más brillante al principio, más oscuro al final. Una "
        "segunda envolvente que mueve el cutoff hace justo eso.",
        "Bei echten Instrumenten ändert sich die Farbe mit der Zeit: am Anfang heller, am Ende dunkler. Eine zweite "
        "Hüllkurve, die den Cutoff bewegt, macht genau das.",
        "Dans les vrais instruments, la couleur change avec le temps : plus brillante au début, plus sombre à la fin. "
        "Une seconde enveloppe qui fait bouger le cutoff fait exactement ça."), set=dict(FAMT=60)),
    T("FENV", (
        "L'amount dice quanto l'inviluppo apre il filtro. La regola d'oro: cutoff basso, amount alto. Se il filtro è "
        "già tutto aperto, l'inviluppo non ha spazio e sembra non funzionare.",
        "The amount says how far the envelope opens the filter. The golden rule: low cutoff, high amount. If the filter "
        "is already wide open, the envelope has no room and seems not to work.",
        "El amount dice cuánto abre el filtro la envolvente. La regla de oro: cutoff bajo, amount alto. Si el filtro ya "
        "está abierto del todo, la envolvente no tiene espacio y parece no funcionar.",
        "Das Amount sagt, wie weit die Hüllkurve den Filter öffnet. Die goldene Regel: Cutoff tief, Amount hoch. Ist "
        "der Filter schon ganz offen, hat die Hüllkurve keinen Platz und scheint nicht zu wirken.",
        "L'amount dit de combien l'enveloppe ouvre le filtre. La règle d'or : cutoff bas, amount haut. Si le filtre est "
        "déjà grand ouvert, l'enveloppe n'a pas de place et semble ne pas marcher."), set=dict(FAMT=60)),
    P("FENV", ["CUTOFF", "FAMT", "FD", "RESO"], (
        "K1 il cutoff, K2 l'amount, K3 il decay del filtro, K4 la risonanza. Suona note brevi e ascolta il 'lampo'.",
        "K1 the cutoff, K2 the amount, K3 the filter's decay, K4 the resonance. Play short notes and listen for the "
        "'flash'.",
        "K1 el cutoff, K2 el amount, K3 el decay del filtro, K4 la resonancia. Toca notas cortas y escucha el "
        "'destello'.",
        "K1 der Cutoff, K2 das Amount, K3 das Filter-Decay, K4 die Resonanz. Spiel kurze Töne und hör auf den 'Blitz'.",
        "K1 le cutoff, K2 l'amount, K3 le decay du filtre, K4 la résonance. Joue des notes courtes et écoute "
        "l''éclair'."), set=dict(FAMT=50)),
    S("FENV", ["CUTOFF", "FAMT", "FD", "RESO"], ("FAMT", "eq", 60, 6), (
        "Alza l'amount (K2) al 60% e suona note staccate: a ogni nota il filtro si apre e si richiude. È un pluck, un "
        "suono pizzicato.",
        "Turn the amount (K2) up to 60% and play short notes: on each note the filter opens and closes. That's a "
        "pluck.",
        "Sube el amount (K2) al 60% y toca notas cortas: en cada nota el filtro se abre y se cierra. Es un pluck, un "
        "sonido pellizcado.",
        "Dreh das Amount (K2) auf 60% und spiel kurze Töne: bei jedem Ton öffnet und schließt sich der Filter. Das ist "
        "ein Pluck, ein gezupfter Klang.",
        "Monte l'amount (K2) à 60% et joue des notes courtes : à chaque note le filtre s'ouvre et se referme. C'est un "
        "pluck, un son pincé."), set=dict(FAMT=0, CUTOFF=20, FD=t(250))),
    S("FENV", ["CUTOFF", "FAMT", "FD", "RESO"], ("FD", "eq", t(1000), 4), (
        "Allunga il decay del filtro (K3) a circa 1 secondo: il suono si apre e si chiude lentamente, come un 'uaaa'.",
        "Lengthen the filter decay (K3) to about 1 second: the sound opens and closes slowly, like a 'waaah'.",
        "Alarga el decay del filtro (K3) a cerca de 1 segundo: el sonido se abre y se cierra despacio, como un 'uaaa'.",
        "Verlängere das Filter-Decay (K3) auf etwa 1 Sekunde: der Klang öffnet und schließt sich langsam, wie ein "
        "'waaah'.",
        "Allonge le decay du filtre (K3) vers 1 seconde : le son s'ouvre et se ferme lentement, comme un 'ouaaah'."),
      set=dict(FAMT=60, CUTOFF=20)),
    S("FENV", ["CUTOFF", "FAMT", "FD", "RESO"], ("CUTOFF", "ge", 88, 0), (
        "Ora apri del tutto il cutoff (K1) al 90%: il lampo sparisce. Ecco perché serve un cutoff basso.",
        "Now open the cutoff (K1) all the way to 90%: the flash is gone. That's why you need a low cutoff.",
        "Ahora abre del todo el cutoff (K1) al 90%: el destello desaparece. Por eso hace falta un cutoff bajo.",
        "Öffne jetzt den Cutoff (K1) ganz auf 90%: der Blitz ist weg. Darum braucht man einen tiefen Cutoff.",
        "Ouvre maintenant le cutoff (K1) à fond, à 90% : l'éclair disparaît. Voilà pourquoi il faut un cutoff bas."),
      set=dict(FAMT=60, FD=t(250))),
])

# 7 --- the pitch envelope
chapter(1, ("L'inviluppo di pitch", "The pitch envelope", "La envolvente de altura", "Die Tonhöhen-Hüllkurve",
            "L'enveloppe de hauteur"), "kick", dict(WAVE=SINE, A=0, D=t(400), S=0, R=t(300), PAMT=24, PDEC=t(60)), [
    T("PENV", (
        "Un inviluppo può muovere anche l'altezza. Se la nota parte alta e scende in pochi millesimi di secondo, senti "
        "un colpo: è così che nasce la cassa della batteria elettronica.",
        "An envelope can move the pitch too. If the note starts high and drops in a few thousandths of a second, you "
        "hear a thump: that's how an electronic kick drum is made.",
        "Una envolvente también puede mover la altura. Si la nota empieza aguda y baja en pocas milésimas de segundo, "
        "oyes un golpe: así nace el bombo de la batería electrónica.",
        "Eine Hüllkurve kann auch die Tonhöhe bewegen. Startet der Ton hoch und fällt in wenigen Tausendstelsekunden, "
        "hörst du einen Schlag: so entsteht die Bassdrum der elektronischen Drums.",
        "Une enveloppe peut aussi faire bouger la hauteur. Si la note part haut et descend en quelques millièmes de "
        "seconde, tu entends un coup : c'est ainsi que naît la grosse caisse électronique."), set=dict(PDEC=t(40))),
    P("PENV", ["PAMT", "PDEC", "D", "WAVE"], (
        "K1 quanto sale l'altezza (semitoni), K2 quanto in fretta scende, K3 il decay del volume, K4 l'onda. Suona "
        "note gravi.",
        "K1 how far the pitch jumps (semitones), K2 how fast it falls, K3 the volume decay, K4 the wave. Play low "
        "notes.",
        "K1 cuánto sube la altura (semitonos), K2 lo rápido que baja, K3 el decay del volumen, K4 la onda. Toca notas "
        "graves.",
        "K1 wie weit die Tonhöhe springt (Halbtöne), K2 wie schnell sie fällt, K3 das Lautstärke-Decay, K4 die Welle. "
        "Spiel tiefe Töne.",
        "K1 de combien la hauteur saute (demi-tons), K2 à quelle vitesse elle retombe, K3 le decay du volume, K4 "
        "l'onde. Joue des notes graves.")),
    S("PENV", ["PAMT", "PDEC", "D", "WAVE"], ("PDEC", "eq", t(40), 4), (
        "Suona una nota grave. Porta K2 a circa 40 ms: un colpo secco. Hai costruito una cassa!",
        "Play a low note. Set K2 to about 40 ms: a tight thump. You've built a kick drum!",
        "Toca una nota grave. Lleva K2 a unos 40 ms: un golpe seco. Has construido un bombo.",
        "Spiel einen tiefen Ton. Stell K2 auf etwa 40 ms: ein trockener Schlag. Du hast eine Bassdrum gebaut!",
        "Joue une note grave. Règle K2 vers 40 ms : un coup sec. Tu as construit une grosse caisse !"),
      set=dict(PDEC=t(250), PAMT=24)),
    S("PENV", ["PAMT", "PDEC", "D", "WAVE"], ("PDEC", "ge", t(120), 0), (
        "Ora allunga K2 oltre 120 ms: il colpo diventa un 'piuu' da laser, come nei videogiochi.",
        "Now take K2 past 120 ms: the thump turns into a laser 'pew', like in video games.",
        "Ahora alarga K2 por encima de 120 ms: el golpe se vuelve un 'piuu' de láser, como en los videojuegos.",
        "Dreh K2 jetzt über 120 ms: der Schlag wird zu einem Laser-'piuu', wie in Videospielen.",
        "Allonge maintenant K2 au-delà de 120 ms : le coup devient un 'piou' de laser, comme dans les jeux vidéo."),
      set=dict(PDEC=t(40))),
])

# 8 --- the LFO
chapter(1, ("L'LFO", "The LFO", "El LFO", "Der LFO", "Le LFO"), "hold", dict(CUTOFF=55, RESO=20), [
    T("LFO", (
        "L'LFO è un oscillatore lentissimo, da 0,1 a 20 vibrazioni al secondo: non lo senti, ma muove un parametro. "
        "Sull'altezza fa il vibrato, sul volume il tremolo, sul filtro un 'wah'.",
        "The LFO is a very slow oscillator, from 0.1 to 20 wobbles per second: you can't hear it, but it moves a "
        "setting. On pitch it makes vibrato, on volume tremolo, on the filter a 'wah'.",
        "El LFO es un oscilador lentísimo, de 0,1 a 20 vibraciones por segundo: no lo oyes, pero mueve un parámetro. En "
        "la altura hace vibrato, en el volumen trémolo, en el filtro un 'wah'.",
        "Der LFO ist ein sehr langsamer Oszillator, 0,1 bis 20 Schwingungen pro Sekunde: man hört ihn nicht, aber er "
        "bewegt einen Wert. Auf der Tonhöhe macht er Vibrato, auf der Lautstärke Tremolo, auf dem Filter ein 'Wah'.",
        "Le LFO est un oscillateur très lent, de 0,1 à 20 vibrations par seconde : on ne l'entend pas, mais il fait "
        "bouger un réglage. Sur la hauteur il fait le vibrato, sur le volume le trémolo, sur le filtre un 'wah'."),
      set=dict(LDEST=L_PITCH, LDEPTH=40, LRATE=lfo(1))),
    T("LFO5", (
        "Le forme dell'LFO: sinusoide e triangolo per movimenti morbidi, dente di sega per rampe, quadra per salti "
        "(come una sirena), S&H per valori a caso, il suono da computer anni '70.",
        "LFO shapes: sine and triangle for smooth movement, saw for ramps, square for jumps (like a siren), S&H for "
        "random steps, the 1970s computer sound.",
        "Las formas del LFO: senoidal y triangular para movimientos suaves, sierra para rampas, cuadrada para saltos "
        "(como una sirena), S&H para valores al azar, el sonido de ordenador de los años 70.",
        "Die LFO-Formen: Sinus und Dreieck für weiche Bewegungen, Sägezahn für Rampen, Rechteck für Sprünge (wie eine "
        "Sirene), S&H für Zufallswerte, der Computer-Sound der 70er.",
        "Les formes du LFO : sinus et triangle pour des mouvements doux, dents de scie pour des rampes, carré pour des "
        "sauts (comme une sirène), S&H pour des valeurs au hasard, le son d'ordinateur des années 70.")),
    P("LFO", ["LRATE", "LDEPTH", "LDEST", "LSHAPE"], (
        "K1 la velocità, K2 la profondità, K3 la destinazione (cosa muove), K4 la forma. Tieni una nota e guarda il "
        "punto.",
        "K1 the rate, K2 the depth, K3 the target (what it moves), K4 the shape. Hold a note and watch the dot.",
        "K1 la velocidad, K2 la profundidad, K3 el destino (lo que mueve), K4 la forma. Mantén una nota y mira el "
        "punto.",
        "K1 das Tempo, K2 die Tiefe, K3 das Ziel (was er bewegt), K4 die Form. Halte einen Ton und beobachte den Punkt.",
        "K1 la vitesse, K2 la profondeur, K3 la cible (ce qu'il fait bouger), K4 la forme. Tiens une note et regarde le "
        "point."), set=dict(LDEST=L_PITCH, LDEPTH=20, LRATE=lfo(3))),
    S("LFO", ["LRATE", "LDEPTH", "LDEST", "LSHAPE"], ("LRATE", "eq", lfo(5.5), 3), (
        "Vibrato: l'LFO muove l'altezza. Porta la velocità (K1) a circa 5,5 Hz: la nota ondeggia come la voce di un "
        "cantante.",
        "Vibrato: the LFO moves the pitch. Set the rate (K1) to about 5.5 Hz: the note wavers like a singer's voice.",
        "Vibrato: el LFO mueve la altura. Lleva la velocidad (K1) a unos 5,5 Hz: la nota ondula como la voz de un "
        "cantante.",
        "Vibrato: der LFO bewegt die Tonhöhe. Stell das Tempo (K1) auf etwa 5,5 Hz: der Ton schwingt wie die Stimme "
        "eines Sängers.",
        "Vibrato : le LFO fait bouger la hauteur. Règle la vitesse (K1) vers 5,5 Hz : la note ondule comme la voix d'un "
        "chanteur."), set=dict(LDEST=L_PITCH, LDEPTH=15, LRATE=lfo(1), LSHAPE=0)),
    S("LFO", ["LRATE", "LDEPTH", "LDEST", "LSHAPE"], ("LDEST", "eq", L_CUTOFF, 0), (
        "Ora sposta la destinazione (K3) sul filtro, con un LFO lento: il suono 'respira', si apre e si chiude da solo.",
        "Now switch the target (K3) to the filter, with a slow LFO: the sound 'breathes', opening and closing by "
        "itself.",
        "Ahora cambia el destino (K3) al filtro, con un LFO lento: el sonido 'respira', se abre y se cierra solo.",
        "Stell jetzt das Ziel (K3) auf den Filter, mit einem langsamen LFO: der Klang 'atmet', öffnet und schließt sich "
        "von selbst.",
        "Mets maintenant la cible (K3) sur le filtre, avec un LFO lent : le son 'respire', il s'ouvre et se ferme tout "
        "seul."), set=dict(LDEST=L_PITCH, LRATE=lfo(0.3), LDEPTH=40, LSHAPE=1)),
    S("LFO", ["LRATE", "LDEPTH", "LDEST", "LSHAPE"], ("LSHAPE", "eq", SH, 0), (
        "Scegli la forma S&H (K4): il filtro salta a caso, il gorgoglio dei computer dei film di fantascienza.",
        "Pick the S&H shape (K4): the filter jumps at random, the bubbling of sci-fi film computers.",
        "Elige la forma S&H (K4): el filtro salta al azar, el burbujeo de los ordenadores de las películas de ciencia "
        "ficción.",
        "Wähl die Form S&H (K4): der Filter springt zufällig, das Blubbern der Computer aus Science-Fiction-Filmen.",
        "Choisis la forme S&H (K4) : le filtre saute au hasard, le gargouillis des ordinateurs des films de "
        "science-fiction."), set=dict(RESO=60, LRATE=lfo(8), LDEPTH=50, LDEST=L_CUTOFF, LSHAPE=0)),
])

# 9 --- FM
chapter(2, ("La sintesi FM", "FM synthesis", "La síntesis FM", "FM-Synthese", "La synthèse FM"), "note",
        dict(ENGINE=E_FM, R=t(300)), [
    T("FM", (
        "Nella sintesi FM un oscillatore, il modulante, fa vibrare velocissimo l'altezza di un altro, la portante. Non "
        "senti un vibrato: nasce un timbro nuovo, brillante o metallico.",
        "In FM synthesis one oscillator, the modulator, wobbles the pitch of another, the carrier, very fast. You don't "
        "hear vibrato: a new timbre is born, bright or metallic.",
        "En la síntesis FM un oscilador, el modulador, hace vibrar muy rápido la altura de otro, la portadora. No oyes "
        "un vibrato: nace un timbre nuevo, brillante o metálico.",
        "Bei der FM-Synthese lässt ein Oszillator, der Modulator, die Tonhöhe eines anderen, des Trägers, sehr schnell "
        "schwingen. Man hört kein Vibrato: eine neue Klangfarbe entsteht, hell oder metallisch.",
        "En synthèse FM, un oscillateur, le modulateur, fait vibrer très vite la hauteur d'un autre, la porteuse. On "
        "n'entend pas de vibrato : un nouveau timbre naît, brillant ou métallique."), set=dict(INDEX=40)),
    T("FM", (
        "Ogni oscillatore FM si chiama operatore: una sinusoide con il suo inviluppo. La portante è quella che ascolti; "
        "il modulante non si sente da solo, cambia il timbro della portante.",
        "Each FM oscillator is called an operator: a sine wave with its own envelope. The carrier is the one you hear; "
        "the modulator isn't heard by itself, it changes the carrier's timbre.",
        "Cada oscilador FM se llama operador: una senoidal con su propia envolvente. La portadora es la que oyes; el "
        "modulador no se oye solo, cambia el timbre de la portadora.",
        "Jeder FM-Oszillator heißt Operator: eine Sinuswelle mit eigener Hüllkurve. Den Träger hörst du; den Modulator "
        "hört man nicht allein, er ändert die Klangfarbe des Trägers.",
        "Chaque oscillateur FM s'appelle un opérateur : une sinusoïde avec sa propre enveloppe. La porteuse est celle "
        "qu'on entend ; le modulateur ne s'entend pas seul, il change le timbre de la porteuse.")),
    T("FM", (
        "L'indice è quanta modulazione c'è. A zero senti una sinusoide pura; alzandolo compaiono sempre più armoniche, "
        "le bande laterali. Nella FM l'indice fa il lavoro del cutoff.",
        "The index is how much modulation there is. At zero you hear a pure sine; turn it up and more and more "
        "harmonics appear, the sidebands. In FM the index does the cutoff's job.",
        "El índice es cuánta modulación hay. A cero oyes una senoidal pura; al subirlo aparecen cada vez más "
        "armónicos, las bandas laterales. En FM el índice hace el trabajo del cutoff.",
        "Der Index ist die Menge der Modulation. Bei null hörst du einen reinen Sinus; drehst du ihn auf, erscheinen "
        "immer mehr Obertöne, die Seitenbänder. Bei FM macht der Index die Arbeit des Cutoffs.",
        "L'indice, c'est la quantité de modulation. À zéro on entend une sinusoïde pure ; en le montant, de plus en "
        "plus d'harmoniques apparaissent, les bandes latérales. En FM, l'indice fait le travail du cutoff."),
      set=dict(INDEX=60)),
    T("FM", (
        "Il ratio tra modulante e portante decide il tipo di suono. Numeri interi (1, 2, 3) danno suoni intonati; "
        "numeri non interi (1,41 o 3,5) suoni metallici, di campana.",
        "The ratio between modulator and carrier decides the kind of sound. Whole numbers (1, 2, 3) give pitched "
        "sounds; others (1.41 or 3.5) give metallic, bell-like sounds.",
        "El ratio entre modulador y portadora decide el tipo de sonido. Los números enteros (1, 2, 3) dan sonidos "
        "afinados; los no enteros (1,41 o 3,5), sonidos metálicos, de campana.",
        "Das Verhältnis (Ratio) von Modulator und Träger bestimmt die Art des Klangs. Ganze Zahlen (1, 2, 3) klingen "
        "tonal; andere (1,41 oder 3,5) metallisch, wie Glocken.",
        "Le ratio entre modulateur et porteuse décide du type de son. Les nombres entiers (1, 2, 3) donnent des sons "
        "accordés ; les autres (1,41 ou 3,5) des sons métalliques, de cloche."), set=dict(RATIO=5, INDEX=50)),
    T("FMALG", (
        "L'FM-1 è proprio un synth FM! I suoi algoritmi collegano 4 operatori: in pila (uno modula l'altro) o in "
        "parallelo (suonano insieme). Il feedback fa modulare un operatore da sé.",
        "The FM-1 really is an FM synth! Its algorithms connect 4 operators: stacked (one modulates the next) or in "
        "parallel (they play together). Feedback makes an operator modulate itself.",
        "El FM-1 es precisamente un sintetizador FM. Sus algoritmos conectan 4 operadores: en pila (uno modula al otro) "
        "o en paralelo (suenan juntos). El feedback hace que un operador se module a sí mismo.",
        "Der FM-1 ist tatsächlich ein FM-Synth! Seine Algorithmen verbinden 4 Operatoren: gestapelt (einer moduliert "
        "den nächsten) oder parallel (sie klingen zusammen). Feedback lässt einen Operator sich selbst modulieren.",
        "Le FM-1 est justement un synthé FM ! Ses algorithmes relient 4 opérateurs : en pile (l'un module le suivant) "
        "ou en parallèle (ils jouent ensemble). Le feedback fait qu'un opérateur se module lui-même.")),
    P("FM", ["RATIO", "INDEX", "MDEC", "FB"], (
        "K1 il ratio, K2 l'indice, K3 il decay del modulante, K4 il feedback. Guarda lo spettro cambiare.",
        "K1 the ratio, K2 the index, K3 the modulator's decay, K4 the feedback. Watch the spectrum change.",
        "K1 el ratio, K2 el índice, K3 el decay del modulador, K4 el feedback. Mira cómo cambia el espectro.",
        "K1 das Ratio, K2 der Index, K3 das Modulator-Decay, K4 das Feedback. Sieh, wie sich das Spektrum ändert.",
        "K1 le ratio, K2 l'indice, K3 le decay du modulateur, K4 le feedback. Regarde le spectre changer."),
      set=dict(RATIO=1, INDEX=30)),
    S("FM", ["RATIO", "INDEX", "MDEC", "FB"], ("INDEX", "eq", 60, 6), (
        "Ratio 1 e indice a zero: una sinusoide. Alza l'indice (K2) al 60%: il suono si arricchisce fino a somigliare "
        "a un dente di sega.",
        "Ratio 1 and index at zero: a sine. Turn the index (K2) up to 60%: the sound grows richer until it sounds like "
        "a saw.",
        "Ratio 1 e índice a cero: una senoidal. Sube el índice (K2) al 60%: el sonido se enriquece hasta parecerse a un "
        "diente de sierra.",
        "Ratio 1 und Index null: ein Sinus. Dreh den Index (K2) auf 60%: der Klang wird reicher, bis er wie ein "
        "Sägezahn klingt.",
        "Ratio 1 et indice à zéro : une sinusoïde. Monte l'indice (K2) à 60% : le son s'enrichit jusqu'à ressembler à "
        "une dent de scie."), set=dict(RATIO=1, INDEX=0, MDEC=100, FB=0, D=t(300), S=100)),
    S("FM", ["RATIO", "INDEX", "MDEC", "FB"], ("RATIO", "eq", 5, 0), (
        "Campana: porta il ratio (K1) a 3,5. Con il modulante che si spegne in fretta, ogni nota è un colpo di "
        "campana.",
        "Bell: set the ratio (K1) to 3.5. With the modulator fading fast, every note is a bell strike.",
        "Campana: lleva el ratio (K1) a 3,5. Con el modulador apagándose rápido, cada nota es un golpe de campana.",
        "Glocke: stell das Ratio (K1) auf 3,5. Mit einem schnell abklingenden Modulator wird jeder Ton ein "
        "Glockenschlag.",
        "Cloche : règle le ratio (K1) sur 3,5. Avec un modulateur qui s'éteint vite, chaque note devient un coup de "
        "cloche."), set=dict(RATIO=1, INDEX=60, MDEC=t(700), S=0, D=t(2500), R=t(1500))),
    S("FM", ["RATIO", "INDEX", "MDEC", "FB"], ("MDEC", "eq", t(300), 5), (
        "Piano elettrico: ratio 1, indice alto. Accorcia il decay del modulante (K3) a circa 300 ms: attacco "
        "brillante che si addolcisce, il suono degli anni '80.",
        "Electric piano: ratio 1, high index. Shorten the modulator decay (K3) to about 300 ms: a bright attack that "
        "softens, the sound of the 1980s.",
        "Piano eléctrico: ratio 1, índice alto. Acorta el decay del modulador (K3) a unos 300 ms: ataque brillante que "
        "se suaviza, el sonido de los años 80.",
        "E-Piano: Ratio 1, hoher Index. Verkürze das Modulator-Decay (K3) auf etwa 300 ms: ein heller Anschlag, der "
        "weicher wird, der Klang der 80er.",
        "Piano électrique : ratio 1, indice haut. Raccourcis le decay du modulateur (K3) vers 300 ms : une attaque "
        "brillante qui s'adoucit, le son des années 80."), set=dict(RATIO=1, INDEX=70, MDEC=100, D=t(1500), S=0)),
])

# 10 --- additive and wavetable
chapter(2, ("Additiva e wavetable", "Additive and wavetable", "Aditiva y wavetable", "Additiv und Wavetable",
            "Additive et wavetable"), "chord", dict(ENGINE=E_ADD), [
    T("ADD", (
        "Sintesi additiva: ogni suono è una somma di sinusoidi, le armoniche. Sommandone tante si può costruire "
        "qualsiasi timbro. L'organo è un synth additivo: ogni registro aggiunge una sinusoide.",
        "Additive synthesis: every sound is a sum of sine waves, the harmonics. Adding many of them can build any "
        "timbre. The organ is an additive synth: each stop adds a sine wave.",
        "Síntesis aditiva: cada sonido es una suma de senoidales, los armónicos. Sumando muchas se puede construir "
        "cualquier timbre. El órgano es un sintetizador aditivo: cada registro añade una senoidal.",
        "Additive Synthese: jeder Klang ist eine Summe von Sinuswellen, den Obertönen. Mit vielen davon lässt sich jede "
        "Klangfarbe bauen. Die Orgel ist ein additiver Synth: jedes Register fügt eine Sinuswelle hinzu.",
        "Synthèse additive : chaque son est une somme de sinusoïdes, les harmoniques. En en additionnant beaucoup, on "
        "peut construire n'importe quel timbre. L'orgue est un synthé additif : chaque registre ajoute une sinusoïde."),
      set=dict(H1=100, H2=50, H3=33, H4=0)),
    P("ADD", ["HSEL", "HLVL", "CUTOFF", "VOL"], (
        "K1 sceglie l'armonica (da 1 a 8), K2 ne regola il volume. Guarda l'onda che nasce dalla somma.",
        "K1 picks the harmonic (1 to 8), K2 sets its level. Watch the wave that the sum makes.",
        "K1 elige el armónico (de 1 a 8), K2 regula su nivel. Mira la onda que nace de la suma.",
        "K1 wählt den Oberton (1 bis 8), K2 stellt seinen Pegel ein. Sieh die Welle, die aus der Summe entsteht.",
        "K1 choisit l'harmonique (de 1 à 8), K2 règle son niveau. Regarde l'onde qui naît de la somme.")),
    S("ADD", ["HSEL", "HLVL", "CUTOFF", "VOL"], ("H3", "ge", 50, 0), (
        "Parti dalla sola fondamentale. Con K1 scegli l'armonica 3 e con K2 alzala al 50%: il suono si accende, come "
        "un registro d'organo.",
        "Start from the fundamental alone. With K1 pick harmonic 3 and with K2 raise it to 50%: the sound lights up, "
        "like an organ stop.",
        "Empieza solo con la fundamental. Con K1 elige el armónico 3 y con K2 súbelo al 50%: el sonido se enciende, "
        "como un registro de órgano.",
        "Beginne nur mit dem Grundton. Wähl mit K1 den Oberton 3 und heb ihn mit K2 auf 50%: der Klang leuchtet auf, "
        "wie ein Orgelregister.",
        "Pars de la fondamentale seule. Avec K1 choisis l'harmonique 3 et avec K2 monte-la à 50% : le son s'allume, "
        "comme un registre d'orgue."), set=dict(H1=100, H2=0, H3=0, H4=0, H5=0, H6=0, H7=0, H8=0)),
    T("WT", (
        "Wavetable: una fila di forme d'onda. La posizione sceglie quale leggere, e tra una e l'altra il synth le "
        "mescola. Muovendo la posizione il timbro si trasforma senza toccare il filtro.",
        "Wavetable: a row of waveforms. The position picks which one to read, and the synth blends between them. "
        "Moving the position transforms the timbre without touching the filter.",
        "Wavetable: una fila de formas de onda. La posición elige cuál leer, y el sintetizador las mezcla entre una y "
        "otra. Al mover la posición el timbre se transforma sin tocar el filtro.",
        "Wavetable: eine Reihe von Wellenformen. Die Position wählt, welche gelesen wird, dazwischen mischt der Synth. "
        "Bewegt man die Position, verwandelt sich die Klangfarbe, ohne den Filter zu berühren.",
        "Wavetable : une rangée de formes d'onde. La position choisit laquelle lire, et le synthé les mélange entre "
        "elles. En bougeant la position, le timbre se transforme sans toucher au filtre."),
      set=dict(ENGINE=E_WT, WTPOS=30)),
    P("WT", ["WTPOS", "LDEPTH", "LRATE", "CUTOFF"], (
        "K1 la posizione. K2 e K3 fanno muovere la posizione all'LFO (profondità e velocità): il suono si evolve da "
        "solo.",
        "K1 the position. K2 and K3 let the LFO move the position (depth and rate): the sound evolves by itself.",
        "K1 la posición. K2 y K3 hacen que el LFO mueva la posición (profundidad y velocidad): el sonido evoluciona "
        "solo.",
        "K1 die Position. K2 und K3 lassen den LFO die Position bewegen (Tiefe und Tempo): der Klang entwickelt sich "
        "von selbst.",
        "K1 la position. K2 et K3 font bouger la position par le LFO (profondeur et vitesse) : le son évolue tout "
        "seul."), set=dict(ENGINE=E_WT, LDEST=L_WT, LDEPTH=0, LRATE=lfo(0.5))),
    S("WT", ["WTPOS", "LDEPTH", "LRATE", "CUTOFF"], ("LDEPTH", "ge", 40, 0), (
        "Alza la profondità (K2) al 40% e tieni un accordo: la forma d'onda scorre avanti e indietro, il suono cambia "
        "colore da solo.",
        "Turn the depth (K2) up to 40% and hold a chord: the wave slides back and forth, the sound changes colour by "
        "itself.",
        "Sube la profundidad (K2) al 40% y mantén un acorde: la onda se desliza adelante y atrás, el sonido cambia de "
        "color solo.",
        "Dreh die Tiefe (K2) auf 40% und halte einen Akkord: die Welle gleitet hin und her, der Klang ändert von selbst "
        "die Farbe.",
        "Monte la profondeur (K2) à 40% et tiens un accord : l'onde glisse d'avant en arrière, le son change de couleur "
        "tout seul."), set=dict(ENGINE=E_WT, LDEST=L_WT, LDEPTH=0, LRATE=lfo(0.5), WTPOS=50)),
])

# 11 --- voices and glide
chapter(3, ("Mono, poly e glide", "Mono, poly and glide", "Mono, poly y glide", "Mono, Poly und Glide",
            "Mono, poly et glide"), "legato", dict(CUTOFF=60, RESO=15), [
    T("VOICES", (
        "Polifonico: più note insieme, per accordi e pad. Monofonico: una nota alla volta, per bassi e melodie. Ogni "
        "nota occupa una voce, una catena completa di oscillatore, filtro e inviluppi.",
        "Polyphonic: several notes at once, for chords and pads. Monophonic: one note at a time, for basses and "
        "melodies. Each note takes a voice, a whole chain of oscillator, filter and envelopes.",
        "Polifónico: varias notas a la vez, para acordes y pads. Monofónico: una nota cada vez, para bajos y melodías. "
        "Cada nota ocupa una voz, una cadena completa de oscilador, filtro y envolventes.",
        "Polyphon: mehrere Töne zugleich, für Akkorde und Pads. Monophon: ein Ton auf einmal, für Bässe und Melodien. "
        "Jeder Ton belegt eine Stimme, eine ganze Kette aus Oszillator, Filter und Hüllkurven.",
        "Polyphonique : plusieurs notes à la fois, pour les accords et les nappes. Monophonique : une note à la fois, "
        "pour les basses et les mélodies. Chaque note prend une voix, une chaîne entière d'oscillateur, filtre et "
        "enveloppes.")),
    T("GLIDE", (
        "Il glide (o portamento) fa scivolare l'altezza da una nota all'altra invece di saltare. In mono, suonando note "
        "legate, l'inviluppo non riparte: la melodia 'canta'.",
        "Glide (or portamento) slides the pitch from one note to the next instead of jumping. In mono, playing legato, "
        "the envelope doesn't restart: the melody 'sings'.",
        "El glide (o portamento) hace deslizar la altura de una nota a otra en vez de saltar. En mono, tocando ligado, "
        "la envolvente no vuelve a empezar: la melodía 'canta'.",
        "Glide (oder Portamento) lässt die Tonhöhe von einem Ton zum nächsten gleiten, statt zu springen. In Mono, "
        "legato gespielt, startet die Hüllkurve nicht neu: die Melodie 'singt'.",
        "Le glide (ou portamento) fait glisser la hauteur d'une note à l'autre au lieu de sauter. En mono, en jouant "
        "lié, l'enveloppe ne redémarre pas : la mélodie 'chante'."), set=dict(POLY=1, GLIDE=t(120))),
    P("GLIDE", ["POLY", "GLIDE", "WAVE", "CUTOFF"], (
        "K1 poly o mono, K2 il glide, K3 l'onda, K4 il cutoff. Suona una melodia con note legate.",
        "K1 poly or mono, K2 the glide, K3 the wave, K4 the cutoff. Play a melody with joined-up notes.",
        "K1 poly o mono, K2 el glide, K3 la onda, K4 el cutoff. Toca una melodía con notas ligadas.",
        "K1 Poly oder Mono, K2 das Glide, K3 die Welle, K4 der Cutoff. Spiel eine Melodie mit gebundenen Tönen.",
        "K1 poly ou mono, K2 le glide, K3 l'onde, K4 le cutoff. Joue une mélodie avec des notes liées.")),
    S("VOICES", ["POLY", "GLIDE", "WAVE", "CUTOFF"], ("POLY", "eq", 1, 0), (
        "Prova un accordo: suonano tutte le note. Ora passa a mono (K1) e riprova: suona una nota sola, l'ultima.",
        "Try a chord: every note sounds. Now switch to mono (K1) and try again: only one note plays, the last one.",
        "Prueba un acorde: suenan todas las notas. Ahora pasa a mono (K1) y vuelve a probar: suena una sola nota, la "
        "última.",
        "Probier einen Akkord: alle Töne klingen. Schalte jetzt auf Mono (K1) und probier nochmal: nur ein Ton klingt, "
        "der letzte.",
        "Essaie un accord : toutes les notes sonnent. Passe maintenant en mono (K1) et réessaie : une seule note joue, "
        "la dernière."), set=dict(POLY=0, GLIDE=0)),
    S("GLIDE", ["POLY", "GLIDE", "WAVE", "CUTOFF"], ("GLIDE", "eq", t(120), 4), (
        "Alza il glide (K2) a circa 120 ms e suona due note lontane: l'altezza scivola dall'una all'altra.",
        "Turn glide (K2) up to about 120 ms and play two notes far apart: the pitch slides from one to the other.",
        "Sube el glide (K2) a unos 120 ms y toca dos notas lejanas: la altura se desliza de una a otra.",
        "Dreh das Glide (K2) auf etwa 120 ms und spiel zwei weit entfernte Töne: die Tonhöhe gleitet vom einen zum "
        "anderen.",
        "Monte le glide (K2) vers 120 ms et joue deux notes éloignées : la hauteur glisse de l'une à l'autre."),
      set=dict(POLY=1, GLIDE=0)),
])

# 12 --- effects
chapter(3, ("Gli effetti", "Effects", "Los efectos", "Effekte", "Les effets"), "chord",
        dict(CUTOFF=55, RESO=20, S=80, R=t(400)), [
    T("FX", (
        "Gli effetti fanno parte del suono. Drive (saturazione): spinge il segnale finché si sporca, aggiunge "
        "armoniche e calore. Chorus: copia il suono e lo fa ondeggiare, per renderlo largo.",
        "Effects are part of the sound. Drive (saturation) pushes the signal until it gets dirty, adding harmonics and "
        "warmth. Chorus copies the sound and makes it waver, so it gets wide.",
        "Los efectos forman parte del sonido. Drive (saturación): empuja la señal hasta ensuciarla, añade armónicos y "
        "calor. Chorus: copia el sonido y lo hace ondular, para ensancharlo.",
        "Effekte gehören zum Klang. Drive (Sättigung) treibt das Signal, bis es schmutzig wird, und fügt Obertöne und "
        "Wärme hinzu. Chorus kopiert den Klang und lässt ihn schwanken, damit er breit wird.",
        "Les effets font partie du son. Drive (saturation) : pousse le signal jusqu'à le salir, ajoute des harmoniques "
        "et de la chaleur. Chorus : copie le son et le fait onduler, pour l'élargir."), set=dict(DRIVE=40)),
    T("NONE", (
        "Altri effetti che troverai sui synth: il delay ripete il suono come un'eco; il riverbero simula una stanza o "
        "una chiesa. Un consiglio: quasi sempre basta meno riverbero di quanto pensi. L'ordine classico è: "
        "saturazione, chorus, delay, riverbero.",
        "Other effects you'll find on synths: delay repeats the sound like an echo; reverb imitates a room or a church. "
        "A tip: you almost always need less reverb than you think. The classic order is: saturation, chorus, delay, "
        "reverb.",
        "Otros efectos que encontrarás en los sintetizadores: el delay repite el sonido como un eco; la reverberación "
        "imita una sala o una iglesia. Un consejo: casi siempre basta menos reverb de lo que piensas. El orden clásico "
        "es: saturación, chorus, delay, reverb.",
        "Weitere Effekte auf Synths: das Delay wiederholt den Klang wie ein Echo; der Hall ahmt einen Raum oder eine "
        "Kirche nach. Ein Tipp: meistens braucht man weniger Hall, als man denkt. Die klassische Reihenfolge: "
        "Sättigung, Chorus, Delay, Hall.",
        "D'autres effets que tu trouveras sur les synthés : le delay répète le son comme un écho ; la réverbération "
        "imite une pièce ou une église. Un conseil : il faut presque toujours moins de réverb qu'on ne le pense. "
        "L'ordre classique : saturation, chorus, delay, réverb.")),
    P("FX", ["DRIVE", "CHORUS", "CUTOFF", "VOL"], (
        "K1 il drive, K2 il chorus, K3 il cutoff, K4 il volume. Suona accordi e confronta con gli effetti a zero.",
        "K1 the drive, K2 the chorus, K3 the cutoff, K4 the volume. Play chords and compare with the effects at zero.",
        "K1 el drive, K2 el chorus, K3 el cutoff, K4 el volumen. Toca acordes y compara con los efectos a cero.",
        "K1 der Drive, K2 der Chorus, K3 der Cutoff, K4 die Lautstärke. Spiel Akkorde und vergleiche mit den Effekten "
        "auf null.",
        "K1 le drive, K2 le chorus, K3 le cutoff, K4 le volume. Joue des accords et compare avec les effets à zéro."),
      set=dict(DRIVE=0)),
    S("FX", ["DRIVE", "CHORUS", "CUTOFF", "VOL"], ("CHORUS", "ge", 50, 0), (
        "Alza il chorus (K2) a metà e suona un accordo: il suono si allarga, sembra che suonino più strumenti.",
        "Turn the chorus (K2) up halfway and play a chord: the sound spreads out, as if several instruments were "
        "playing.",
        "Sube el chorus (K2) a la mitad y toca un acorde: el sonido se ensancha, parece que tocan varios instrumentos.",
        "Dreh den Chorus (K2) auf die Hälfte und spiel einen Akkord: der Klang wird breiter, als spielten mehrere "
        "Instrumente.",
        "Monte le chorus (K2) à moitié et joue un accord : le son s'élargit, comme si plusieurs instruments jouaient."),
      set=dict(CHORUS=0, DRIVE=0)),
    S("FX", ["DRIVE", "CHORUS", "CUTOFF", "VOL"], ("DRIVE", "ge", 40, 0), (
        "Ora alza il drive (K1) al 40%: il suono si scalda e si sporca, come un amplificatore spinto al massimo.",
        "Now turn the drive (K1) up to 40%: the sound gets warm and gritty, like an amp turned all the way up.",
        "Ahora sube el drive (K1) al 40%: el sonido se calienta y se ensucia, como un amplificador al máximo.",
        "Dreh jetzt den Drive (K1) auf 40%: der Klang wird warm und rau, wie ein voll aufgedrehter Verstärker.",
        "Monte maintenant le drive (K1) à 40% : le son chauffe et se salit, comme un ampli poussé à fond."),
      set=dict(DRIVE=0)),
])

# 13 --- a bass
OSCK = ["OCT", "OSC2", "WAVE2", "DETUNE"]
FILTK = ["CUTOFF", "RESO", "FAMT", "FD"]
AMPK = ["A", "D", "S", "R"]
chapter(4, ("Un basso", "A bass", "Un bajo", "Ein Bass", "Une basse"), "bass", {}, build=True, pages=[
    T("PATCH", (
        "Il basso è la base del brano: deve essere solido e chiaro. Lo costruiamo da zero, un blocco alla volta, "
        "partendo dalla patch init: dente di sega, filtro aperto, volume da organo.",
        "The bass is the foundation of a song: solid and clear. We'll build it from scratch, one block at a time, "
        "starting from the init patch: a saw, open filter, organ-style volume.",
        "El bajo es la base de la canción: debe ser sólido y claro. Lo construimos desde cero, bloque a bloque, "
        "partiendo del patch init: diente de sierra, filtro abierto, volumen de órgano.",
        "Der Bass ist das Fundament eines Songs: fest und klar. Wir bauen ihn von Grund auf, Block für Block, ausgehend "
        "vom Init-Patch: Sägezahn, offener Filter, Lautstärke wie eine Orgel.",
        "La basse est la base du morceau : solide et claire. On la construit à partir de zéro, bloc par bloc, en "
        "partant du patch init : dent de scie, filtre ouvert, volume d'orgue.")),
    S("WAVE", OSCK, ("OCT", "eq", -1, 0), (
        "1. Sorgenti: abbassa l'ottava (K1) a -1. Il suono prende peso.",
        "1. Sources: lower the octave (K1) to -1. The sound gains weight.",
        "1. Fuentes: baja la octava (K1) a -1. El sonido gana peso.",
        "1. Quellen: senk die Oktave (K1) auf -1. Der Klang bekommt Gewicht.",
        "1. Sources : baisse l'octave (K1) à -1. Le son prend du poids.")),
    S("WAVE", OSCK, ("OSC2", "eq", 70, 6), (
        "2. Aggiungi il secondo oscillatore, una quadra leggermente scordata: porta il suo livello (K2) al 70%.",
        "2. Add the second oscillator, a slightly detuned square: bring its level (K2) to 70%.",
        "2. Añade el segundo oscilador, una cuadrada un poco desafinada: lleva su nivel (K2) al 70%.",
        "2. Nimm den zweiten Oszillator dazu, ein leicht verstimmtes Rechteck: stell seinen Pegel (K2) auf 70%.",
        "2. Ajoute le second oscillateur, un carré un peu désaccordé : monte son niveau (K2) à 70%."),
      set=dict(WAVE2=SQUARE, DETUNE=5)),
    S("FILTER", FILTK, ("CUTOFF", "eq", 25, 4), (
        "3. Filtro: porta il cutoff (K1) al 25%. Il basso diventa morbido e scuro.",
        "3. Filter: set the cutoff (K1) to 25%. The bass turns soft and dark.",
        "3. Filtro: lleva el cutoff (K1) al 25%. El bajo se vuelve suave y oscuro.",
        "3. Filter: stell den Cutoff (K1) auf 25%. Der Bass wird weich und dunkel.",
        "3. Filtre : règle le cutoff (K1) à 25%. La basse devient douce et sombre.")),
    S("FILTER", FILTK, ("RESO", "eq", 20, 5), (
        "4. Un po' di risonanza (K2): circa 20%.",
        "4. A little resonance (K2): about 20%.",
        "4. Un poco de resonancia (K2): cerca del 20%.",
        "4. Etwas Resonanz (K2): etwa 20%.",
        "4. Un peu de résonance (K2) : environ 20%.")),
    S("FENV", FILTK, ("FAMT", "eq", 45, 5), (
        "5. Inviluppo del filtro: amount (K3) al 45%. Ogni nota ora ha un attacco chiaro che la fa 'parlare'.",
        "5. Filter envelope: amount (K3) at 45%. Every note now has a clear attack that makes it 'talk'.",
        "5. Envolvente del filtro: amount (K3) al 45%. Ahora cada nota tiene un ataque claro que la hace 'hablar'.",
        "5. Filterhüllkurve: Amount (K3) auf 45%. Jeder Ton hat jetzt einen klaren Anschlag, der ihn 'sprechen' lässt.",
        "5. Enveloppe du filtre : amount (K3) à 45%. Chaque note a maintenant une attaque nette qui la fait 'parler'."),
      set=dict(FD=t(180), FS=20, FR=t(100))),
    S("ADSR", AMPK, ("R", "eq", t(60), 4), (
        "6. Volume: release (K4) breve, circa 60 ms, così le note non si impastano.",
        "6. Volume: a short release (K4), about 60 ms, so the notes don't smear together.",
        "6. Volumen: release (K4) corto, unos 60 ms, para que las notas no se mezclen.",
        "6. Lautstärke: ein kurzes Release (K4), etwa 60 ms, damit die Töne nicht verschmieren.",
        "6. Volume : un release (K4) court, vers 60 ms, pour que les notes ne se mélangent pas."),
      set=dict(A=0, D=t(300), S=80, R=t(300))),
    S("GLIDE", ["POLY", "GLIDE", "VELAMT", "VOL"], ("POLY", "eq", 1, 0), (
        "7. Voce: passa a mono (K1). Con un po' di glide le note legate scivolano. Il basso è pronto: suona una linea!",
        "7. Voice: switch to mono (K1). With a little glide, joined notes slide. The bass is ready: play a bass line!",
        "7. Voz: pasa a mono (K1). Con un poco de glide las notas ligadas se deslizan. El bajo está listo: toca una "
        "línea.",
        "7. Stimme: schalte auf Mono (K1). Mit etwas Glide gleiten gebundene Töne. Der Bass ist fertig: spiel eine "
        "Basslinie!",
        "7. Voix : passe en mono (K1). Avec un peu de glide, les notes liées glissent. La basse est prête : joue une "
        "ligne !"), set=dict(GLIDE=t(40))),
    T("PATCH", (
        "Varianti da provare: acid, con risonanza al 70% e decay del filtro breve; sub, solo una sinusoide senza "
        "filtro. Errore comune: troppe basse. Un basso deve essere chiaro, non enorme.",
        "Variations to try: acid, with resonance at 70% and a short filter decay; sub, just a sine with no filter. "
        "Common mistake: too much low end. A bass should be clear, not huge.",
        "Variantes para probar: acid, con resonancia al 70% y decay del filtro corto; sub, solo una senoidal sin "
        "filtro. Error común: demasiados graves. Un bajo debe ser claro, no enorme.",
        "Varianten zum Ausprobieren: Acid, mit Resonanz auf 70% und kurzem Filter-Decay; Sub, nur ein Sinus ohne "
        "Filter. Häufiger Fehler: zu viel Tiefe. Ein Bass soll klar sein, nicht riesig.",
        "Variantes à essayer : acid, avec la résonance à 70% et un decay du filtre court ; sub, juste une sinusoïde "
        "sans filtre. Erreur fréquente : trop de graves. Une basse doit être claire, pas énorme.")),
])

# 14 --- a pad
chapter(4, ("Un pad", "A pad", "Un pad", "Ein Pad", "Une nappe"), "chord", {}, build=True, pages=[
    T("PATCH", (
        "Il pad è il tappeto del brano: lento, ampio, sempre in movimento ma discreto. Riempie lo spazio senza chiedere "
        "attenzione. Partiamo di nuovo dalla patch init.",
        "A pad is the carpet of a song: slow, wide, always moving but quietly. It fills the space without asking for "
        "attention. We start again from the init patch.",
        "El pad es la alfombra de la canción: lento, amplio, siempre en movimiento pero discreto. Llena el espacio sin "
        "pedir atención. Partimos otra vez del patch init.",
        "Ein Pad ist der Teppich eines Songs: langsam, breit, immer in Bewegung, aber dezent. Er füllt den Raum, ohne "
        "Aufmerksamkeit zu verlangen. Wir starten wieder beim Init-Patch.",
        "La nappe (pad) est le tapis du morceau : lente, large, toujours en mouvement mais discrète. Elle remplit "
        "l'espace sans attirer l'attention. On repart du patch init.")),
    S("WAVE", ["OSC2", "DETUNE", "CUTOFF", "RESO"], ("OSC2", "ge", 90, 0), (
        "1. Sorgenti: due denti di sega. Alza il secondo oscillatore (K1) al 100%.",
        "1. Sources: two saws. Turn the second oscillator (K1) up to 100%.",
        "1. Fuentes: dos dientes de sierra. Sube el segundo oscilador (K1) al 100%.",
        "1. Quellen: zwei Sägezähne. Dreh den zweiten Oszillator (K1) auf 100%.",
        "1. Sources : deux dents de scie. Monte le second oscillateur (K1) à 100%."), set=dict(WAVE2=SAW)),
    S("WAVE", ["OSC2", "DETUNE", "CUTOFF", "RESO"], ("DETUNE", "eq", 8, 2), (
        "2. Scordalo un poco: detune (K2) a circa 8 centesimi. Il suono si allarga.",
        "2. Detune it a little: detune (K2) at about 8 cents. The sound widens.",
        "2. Desafínalo un poco: detune (K2) a unos 8 cents. El sonido se ensancha.",
        "2. Verstimm ihn etwas: Detune (K2) auf etwa 8 Cent. Der Klang wird breiter.",
        "2. Désaccorde-le un peu : detune (K2) vers 8 cents. Le son s'élargit.")),
    S("FILTER", ["OSC2", "DETUNE", "CUTOFF", "RESO"], ("CUTOFF", "eq", 45, 4), (
        "3. Filtro: cutoff (K3) al 45%, morbido.",
        "3. Filter: cutoff (K3) at 45%, soft.",
        "3. Filtro: cutoff (K3) al 45%, suave.",
        "3. Filter: Cutoff (K3) auf 45%, weich.",
        "3. Filtre : cutoff (K3) à 45%, doux."), set=dict(RESO=10)),
    S("ADSR", AMPK, ("A", "eq", t(800), 4), (
        "4. Attack (K1) lento, circa 800 ms: le note entrano piano, senza colpo.",
        "4. A slow attack (K1), about 800 ms: the notes fade in, with no hit.",
        "4. Attack (K1) lento, unos 800 ms: las notas entran despacio, sin golpe.",
        "4. Langsames Attack (K1), etwa 800 ms: die Töne blenden ein, ohne Schlag.",
        "4. Attack (K1) lente, vers 800 ms : les notes entrent doucement, sans coup."),
      set=dict(D=t(1000), S=90, R=t(300))),
    S("ADSR", AMPK, ("R", "eq", t(2500), 4), (
        "5. Release (K4) lungo, circa 2,5 s: gli accordi si fondono l'uno nell'altro.",
        "5. A long release (K4), about 2.5 s: the chords melt into each other.",
        "5. Release (K4) largo, unos 2,5 s: los acordes se funden unos con otros.",
        "5. Langes Release (K4), etwa 2,5 s: die Akkorde verschmelzen ineinander.",
        "5. Release (K4) long, vers 2,5 s : les accords se fondent les uns dans les autres.")),
    S("LFO", ["LRATE", "LDEPTH", "LDEST", "CHORUS"], ("LDEPTH", "eq", 20, 5), (
        "6. Movimento: un LFO lentissimo sul filtro. Porta la profondità (K2) al 20%: il pad respira.",
        "6. Movement: a very slow LFO on the filter. Set the depth (K2) to 20%: the pad breathes.",
        "6. Movimiento: un LFO lentísimo en el filtro. Lleva la profundidad (K2) al 20%: el pad respira.",
        "6. Bewegung: ein sehr langsamer LFO auf dem Filter. Stell die Tiefe (K2) auf 20%: das Pad atmet.",
        "6. Mouvement : un LFO très lent sur le filtre. Règle la profondeur (K2) à 20% : la nappe respire."),
      set=dict(LDEST=L_CUTOFF, LSHAPE=1, LRATE=lfo(0.15))),
    S("FX", ["LRATE", "LDEPTH", "LDEST", "CHORUS"], ("CHORUS", "ge", 50, 0), (
        "7. Effetti: chorus (K4) al 50%. Il pad è pronto: suona accordi di 3 o 4 note!",
        "7. Effects: chorus (K4) at 50%. The pad is ready: play chords of 3 or 4 notes!",
        "7. Efectos: chorus (K4) al 50%. El pad está listo: toca acordes de 3 o 4 notas.",
        "7. Effekte: Chorus (K4) auf 50%. Das Pad ist fertig: spiel Akkorde mit 3 oder 4 Tönen!",
        "7. Effets : chorus (K4) à 50%. La nappe est prête : joue des accords de 3 ou 4 notes !")),
    T("PATCH", (
        "Perché funziona: l'attacco lento non disturba la ritmica, il release lungo lega gli accordi, l'LFO lento "
        "impedisce che il suono sembri fermo. Spegni l'LFO per un attimo: se ti manca, è dosato bene.",
        "Why it works: the slow attack stays out of the rhythm's way, the long release joins the chords, the slow LFO "
        "keeps the sound from standing still. Turn the LFO off for a moment: if you miss it, it's just right.",
        "Por qué funciona: el ataque lento no molesta al ritmo, el release largo une los acordes, el LFO lento evita "
        "que el sonido parezca quieto. Apaga el LFO un momento: si lo echas de menos, está bien dosificado.",
        "Warum es klappt: das langsame Attack stört den Rhythmus nicht, das lange Release verbindet die Akkorde, der "
        "langsame LFO lässt den Klang nicht stillstehen. Schalte den LFO kurz aus: fehlt er dir, ist er richtig "
        "dosiert.",
        "Pourquoi ça marche : l'attaque lente ne gêne pas le rythme, le release long relie les accords, le LFO lent "
        "empêche le son d'être figé. Coupe le LFO un instant : s'il te manque, il est bien dosé.")),
])

# 15 --- a lead
chapter(4, ("Un lead", "A lead", "Un lead", "Ein Lead", "Un lead"), "legato", {}, build=True, pages=[
    T("PATCH", (
        "Il lead è la melodia principale: deve 'cantare' sopra tutto il resto ed essere espressivo sotto le dita. Il "
        "timbro conta, ma contano ancora di più glide e vibrato.",
        "The lead plays the main melody: it has to 'sing' above everything else and respond under your fingers. The "
        "timbre matters, but glide and vibrato matter even more.",
        "El lead es la melodía principal: debe 'cantar' por encima de todo y ser expresivo bajo los dedos. El timbre "
        "importa, pero el glide y el vibrato importan aún más.",
        "Der Lead spielt die Hauptmelodie: er muss über allem 'singen' und unter den Fingern ausdrucksvoll sein. Die "
        "Klangfarbe zählt, aber Glide und Vibrato zählen noch mehr.",
        "Le lead joue la mélodie principale : il doit 'chanter' au-dessus de tout et être expressif sous les doigts. "
        "Le timbre compte, mais le glide et le vibrato comptent encore plus.")),
    S("WAVE", ["OSC2", "WAVE2", "CUTOFF", "RESO"], ("OSC2", "eq", 60, 6), (
        "1. Sorgenti: dente di sega più una quadra. Alza il secondo oscillatore (K1) al 60%.",
        "1. Sources: a saw plus a square. Turn the second oscillator (K1) up to 60%.",
        "1. Fuentes: diente de sierra más una cuadrada. Sube el segundo oscilador (K1) al 60%.",
        "1. Quellen: Sägezahn plus Rechteck. Dreh den zweiten Oszillator (K1) auf 60%.",
        "1. Sources : dent de scie plus carré. Monte le second oscillateur (K1) à 60%."),
      set=dict(WAVE2=SQUARE, DETUNE=6)),
    S("FILTER", ["OSC2", "WAVE2", "CUTOFF", "RESO"], ("CUTOFF", "eq", 55, 4), (
        "2. Filtro: cutoff (K3) al 55%.",
        "2. Filter: cutoff (K3) at 55%.",
        "2. Filtro: cutoff (K3) al 55%.",
        "2. Filter: Cutoff (K3) auf 55%.",
        "2. Filtre : cutoff (K3) à 55%.")),
    S("FILTER", ["OSC2", "WAVE2", "CUTOFF", "RESO"], ("RESO", "eq", 25, 5), (
        "3. Risonanza (K4) al 25%: un po' di carattere.",
        "3. Resonance (K4) at 25%: a bit of character.",
        "3. Resonancia (K4) al 25%: un poco de carácter.",
        "3. Resonanz (K4) auf 25%: etwas Charakter.",
        "3. Résonance (K4) à 25% : un peu de caractère.")),
    S("FENV", ["FAMT", "FD", "FS", "KTRACK"], ("FAMT", "eq", 30, 5), (
        "4. Inviluppo del filtro: amount (K1) al 30%, un lampo all'inizio di ogni nota.",
        "4. Filter envelope: amount (K1) at 30%, a flash at the start of every note.",
        "4. Envolvente del filtro: amount (K1) al 30%, un destello al principio de cada nota.",
        "4. Filterhüllkurve: Amount (K1) auf 30%, ein Blitz am Anfang jedes Tons.",
        "4. Enveloppe du filtre : amount (K1) à 30%, un éclair au début de chaque note."),
      set=dict(FA=t(10), FD=t(400), FS=50, FR=t(200))),
    S("GLIDE", ["POLY", "GLIDE", "LRATE", "LDEPTH"], ("POLY", "eq", 1, 0), (
        "5. Espressione: mono (K1), perché una melodia è fatta di una nota alla volta.",
        "5. Expression: mono (K1), because a melody is one note at a time.",
        "5. Expresión: mono (K1), porque una melodía es una nota cada vez.",
        "5. Ausdruck: Mono (K1), denn eine Melodie ist ein Ton nach dem anderen.",
        "5. Expression : mono (K1), parce qu'une mélodie, c'est une note à la fois.")),
    S("GLIDE", ["POLY", "GLIDE", "LRATE", "LDEPTH"], ("GLIDE", "eq", t(60), 4), (
        "6. Glide (K2) a circa 60 ms: le note legate scivolano come una voce.",
        "6. Glide (K2) at about 60 ms: joined notes slide like a voice.",
        "6. Glide (K2) a unos 60 ms: las notas ligadas se deslizan como una voz.",
        "6. Glide (K2) auf etwa 60 ms: gebundene Töne gleiten wie eine Stimme.",
        "6. Glide (K2) vers 60 ms : les notes liées glissent comme une voix.")),
    S("LFO", ["POLY", "GLIDE", "LRATE", "LDEPTH"], ("LDEPTH", "eq", 12, 4), (
        "7. Vibrato: l'LFO a 5,5 Hz sull'altezza. Profondità (K4) al 12%: poco, giusto per farlo vivere.",
        "7. Vibrato: the LFO at 5.5 Hz on pitch. Depth (K4) at 12%: just a little, to bring it to life.",
        "7. Vibrato: el LFO a 5,5 Hz en la altura. Profundidad (K4) al 12%: poco, lo justo para darle vida.",
        "7. Vibrato: der LFO mit 5,5 Hz auf der Tonhöhe. Tiefe (K4) auf 12%: nur ein wenig, damit er lebt.",
        "7. Vibrato : le LFO à 5,5 Hz sur la hauteur. Profondeur (K4) à 12% : juste un peu, pour le rendre vivant."),
      set=dict(LDEST=L_PITCH, LSHAPE=0, LRATE=lfo(5.5))),
    S("FX", ["DRIVE", "CHORUS", "VOL"], ("DRIVE", "eq", 20, 6), (
        "8. Un filo di drive (K1), circa 20%. Il lead è pronto: suona una melodia lenta!",
        "8. A touch of drive (K1), about 20%. The lead is ready: play a slow melody!",
        "8. Un poco de drive (K1), cerca del 20%. El lead está listo: toca una melodía lenta.",
        "8. Ein Hauch Drive (K1), etwa 20%. Der Lead ist fertig: spiel eine langsame Melodie!",
        "8. Une pointe de drive (K1), vers 20%. Le lead est prêt : joue une mélodie lente !")),
])

# 16 --- a stab
chapter(4, ("Uno stab", "A stab", "Un stab", "Ein Stab", "Un stab"), "stab", {}, build=True, pages=[
    T("PATCH", (
        "Lo stab è un accordo breve e percussivo, tipico della musica house e pop elettronica. Il suo segreto: la "
        "durata della nota non dipende da quanto tieni premuti i tasti.",
        "A stab is a short, punchy chord, typical of house and electronic pop. Its secret: how long it lasts doesn't "
        "depend on how long you hold the keys.",
        "El stab es un acorde corto y percusivo, típico del house y del pop electrónico. Su secreto: la duración no "
        "depende de cuánto mantienes las teclas.",
        "Ein Stab ist ein kurzer, knackiger Akkord, typisch für House und Elektropop. Sein Geheimnis: wie lange er "
        "klingt, hängt nicht davon ab, wie lange du die Tasten hältst.",
        "Le stab est un accord bref et percutant, typique de la house et de la pop électronique. Son secret : sa durée "
        "ne dépend pas du temps où tu tiens les touches.")),
    S("WAVE", ["OSC2", "DETUNE", "CUTOFF", "RESO"], ("DETUNE", "eq", 7, 2), (
        "1. Due denti di sega: detune (K2) a circa 7 centesimi.",
        "1. Two saws: detune (K2) at about 7 cents.",
        "1. Dos dientes de sierra: detune (K2) a unos 7 cents.",
        "1. Zwei Sägezähne: Detune (K2) auf etwa 7 Cent.",
        "1. Deux dents de scie : detune (K2) vers 7 cents."), set=dict(WAVE2=SAW, OSC2=100)),
    S("FILTER", ["OSC2", "DETUNE", "CUTOFF", "RESO"], ("CUTOFF", "eq", 30, 4), (
        "2. Filtro: cutoff (K3) al 30%, risonanza al 30%.",
        "2. Filter: cutoff (K3) at 30%, resonance at 30%.",
        "2. Filtro: cutoff (K3) al 30%, resonancia al 30%.",
        "2. Filter: Cutoff (K3) auf 30%, Resonanz auf 30%.",
        "2. Filtre : cutoff (K3) à 30%, résonance à 30%."), set=dict(RESO=30)),
    S("FENV", ["FAMT", "FD", "FS", "FR"], ("FAMT", "eq", 55, 5), (
        "3. Inviluppo del filtro corto: amount (K1) al 55%. Ogni accordo ha un lampo brillante.",
        "3. A short filter envelope: amount (K1) at 55%. Every chord gets a bright flash.",
        "3. Envolvente del filtro corta: amount (K1) al 55%. Cada acorde tiene un destello brillante.",
        "3. Kurze Filterhüllkurve: Amount (K1) auf 55%. Jeder Akkord bekommt einen hellen Blitz.",
        "3. Enveloppe du filtre courte : amount (K1) à 55%. Chaque accord a un éclair brillant."),
      set=dict(FA=0, FD=t(220), FS=0, FR=t(220))),
    S("ADSR", AMPK, ("S", "le", 3, 0), (
        "4. Volume: sustain (K3) a zero. Ora l'accordo si spegne da solo, anche se tieni i tasti.",
        "4. Volume: sustain (K3) at zero. Now the chord dies away by itself, even if you hold the keys.",
        "4. Volumen: sustain (K3) a cero. Ahora el acorde se apaga solo, aunque mantengas las teclas.",
        "4. Lautstärke: Sustain (K3) auf null. Jetzt verklingt der Akkord von selbst, auch wenn du die Tasten hältst.",
        "4. Volume : sustain (K3) à zéro. L'accord s'éteint maintenant tout seul, même si tu tiens les touches."),
      set=dict(A=0, D=t(350), R=t(350))),
    S("FX", ["DRIVE", "CHORUS", "VOL"], ("CHORUS", "eq", 30, 6), (
        "5. Un po' di chorus (K2), circa 30%. Lo stab è pronto: suona accordi brevi e ritmici!",
        "5. A little chorus (K2), about 30%. The stab is ready: play short, rhythmic chords!",
        "5. Un poco de chorus (K2), cerca del 30%. El stab está listo: toca acordes cortos y rítmicos.",
        "5. Etwas Chorus (K2), etwa 30%. Der Stab ist fertig: spiel kurze, rhythmische Akkorde!",
        "5. Un peu de chorus (K2), vers 30%. Le stab est prêt : joue des accords courts et rythmés !")),
])

# 17 --- drums
DRUMK = ["NOISE", "D", "CUTOFF", "FTYPE"]
chapter(4, ("Le percussioni", "Drums", "La percusión", "Schlagzeug", "Les percussions"), "kick", {}, pages=[
    T("PATCH", (
        "Con oscillatori, rumore, filtri e inviluppi si costruisce un'intera batteria elettronica. Le drum machine "
        "classiche, come la Roland TR-808, funzionano proprio così. Cominciamo dalla cassa.",
        "With oscillators, noise, filters and envelopes you can build a whole electronic drum kit. Classic drum "
        "machines like the Roland TR-808 work just like this. Let's start with the kick.",
        "Con osciladores, ruido, filtros y envolventes se construye toda una batería electrónica. Las cajas de ritmos "
        "clásicas, como la Roland TR-808, funcionan así. Empecemos por el bombo.",
        "Mit Oszillatoren, Rauschen, Filtern und Hüllkurven baut man ein ganzes elektronisches Schlagzeug. Klassische "
        "Drumcomputer wie die Roland TR-808 funktionieren genau so. Fangen wir mit der Bassdrum an.",
        "Avec des oscillateurs, du bruit, des filtres et des enveloppes, on construit toute une batterie électronique. "
        "Les boîtes à rythmes classiques, comme la Roland TR-808, marchent ainsi. Commençons par la grosse caisse."),
      set=dict(WAVE=SINE, A=0, D=t(450), S=0, R=t(300))),
    S("PENV", ["PAMT", "PDEC", "D", "WAVE"], ("PAMT", "eq", 30, 4), (
        "Cassa: una sinusoide. Alza l'inviluppo di pitch (K1) a 30 semitoni e suona una nota grave: tum!",
        "Kick: a sine wave. Turn the pitch envelope (K1) up to 30 semitones and play a low note: boom!",
        "Bombo: una senoidal. Sube la envolvente de altura (K1) a 30 semitonos y toca una nota grave: bum.",
        "Bassdrum: eine Sinuswelle. Dreh die Tonhöhen-Hüllkurve (K1) auf 30 Halbtöne und spiel einen tiefen Ton: bumm!",
        "Grosse caisse : une sinusoïde. Monte l'enveloppe de hauteur (K1) à 30 demi-tons et joue une note grave : "
        "boum !"), set=dict(WAVE=SINE, A=0, D=t(450), S=0, R=t(300), PAMT=0, PDEC=t(40), NOISE=0, FTYPE=LP,
                            CUTOFF=100)),
    S("ADSR", DRUMK, ("NOISE", "eq", 60, 6), (
        "Rullante: un triangolo per il corpo più il rumore per la cordiera. Porta il rumore (K1) al 60%.",
        "Snare: a triangle for the body plus noise for the wires. Bring the noise (K1) to 60%.",
        "Caja: un triángulo para el cuerpo más ruido para la bordonera. Lleva el ruido (K1) al 60%.",
        "Snare: ein Dreieck für den Körper plus Rauschen für den Teppich. Stell das Rauschen (K1) auf 60%.",
        "Caisse claire : un triangle pour le corps et du bruit pour le timbre. Règle le bruit (K1) à 60%."),
      set=dict(WAVE=TRI, PAMT=12, PDEC=t(45), D=t(220), S=0, NOISE=0, FTYPE=LP, CUTOFF=100)),
    S("FILTER", DRUMK, ("FTYPE", "eq", HP, 0), (
        "Hi-hat: solo rumore, corto. Scegli il filtro passa-alto (K4 su HP): restano solo gli acuti, tss!",
        "Hi-hat: just noise, short. Pick the high-pass filter (K4 on HP): only the highs are left, tss!",
        "Hi-hat: solo ruido, corto. Elige el filtro paso alto (K4 en HP): solo quedan los agudos, tss.",
        "Hi-Hat: nur Rauschen, kurz. Wähl den Hochpassfilter (K4 auf HP): nur die Höhen bleiben, tss!",
        "Charleston : juste du bruit, court. Choisis le filtre passe-haut (K4 sur HP) : il ne reste que les aigus, "
        "tss !"), set=dict(NOISE=100, PAMT=0, D=t(50), S=0, CUTOFF=cut(7500), FTYPE=LP)),
    S("ADSR", DRUMK, ("D", "ge", t(380), 0), (
        "Hi-hat aperto: allunga il decay (K2) a circa 400 ms. È proprio il decay dell'hi-hat a decidere il groove.",
        "Open hi-hat: lengthen the decay (K2) to about 400 ms. The hi-hat's decay is what sets the groove.",
        "Hi-hat abierto: alarga el decay (K2) a unos 400 ms. El decay del hi-hat es lo que decide el groove.",
        "Offene Hi-Hat: verlängere das Decay (K2) auf etwa 400 ms. Gerade das Decay der Hi-Hat bestimmt den Groove.",
        "Charleston ouvert : allonge le decay (K2) vers 400 ms. C'est le decay du charleston qui décide du groove."),
      set=dict(NOISE=100, PAMT=0, D=t(50), S=0, CUTOFF=cut(7500), FTYPE=HP)),
])

# 18 --- your synth (the patch carries over from the chapter before)
chapter(5, ("Synth libero", "Free synth", "Synth libre", "Freier Synth", "Synthé libre"), "chord", {}, keep=True,
        pages=[
    P("WAVE", ["ENGINE", "WAVE", "OCT", "NOISE"], (
        "Il tuo synth. Pagina 1, sorgente: K1 il motore (sottrattiva, FM, additiva, wavetable), K2 l'onda, K3 "
        "l'ottava, K4 il rumore.",
        "Your synth. Page 1, source: K1 the engine (subtractive, FM, additive, wavetable), K2 the wave, K3 the octave, "
        "K4 the noise.",
        "Tu sintetizador. Página 1, fuente: K1 el motor (sustractiva, FM, aditiva, wavetable), K2 la onda, K3 la "
        "octava, K4 el ruido.",
        "Dein Synth. Seite 1, Quelle: K1 die Syntheseart (subtraktiv, FM, additiv, Wavetable), K2 die Welle, K3 die "
        "Oktave, K4 das Rauschen.",
        "Ton synthé. Page 1, source : K1 le moteur (soustractive, FM, additive, wavetable), K2 l'onde, K3 l'octave, K4 "
        "le bruit.")),
    P("WAVE", ["OSC2", "WAVE2", "DETUNE", "PW"], (
        "Pagina 2, secondo oscillatore: K1 il livello, K2 l'onda, K3 il detune, K4 la larghezza dell'impulso.",
        "Page 2, second oscillator: K1 the level, K2 the wave, K3 the detune, K4 the pulse width.",
        "Página 2, segundo oscilador: K1 el nivel, K2 la onda, K3 el detune, K4 la anchura del pulso.",
        "Seite 2, zweiter Oszillator: K1 der Pegel, K2 die Welle, K3 das Detune, K4 die Pulsbreite.",
        "Page 2, second oscillateur : K1 le niveau, K2 l'onde, K3 le detune, K4 la largeur d'impulsion.")),
    P("FM", ["RATIO", "INDEX", "MDEC", "FB"], (
        "Pagina 3, FM (con il motore FM): K1 il ratio, K2 l'indice, K3 il decay del modulante, K4 il feedback.",
        "Page 3, FM (with the FM engine): K1 the ratio, K2 the index, K3 the modulator decay, K4 the feedback.",
        "Página 3, FM (con el motor FM): K1 el ratio, K2 el índice, K3 el decay del modulador, K4 el feedback.",
        "Seite 3, FM (mit der FM-Synthese): K1 das Ratio, K2 der Index, K3 das Modulator-Decay, K4 das Feedback.",
        "Page 3, FM (avec le moteur FM) : K1 le ratio, K2 l'indice, K3 le decay du modulateur, K4 le feedback.")),
    P("FILTER", ["CUTOFF", "RESO", "FTYPE", "KTRACK"], (
        "Pagina 4, filtro: K1 il cutoff, K2 la risonanza, K3 il tipo, K4 il key tracking (il filtro segue la nota).",
        "Page 4, filter: K1 the cutoff, K2 the resonance, K3 the type, K4 key tracking (the filter follows the note).",
        "Página 4, filtro: K1 el cutoff, K2 la resonancia, K3 el tipo, K4 el key tracking (el filtro sigue la nota).",
        "Seite 4, Filter: K1 der Cutoff, K2 die Resonanz, K3 der Typ, K4 Key Tracking (der Filter folgt dem Ton).",
        "Page 4, filtre : K1 le cutoff, K2 la résonance, K3 le type, K4 le key tracking (le filtre suit la note).")),
    P("FENV", ["FA", "FD", "FS", "FAMT"], (
        "Pagina 5, inviluppo del filtro: K1 attack, K2 decay, K3 sustain, K4 amount.",
        "Page 5, filter envelope: K1 attack, K2 decay, K3 sustain, K4 amount.",
        "Página 5, envolvente del filtro: K1 attack, K2 decay, K3 sustain, K4 amount.",
        "Seite 5, Filterhüllkurve: K1 Attack, K2 Decay, K3 Sustain, K4 Amount.",
        "Page 5, enveloppe du filtre : K1 attack, K2 decay, K3 sustain, K4 amount.")),
    P("ADSR", ["A", "D", "S", "R"], (
        "Pagina 6, inviluppo del volume: K1 attack, K2 decay, K3 sustain, K4 release.",
        "Page 6, volume envelope: K1 attack, K2 decay, K3 sustain, K4 release.",
        "Página 6, envolvente del volumen: K1 attack, K2 decay, K3 sustain, K4 release.",
        "Seite 6, Lautstärke-Hüllkurve: K1 Attack, K2 Decay, K3 Sustain, K4 Release.",
        "Page 6, enveloppe du volume : K1 attack, K2 decay, K3 sustain, K4 release.")),
    P("LFO", ["LRATE", "LDEPTH", "LDEST", "LSHAPE"], (
        "Pagina 7, LFO: K1 la velocità, K2 la profondità, K3 la destinazione, K4 la forma.",
        "Page 7, LFO: K1 the rate, K2 the depth, K3 the target, K4 the shape.",
        "Página 7, LFO: K1 la velocidad, K2 la profundidad, K3 el destino, K4 la forma.",
        "Seite 7, LFO: K1 das Tempo, K2 die Tiefe, K3 das Ziel, K4 die Form.",
        "Page 7, LFO : K1 la vitesse, K2 la profondeur, K3 la cible, K4 la forme.")),
    P("PENV", ["PAMT", "PDEC", "POLY", "GLIDE"], (
        "Pagina 8: K1 e K2 l'inviluppo di pitch, K3 poly o mono, K4 il glide.",
        "Page 8: K1 and K2 the pitch envelope, K3 poly or mono, K4 the glide.",
        "Página 8: K1 y K2 la envolvente de altura, K3 poly o mono, K4 el glide.",
        "Seite 8: K1 und K2 die Tonhöhen-Hüllkurve, K3 Poly oder Mono, K4 das Glide.",
        "Page 8 : K1 et K2 l'enveloppe de hauteur, K3 poly ou mono, K4 le glide.")),
    P("FX", ["DRIVE", "CHORUS", "VOL", "VELAMT"], (
        "Pagina 9, effetti: K1 il drive, K2 il chorus, K3 il volume, K4 la velocity sul filtro.",
        "Page 9, effects: K1 the drive, K2 the chorus, K3 the volume, K4 velocity on the filter.",
        "Página 9, efectos: K1 el drive, K2 el chorus, K3 el volumen, K4 la velocity en el filtro.",
        "Seite 9, Effekte: K1 der Drive, K2 der Chorus, K3 die Lautstärke, K4 die Velocity auf dem Filter.",
        "Page 9, effets : K1 le drive, K2 le chorus, K3 le volume, K4 la vélocité sur le filtre.")),
])

DIAGS = ["NONE", "SOUND", "WAVE", "BLOCKS", "BLOCKS2", "FILTER", "ADSR", "ADSR4", "FENV", "PENV", "LFO", "LFO5", "FM",
         "FMALG", "ADD", "WT", "VOICES", "GLIDE", "FX", "PATCH"]
DEMOS = ["note", "scale", "chord", "pluck", "kick", "hold", "legato", "bass", "stab"]
KINDS = {"T": 0, "P": 1, "S": 2}
OPS = {"eq": 0, "ge": 1, "le": 2}


def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def langs(tup):
    assert len(tup) == LANGS, tup
    return "{" + ", ".join(cstr(x) for x in tup) + "}"


def pid(name):
    if name in PARAMS:
        return "P_" + name
    return "V_" + name


def main():
    out = []
    w = out.append
    w("/* SPDX-License-Identifier: MPL-2.0")
    w(" * THEORY-1: music theory for the M-VAVE FM-1. */")
    w("/* GENERATED by app/tools/gen_synth.py: the SYNTH section's chapters, pages and words. Edit that, not this. */")
    w("#pragma once")
    w("")
    w("enum { V_HSEL = NPAR, V_HLVL, NVPAR };")
    w("enum { " + ", ".join("SD_" + d for d in DIAGS) + ", NSD };")
    w("enum { " + ", ".join("SDEMO_" + d.upper() for d in DEMOS) + " };")
    w("enum { " + ", ".join("SS_" + k for k, _ in SS) + ", NSS };")
    w("static const char *const SSTR[NSS][NLANGS] = {")
    for k, v in SS:
        w("    " + langs(v) + ",")
    w("};")
    w("static const char *const SMOD[%d][NLANGS] = {" % len(MODULES))
    for m in MODULES:
        w("    " + langs(m) + ",")
    w("};")
    w("static const char *const SPNAME[NVPAR][NLANGS] = {")
    for n in PARAMS + VIRTUAL:
        w("    " + langs(PNAME[n]) + ",")
    w("};")
    for k, vals in ENUMS.items():
        w("static const char *const SENUM_%s[%d][NLANGS] = {" % (k, len(vals)))
        for v in vals:
            w("    " + langs(v) + ",")
        w("};")
    w("static const int16_t SINIT[NPAR] = {" + ", ".join(str(INIT[p]) for p in PARAMS) + "};")
    w("")
    w("typedef struct {")
    w("    uint8_t kind, diag, nk;                  /* 0 read, 1 try, 2 a step; SD_*; knobs used */")
    w("    uint8_t knob[4];                         /* P_* or V_* */")
    w("    uint8_t gp, gop;                         /* the goal: a parameter, 0 =, 1 >=, 2 <= */")
    w("    int16_t gv, gtol;")
    w("    uint8_t nset;")
    w("    uint8_t sp[16];                          /* what the page sets */")
    w("    int16_t sv[16];")
    w("    const char *text[NLANGS];")
    w("} spage_t;")
    w("typedef struct {")
    w("    const char *title[NLANGS];")
    w("    uint8_t module, demo, keep, first, n;    /* keep: the patch carries over */")
    w("    int16_t patch[NPAR];")
    w("} schap_t;")
    pages = []
    chaps = []
    for c in CHAPTERS:
        patch = dict(INIT)
        patch.update(c["patch"])
        first = len(pages)
        goals = {}
        for (kind, diag, knobs, goal, text, st) in c["pages"]:
            sets = dict(goals) if c["build"] else {}
            sets.update(st)
            assert len(sets) <= 16, c["title"]
            for k in list(knobs) + list(sets):
                assert k in PARAMS or k in VIRTUAL, k
            if goal:
                assert goal[0] in PARAMS, goal
                if c["build"]:
                    goals[goal[0]] = goal[2]
                    goals.update(st)
            elif c["build"]:
                goals.update(st)
            pages.append((kind, diag, knobs, goal, text, sets))
        chaps.append((c, patch, first, len(pages) - first))
    w("#define NSPAGES %d" % len(pages))
    w("static const spage_t SPAGES[NSPAGES] = {")
    for (kind, diag, knobs, goal, text, sets) in pages:
        kn = [pid(k) for k in knobs] + ["0"] * (4 - len(knobs))
        g = goal or ("ENGINE", "eq", 0, 0)
        sp = [pid(k) for k in sets] + ["0"] * (16 - len(sets))
        sv = [str(v) for v in sets.values()] + ["0"] * (16 - len(sets))
        w("    {%d, SD_%s, %d, {%s}, %s, %d, %d, %d, %d, {%s}, {%s}," % (
            KINDS[kind], diag, len(knobs), ", ".join(kn), pid(g[0]), OPS[g[1]], g[2], g[3], len(sets),
            ", ".join(sp), ", ".join(sv)))
        w("     " + langs(text) + "},")
    w("};")
    w("#define NSCHAP %d" % len(chaps))
    w("static const schap_t SCHAPS[NSCHAP] = {")
    for (c, patch, first, n) in chaps:
        w("    {%s, %d, SDEMO_%s, %d, %d, %d," % (langs(c["title"]), c["module"], c["demo"].upper(), int(c["keep"]),
                                                 first, n))
        w("     {" + ", ".join(str(patch[p]) for p in PARAMS) + "}},")
    w("};")
    open(os.path.join(APP, "synth_lessons.h"), "w").write("\n".join(out) + "\n")

    # the host test: every page of every chapter in every language (texts that do not fit fail the run), and
    # every goal reached by the host's "synsolve"
    tl = ["# GENERATED by app/tools/gen_synth.py: every SYNTH page in every language; every goal reached",
          "wait 600"]
    for lang in range(LANGS):
        tl.append("lang %d" % lang)
        for ci, (c, patch, first, n) in enumerate(chaps):
            tl.append("synch %d" % ci)
            tl.append("wait 20")
            for pi in range(n):
                tl.append("synpage %d" % pi)
                tl.append("wait 20")
                if pages[first + pi][0] == "S" and lang == 0:
                    tl.append("expect synok 0")
                    tl.append("synsolve")
                    tl.append("wait 20")
                    tl.append("expect synok 1")
    tl.append("lang 0")
    open(os.path.join(APP, "tests", "synth.txt"), "w").write("\n".join(tl) + "\n")
    print("synth_lessons.h: %d chapters, %d pages" % (len(chaps), len(pages)))


if __name__ == "__main__":
    main()
