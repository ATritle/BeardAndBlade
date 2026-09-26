# Audio controls artwork

Generated with the built-in image-generation tool. Source: `ArtSource/AudioControlsAtlas.png`. Runtime assets: `Content/Art/V2/AudioMusic`, `AudioEffects`, `AudioRail`, `AudioThumb`. Transparent atlas quadrants are mechanically cropped with `prepare_audio_controls.ps1`, then imported with `import_audio_controls.py`.

## Final generation prompt

Use case: stylized-concept. Asset type: single production game UI sprite atlas, 1024x1024 transparent PNG. Detailed crisp pixel art matching a dark fantasy dungeon RPG with ornate aged gold filigree, dark iron recesses and emerald gemstones. No rectangular panel, no text, no letters, no scene or background. Exactly four separate sprites in a regular 2x2 atlas each within its own 512x512 quadrant with generous transparent padding: top left an ornate golden musical double-note icon (music toggle); top right an ornate golden small loudspeaker with sound waves (SFX toggle); bottom left a long thin horizontal gold filigree slider rail with dark inset empty groove, centered vertically and occupying most quadrant width, no knob; bottom right a single small emerald diamond slider thumb mounted in gold. Keep silhouettes clean and legible at small display sizes. Actual transparent alpha outside each sprite. No shadows outside silhouettes. All four sprites fully contained and separated.

The generated atlas is 1254 square; extraction uses relative quadrants rather than assuming the requested resolution.
