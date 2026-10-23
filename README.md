# Nebula Pad

Atmosphärischer Pad-Synthesizer als VST3 für Windows (plus Standalone-Version).

## Was drin ist

Drei Oszillatoren mit je 16 eingebauten Klangquellen, die sich per Morph-Regler durch mehrere Klangstufen bewegen:

- Wavetables: Saw > Square, Pulse Sweep, Formant Vox, Glass Bell, Organ, Spectral Comb, Hollow Sync, Digital Grit
- Texturen: Choir Air, Strings Ensemble, Shimmer Octaves, Dark Drone, Glass Pad, Bowed Metal, Vowel Morph, Spectral Cloud

Pro Oszillator: Level, Morph, Unison (bis 7 Stimmen), Detune, Oktave, Halbton, Feinstimmung.
Dazu: Multimode-Filter mit Drive und Rauschen, zwei Hüllkurven, zwei LFOs, "Chaos" (zufällige Drift von Tonhöhe, Morph und Filter), Stereobreite, Chorus, Ping-Pong-Delay, Hall und 10 Werks-Presets. 12-stimmig polyphon, Bildschirmtastatur zum Antesten.

## Plugin bauen lassen (ohne eigene Entwicklungsumgebung)

1. Kostenloses Konto auf github.com anlegen.
2. Oben rechts auf "+" > "New repository", Namen vergeben (z. B. `nebula-pad`), auf "Create repository" klicken.
3. Auf der Seite "uploading an existing file" anklicken und den **Inhalt** dieses Ordners hineinziehen (also `CMakeLists.txt`, `README.md`, den Ordner `Source` und den Ordner `.github`). Unten auf "Commit changes" klicken.
   - Falls der Ordner `.github` beim Hochladen fehlt: "Add file" > "Create new file", als Namen `.github/workflows/build.yml` eintippen und den Inhalt der Datei `build.yml` hineinkopieren.
4. Oben auf den Reiter "Actions" klicken. Der Build startet automatisch und dauert beim ersten Mal etwa 10–15 Minuten.
5. Wenn ein grüner Haken erscheint: auf den Durchlauf klicken, ganz unten unter "Artifacts" `NebulaPad-Windows` herunterladen und entpacken.
6. Den Ordner `Nebula Pad.vst3` nach `C:\Program Files\Common Files\VST3` kopieren.
7. In der DAW die Plugins neu scannen lassen. Nebula Pad erscheint als Instrument.

Die Standalone-Version (`Nebula Pad.exe`) liegt ebenfalls im Download und läuft ohne DAW.

## Wenn der Build rot wird

Auf den fehlgeschlagenen Durchlauf klicken, den aufgeklappten Schritt mit dem roten X öffnen, die Fehlermeldung kopieren und an Claude schicken.
