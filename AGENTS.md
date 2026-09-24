# Agent Instructions — Unreal Movement Sample

Unreal-Movement is a Meta Quest Pro sample that exposes Body, Eye, and Face Tracking through OpenXR's tracking APIs, intended as a starting point for driving expressive custom avatars in VR. See the official [Movement Sample docs](https://developer.oculus.com/documentation/unreal/unreal-movement-sample/).

## Source-of-truth files (read these first, do not duplicate their contents in this file)

For setup, build steps, SDK versions, and project layout, read:

- `README.md` — official setup, editor paths (Epic Games Launcher with MetaXR plugin, or Oculus-VR fork)
- `MovementSample.uproject` — engine association and enabled plugins (`OculusXR`, `OculusXRMovement`, `RigLogic`, `HairStrands`, `LiveLink`, `LiveLinkControlRig`, and testing plugins)
- `CHANGELOG.md` — release / migration notes
- `Plugins/OculusXRMovement/` — the bundled movement plugin (read its own docs if shipped)
- `LICENSE` — license terms

## Quest / Horizon-specific notes

- Eye and Face Tracking require **Meta Quest Pro** sensors. On Quest 3 / 3S those features silently no-op. Verify on a Quest Pro headset before chasing "missing animation" bugs.
- The `OculusXRMovement` plugin is **bundled** under `Plugins/` rather than installed from a marketplace. Do not replace the bundled copy with an external version without matching plugin and engine versions.
- The Oculus Unreal fork path is documented for the latest Movement integration. The marketplace MetaXR plugin works for most flows but may lag behind on bleeding-edge tracking features.
- The default character setup wires LiveLink + Control Rig + RigLogic together. Disabling any of those plugins will break the high-fidelity and retarget maps.
- The project ships a full testing-plugin stack (`FunctionalTestingEditor`, `RuntimeTests`, `Gauntlet`, etc.). Preserve these if you intend to run the project's automated tests.

# Meta Quest tooling

This is a Meta Quest / Horizon OS sample. The bespoke intro above is the source of truth for what this project is and how it's built — use it (and the files it points at) instead of restating facts from memory.

When the user asks anything about Quest device behavior, build / deploy / debug / capture flows, on-device performance, or Horizon OS APIs, reach for these tools instead of generic Unreal answers:

- **`hzdb`** — Quest-aware ADB wrapper (device list, install / launch / stop, logs, screenshots, Perfetto traces, on-device docs search). Already wired up as an MCP server via `.mcp.json`, `.vscode/mcp.json`, and `.cursor/mcp.json`. Also runnable directly: `npx -y @meta-quest/hzdb <subcommand>`.
- **Meta Quest Agentic Tools** — the full skill set, including Unreal-specific skills: <https://github.com/meta-quest/agentic-tools>. Install per your client (Claude Code: `/plugin install meta-vr@meta-quest`; Gemini CLI: `gemini extensions install https://github.com/meta-quest/agentic-tools`; Cursor / VS Code: install the **Meta Horizon** extension from the Marketplace).

A few behavior expectations:

- **Read this repo's files first.** Before answering anything project-specific, read `README.md` and whichever source-of-truth files the intro above points at. Don't restate their contents in chat — quote or link instead.
- **Use `hzdb` for device-side work.** Anything that touches an attached Quest (install, launch, logs, screenshot, capture, manifest inspection) goes through `hzdb`, not raw `adb`.
- **Check live Horizon OS docs before answering API questions.** `hzdb docs search "..."` queries the live docs; training data on Horizon OS APIs goes stale fast.
- **Don't fabricate SDK / engine versions.** If a version isn't visible in this repo's files, say so rather than guessing.
