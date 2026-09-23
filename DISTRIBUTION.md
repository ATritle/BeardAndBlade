# Playing outside Unreal Editor

## Windows players

Download the Windows game ZIP from this repository's Releases page when available. Extract the entire archive, then launch `TheBeardAndBlade/TheBeardAndBlade.exe` in newly packaged downloads. The existing v0.2.1 download uses an outer `Windows` folder; rename that outer folder to `TheBeardAndBlade` to use the new path. Keep the Engine and TheBeardAndBlade subfolders alongside the launcher. Unreal Editor is not required. If Windows reports missing Visual C++ runtime components, install the bundled Epic prerequisite installer under `Engine/Extras/Redist/en-us`.

The build is unsigned. Windows may show a publisher/reputation warning. Only run a release obtained from a source you trust; do not disable Windows security globally. Windows x64 is the current target. macOS, Linux, phones and browsers are not included.

## Developers

Clone the repository and open `TheBeardAndBlade.uproject` in UE 5.8. Install the Visual Studio C++ game-development tools and Windows SDK. All imported runtime art and audio are checked in; the engine, build caches and private reference/source-generation images are not.

Run `Tools/package_windows.ps1` to build, cook, stage and archive a Shipping Windows build. Run `python Tools/zip_release.py` to ZIP the entire staged Windows directory under the player-facing `TheBeardAndBlade` folder. Unreal's staging directory remains `Windows`; all internal game and Engine paths are preserved. Do not distribute the editor DLL, the executable in Binaries alone, or GitHub's automatic Source code ZIP as the playable download.

Upload the player ZIP as a GitHub Release asset, separately from the project source. Use a prerelease for this playtest version. Keep release notes, controls and known limitations alongside it. No paid server is needed for a downloaded single-player build.

## Browser play

This UE 5.8 project is not a static HTML/WebAssembly game and cannot be played simply by enabling GitHub Pages. Epic's supported Pixel Streaming workflow runs the packaged application on a GPU-equipped computer/server and streams video/audio and controls through WebRTC. That requires additional hosting, signalling, access/security and capacity planning. A native browser-engine port is a separate development project.

References: https://dev.epicgames.com/documentation/unreal-engine/packaging-your-project ; https://dev.epicgames.com/documentation/unreal-engine/pixel-streaming-in-unreal-engine ; https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases

## Current limitations

Early single-player playtest. No save system or campaign ending; four themes repeat. No code-signing, installer, automatic updater or online multiplayer. Source and generated assets have not been assigned an open-source license; public hosting alone does not grant broad reuse rights. Unreal Engine components remain subject to Epic's terms.
