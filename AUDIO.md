# Audio pass

Three original synthesized loops: menu (76 BPM), dungeon (68 BPM), boss (104 BPM). Sixteen synthesized cues cover sword swings, impacts, rolls, throws, tea splashes, explosions, paper, magic, footsteps, hurt/death, chest rewards, equipment, portals, UI and spawning. Enemy attacks share cue families rather than unique recordings per species.

Music crossfades when entering/leaving the menu or a boss room. M toggles music; N toggles effects. These preferences persist locally. Music continues quietly while the inventory is open. Per-cue rate limiting avoids multiplying the volume when an AoE hits many targets simultaneously.

`Tools/generate_audio.py` reproducibly generates 32 kHz stereo 16-bit WAV files using NumPy; `Tools/import_audio.py` imports them as Unreal SoundWave assets. No external samples, recordings or copyrighted music were used. The synthesis source is included; WAV intermediates are excluded from Git because the imported assets are included.

This is a first synthesized audio pass, not an orchestral recording or human-performed Foley set. Listen in the packaged game to evaluate mix/loop fatigue on your own speakers.
