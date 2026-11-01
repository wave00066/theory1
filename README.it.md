# THEORY-1 — teoria musicale sull'M-VAVE FM-1

*[English version](README.md)*

THEORY-1 è un firmware didattico per l'M-VAVE FM-1. Insegna la teoria musicale a ragazzi e principianti, usando i
tasti, le luci e il display del synth. È disponibile in **italiano, inglese, spagnolo e tedesco**. Le note sono
scritte come **Do (C)**: nome italiano con la lettera inglese/americana accanto, oppure solo Do Re Mi, solo C D E o
alla tedesca (C D E … H, Fis, Es).

- **LEZIONI**: le basi, gli intervalli, 9 scale, i 7 modi, 13 tipi di accordo, gli accordi costruiti su ogni grado
  (triadi e settime, con numeri romani e funzione armonica) e il circolo delle quinte con tutte le 15 tonalità.
  Tutto parte da qualsiasi tonica ed è scritto correttamente (Fa maggiore ha il Sib, non il La#).
- **QUIZ**: 9 tipi di domande, da 10 domande ciascuna, su 3 livelli. Si risponde **suonando sui tasti** (scale,
  accordi, intervalli, note, gradi). Poi ci sono domande **a orecchio** (intervalli e accordi) e sulle tonalità.
  Ogni risposta riceve un feedback, con aiuti, punteggio e record salvati.
- **LABORATORIO**: suona quello che vuoi e THEORY-1 ti dice che nota, intervallo o accordo è, incluso il rivolto.
  Se scegli una scala, ti mostra i gradi e segnala in rosso le note fuori scala.

I tasti dell'FM-1 si illuminano con le note della lezione: arancio la tonica, poi le note della scala, il verde
per le risposte giuste. In basso sul display c'è una tastiera con gli stessi colori. Funziona anche con una
tastiera MIDI esterna via USB, il jack TRS o il **Bluetooth** (per esempio un piano digitale con Bluetooth MIDI).

| | | | |
|---|---|---|---|
| ![menu](docs/img/01-menu.png) | ![scala](docs/img/05-scala.png) | ![modo](docs/img/08-modo.png) | ![armonia](docs/img/10-armonia.png) |
| ![tonalità](docs/img/12-tonalita.png) | ![quiz](docs/img/15-quiz-scala.png) | ![orecchio](docs/img/17-quiz-orecchio.png) | ![laboratorio](docs/img/19-lab-accordo.png) |
| ![English](docs/img/en-05-scala.png) | ![Español](docs/img/es-10-armonia.png) | ![Deutsch](docs/img/de-15-quiz-scala.png) | ![impostazioni](docs/img/22-impostazioni.png) |

## Lingua e nomi delle note

In **IMPOSTAZIONI** (o con GLO da qualsiasi schermata):

- **Lingua**: Italiano, English, Español, Deutsch. Tutto il testo cambia, lezioni comprese.
- **Nomi delle note**:
  - `Do (C)`: Do Re Mi con le lettere C D E accanto (predefinito). Nei riquadri la lettera compare sotto.
  - `Do Re Mi`: solo il nome italiano.
  - `C D E`: solo la lettera.
  - `C D E ... H`: alla tedesca, con H per il Si e B per il Sib, Fis/Es per le alterazioni.

Nelle lezioni e nel laboratorio **EDIT** passa al sistema di nomi successivo senza aprire le impostazioni. Le
sigle degli accordi (Cm7, G7, Bm7b5) sono sempre in lettere, come si scrivono sugli spartiti pop e jazz.

## Comandi

| Comando | Cosa fa |
|---|---|
| **SELECT** (encoder) | scorre le voci: pagina, scala, modo, accordo, risposta |
| **SEL** | entra, conferma, risponde, va alla domanda successiva |
| **HOME** | torna indietro di un livello |
| **PLAY** | ascolta l'esempio (nel quiz: la nota di riferimento o di nuovo il suono) |
| **K1** o **PRESETS** | la tonica. Nel menu quiz: il livello |
| **K2 / K3** | in Armonia: il grado / triadi o settime |
| **ALGORITHM** | nelle lezioni: passa all'argomento successivo |
| **ARP** | in Armonia: suona gli accordi di tutti i gradi |
| **EDIT** | nel quiz: aiuto (si accendono i tasti giusti). Nelle lezioni: il sistema di nomi delle note |
| **FX** | nel quiz: salta la domanda |
| **GLO** | impostazioni |
| **REC** | cerca e collega una tastiera o un piano Bluetooth MIDI (ALGORITHM sceglie, REC collega) |
| **OCT− / OCT+** | spostano di un'ottava il suono dei tasti |

I pulsanti utili nella schermata in cui sei restano accesi tenui. Nel quiz contano solo i nomi delle note, quindi
puoi rispondere in qualsiasi ottava. Un accordo puoi suonarlo tutto insieme o una nota alla volta.

## Punteggio del quiz

Una risposta giusta al primo colpo vale 10 punti. Ogni nota sbagliata toglie 2 punti e ogni aiuto ne toglie 3, ma
una risposta suonata completa vale sempre almeno 2 punti. Una domanda a scelta vale 10 punti se giusta e 0 se
sbagliata. Dopo 3 errori i tasti giusti si accendono da soli. A fine quiz vedi le stelle e il record.

## Installare

1. Prendi il file `theory1-X.Y.fwsc` dalla pagina *Releases* di questo repository, oppure dall'ultima esecuzione
   della workflow *Build and attest firmware* (artefatto `packages`).
2. Collega l'FM-1 al computer con un cavo USB **dati**, direttamente e senza hub.
3. Installa il file con l'installer web dell'[FM-1 Firmware Hub](https://thefirmwarehub.com/) in Chrome o Edge,
   oppure da terminale: `python3 base/tools/fm1_install.py theory1-X.Y.fwsc`.
4. Non staccare il cavo mentre scrive.

**Tornare al firmware ufficiale:** scarica `FM-1.fwsc` (V15) dalla pagina download di M-VAVE e installalo con
M-UPGRADE o con l'installer. È un firmware non ufficiale: lo installi a tuo rischio.

## Compilare

Il firmware si compila nel cloud con GitHub Actions, perché il compilatore del chip (JieLi) e il suo SDK si
scaricano lì:

- **Actions → Build and attest firmware → Run workflow**: produce `theory1.fwsc` come artefatto.
- **Un tag `vX.Y`** (`git tag v0.1 && git push --tags`): produce una release firmata con `theory1-0.1.fwsc`.

Sul computer, senza FM-1 né compilatore:

```sh
git clone --recursive https://github.com/wave00066/theory1
sh tests/run_tests.sh          # teoria, quiz, laboratorio, e ogni testo nelle 4 lingue
python3 base/tools/build.py app --host
app/build/host/theory1_host app/tests/screens.txt OUT   # le schermate come PNG
```

## Struttura

| File | Cosa contiene |
|---|---|
| `app/theory.c` | La teoria: grafia e nomi delle note (Do, C, H), intervalli, scale, modi, accordi, gradi, tonalità e riconoscimento degli accordi, con nomi e spiegazioni nelle 4 lingue. È C puro e si prova da solo. |
| `app/texts.c` | Tutti gli altri testi dello schermo nelle 4 lingue |
| `app/synth.c` | Il suono: un piccolo FM a 2 operatori e 8 voci (piano elettrico, organo, campane) |
| `app/app.c` | Le schermate, il disegno delle lettere accentate, le lezioni, il quiz, il laboratorio, le impostazioni, i tasti e il MIDI |
| `app/tests/` | Le prove: `theory_test.c`, `basic.txt` (tutti i quiz, il laboratorio, le impostazioni), `texts_fit.txt` (ogni pagina in ogni lingua entra nello schermo) e `screens.txt` |
| `base/` | [fm1-base](https://github.com/charlesvestal/fm1-base) (submodule) con [felucca-bsp](https://github.com/hugelton/felucca-bsp) |

I font dell'FM-1 contengono solo ASCII. Le lettere accentate (è, ñ, ü, ö) sono la lettera semplice con l'accento
disegnato sopra. La ß tedesca è scritta "ss".

## Licenza e crediti

MPL-2.0. THEORY-1 è costruito su fm1-base di Charles Vestal e su felucca-bsp di Leo Kuroshita (Hügelton
Instruments). Non è affiliato a M-VAVE. M-VAVE e FM-1 sono marchi dei rispettivi proprietari.
