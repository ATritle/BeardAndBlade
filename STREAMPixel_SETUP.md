# StreamPixel setup

StreamPixel hosts Unreal Pixel Streaming on a remote GPU and sends video, audio, and player input through the browser. It is separate from GitHub download hosting.

The v0.3.1 Windows download is not yet a streaming build. Prepare a separate package before uploading:

1. In Unreal Editor, enable **Pixel Streaming** in Edit > Plugins and restart. StreamPixel supports Pixel Streaming and Pixel Streaming 2, recommends the original plugin, and requires only one to be enabled. Confirm the hosting configuration supports UE 5.8.
2. Package a fresh Windows Shipping build with the plugin enabled. ZIP the complete packaged Windows folder, including the executable and all content, not the source project or just the EXE.
3. Open the existing **The Beard And Blade** project in StreamPixel. Under **Builds > Upload New Build**, upload this ZIP. Disable automatic release if you want to review the build before making it live. Wait for processing and approval.
4. Test movement, mouse aiming, all three mouse buttons, inventory dragging, audio, boss intros, endings, and restart. Review the project's GPU/session capacity and hosting charges before enabling paid capacity. Desktop keyboard/mouse is the current control target; mobile needs a separate touch-control design.
5. Once approved and active, copy **Sharing > Share Link**. Share that browser-play URL on Discord. Keep the GitHub release link for downloadable offline Windows play.

Official references:

- https://docs.streampixel.io/resources/quick-start-guide/prepare-your-unreal-engine-project-for-windows
- https://docs.streampixel.io/resources/quick-start-guide/uploading-your-build
- https://docs.streampixel.io/resources/quick-start-guide/sharing-and-embedding

No StreamPixel deployment or paid hosting changes were made as part of the Windows v0.3.1 release.
