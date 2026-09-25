# Selected main-menu music: The Forgotten Keep

Selected by Aaron on 2026-09-25.
Source file: kaazoom-the-forgotten-keep-night-version-full-version-572748.mp3
Artist/title supplied by user: Kaazoom - The Forgotten Keep (Night Version, Full Version).

## Status
Original MP3 uploaded unchanged. Conversion has NOT been performed: this work PC has no available FFmpeg or audio decoding library. No Unreal asset or game code has been changed. This upload does not change an existing packaged release.
Source licensing/attribution documentation was not supplied with the file; keep any download-page license or certificate with the project.

## Convert on home PC
With FFmpeg installed, run from this folder:
```powershell
ffmpeg -i "kaazoom-the-forgotten-keep-night-version-full-version-572748.mp3" -vn -c:a pcm_s16le -ar 48000 -ac 2 "MusicMenu.wav"
```
This creates a 16-bit PCM stereo WAV at 48 kHz for import. It preserves the full track without trimming, normalization, or invented loop edits. Conversion does not improve the original MP3 quality.
Listen to the full result, compare duration with the MP3, and check the beginning/end before import. A seamless musical loop has not been prepared or verified.

## UE5 integration
1. Pull latest main on the home PC; convert as above.
2. Inspect the existing /Game/Audio/MusicMenu asset and its type before changing it. Preserve the previous asset through version control.
3. Import the WAV as a Sound Wave. If MusicMenu is a Sound Wave, replace/reimport it with this WAV; if it is a Sound Cue, replace its referenced wave while preserving the cue's path and settings.
4. Keep the runtime reference /Game/Audio/MusicMenu.MusicMenu. DungeonAudio.cpp UpdateAudio() loads this path, fades in over 1.2 seconds at volume 0.23, and fades out over 0.7 seconds when switching tracks. ToggleMusic() also uses volume 0.23.
5. IMPORTANT: UpdateAudio() currently selects MusicMenu when bMenu OR HasEnding() is true. Replacing that asset also changes ending-screen music. User selected this for the MAIN MENU. Preserve the old ending music via a separate asset/routing if ending music is to stay unchanged; do not unintentionally replace it.
6. Enable looping in the appropriate Sound Wave or Sound Cue. Audition a complete wrap; if the start/end do not join naturally, prepare a reviewed loop or fade transition rather than calling it seamless.
7. Test initial menu, entering gameplay, reopening menu, mute/unmute, death/victory screens, and repeated menu transitions (no overlapping tracks).
8. Save and commit imported assets and any necessary routing change. Package a new Windows/Pixel Streaming build and test its audio. Uploading this source MP3 alone does not update StreamPixel.

## Home-PC task prompt
Import the selected Kaazoom track from AudioSource/Music/MainMenu/forgotten-keep-v1 into the UE5 main menu. Read this README first. Convert to 48 kHz stereo 16-bit PCM WAV if needed; preserve the original MP3. Inspect existing MusicMenu asset type and replace its playback source appropriately. Preserve ending-screen music separately because current routing shares MusicMenu with endings. Verify looping by listening, menu/game transitions, mute, and packaged playback. Commit the resulting assets and required code changes.
