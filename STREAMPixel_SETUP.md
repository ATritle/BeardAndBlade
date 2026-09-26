# StreamPixel setup

StreamPixel hosts Unreal Pixel Streaming on a remote GPU and sends video, audio, and player input through the browser. It is separate from GitHub download hosting.

Both v0.3.2 archives use the same Pixel Streaming-enabled Shipping build under `Builds/v0.3.2/Windows`.

Upload `Builds/TheBeardAndBlade-StreamPixel-v0.3.2.zip`. Its outer folder is `Windows`, containing the launcher and all runtime content. Use this upload archive rather than the GitHub source-code ZIP or offline-layout ZIP.

Build/rebuild procedure:

1. In Unreal Editor, enable **Pixel Streaming** in Edit > Plugins and restart. StreamPixel supports Pixel Streaming and Pixel Streaming 2, recommends the original plugin, and requires only one to be enabled. Confirm the hosting configuration supports UE 5.8.
2. Package a fresh Windows Shipping build with the plugin enabled. ZIP the complete packaged Windows folder, including the executable and all content, not the source project or just the EXE.
3. Open the existing **The Beard And Blade** project in StreamPixel. Under **Builds > Upload New Build**, upload this ZIP. Disable automatic release if you want to review the build before making it live. Wait for processing and approval.
4. Test movement, mouse aiming, all three mouse buttons, inventory dragging, audio, boss intros, endings, and restart. Review the project's GPU/session capacity and hosting charges before enabling paid capacity. Desktop keyboard/mouse is the current control target; mobile needs a separate touch-control design.
5. Once approved and active, copy **Sharing > Share Link**. Share that browser-play URL on Discord. Keep the GitHub release link for downloadable offline Windows play.

Official references:

- https://docs.streampixel.io/resources/quick-start-guide/prepare-your-unreal-engine-project-for-windows
- https://docs.streampixel.io/resources/quick-start-guide/uploading-your-build
- https://docs.streampixel.io/resources/quick-start-guide/sharing-and-embedding

The user uploads and activates this build. No StreamPixel deployment or paid hosting changes are performed by the packaging process.

## Reproduce the streaming package

With PixelStreaming enabled in the project (not PixelStreaming2), close Unreal Editor and run:

```powershell
./Tools/package_windows.ps1 -Destination "$PWD/Builds/v0.3.2"
python Tools/zip_release.py --version v0.3.2 --streaming
```

These commands refuse to overwrite existing archives; choose a fresh destination/version for a rebuild.

No signalling URL, token or hosting credentials are baked into the game. StreamPixel manages the signalling connection and launch configuration. If its dashboard asks for the executable, select `Windows/TheBeardAndBlade.exe` relative to the ZIP. Confirm UE 5.8 support with StreamPixel if its engine selector does not list that version. Browser end-to-end validation must be performed after upload; local executable smoke tests do not verify the hosted WebRTC connection.
