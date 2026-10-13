# Shardfall: Realm of the Lost Crystals

**Shardfall** — a single-binary 2D action-adventure game written in modern C++ with [SFML 3](https://www.sfml-dev.org/).

Build a hero, travel between two procedurally detailed realms through animated teleport pods, harvest lost crystals, and defeat the Shadow Lord to achieve victory.

---

## Download & Play (Windows)

1. Go to the Releases page of this repository and download the latest Shardfall ZIP file.
2. Extract the ENTIRE ZIP to a normal folder (for example your Desktop or Documents). Do not run the game from inside the ZIP, and do not extract into Program Files.
3. Double-click `shardfall.exe` to play.

Requirements: Windows 64-bit.

Note: Windows SmartScreen may show a warning because the game is not digitally signed. Click "More info" and then "Run anyway".

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Controls](#controls)
- [Gameplay Loop](#gameplay-loop)
- [Build Instructions](#build-instructions)
- [Running the Game](#running-the-game)
- [Project Structure](#project-structure)
- [Technical Architecture](#technical-architecture)
- [Save Data](#save-data)
- [Credits](#credits)

---

## Overview

Shardfall is a top-down action game focused on tight moment-to-moment combat and exploration across two distinct environments. The entire game — rendering, physics, audio synthesis, persistence, and UI — is implemented in a single translation unit (`main.cpp`), with no external asset files. All sound is generated procedurally at runtime, and all visuals are drawn from procedural vector geometry.

| Property        | Value                                    |
| --------------- | ---------------------------------------- |
| Genre           | 2D Action / Adventure                    |
| Language        | C++17                                     |
| Framework       | SFML 3                                    |
| Window          | 1100 x 750, windowed, 8x anti-aliasing   |
| Frame rate      | Capped at 60 FPS                          |
| Source files    | 1 (`main.cpp`, ~4100 lines)              |
| External assets | None (audio synthesized, art procedural) |

---

## Features

### Player
- Knight hero composed of procedurally drawn parts (legs, torso, helmet, lantern) with directional facing.
- **Animated walk cycle** — legs alternate in a sine-driven step while moving.
- **Carried lantern** that orbits the hero based on facing direction.
- **Sword attack** with a slash-arc VFX, a forward cone hit test (dot-product facing check), knockback, and a cooldown.
- **Dash roll** with invulnerability frames (i-frames), a cooldown meter, and randomized ghost-trail afterimages.
- **Auto-healing** — regenerates `+12 HP` every `3.5 s`, in addition to crystal pickups.
- Health system with three-tier damage colouring (green > 60, amber > 30, red otherwise).

### Combat
- Three enemy archetypes:
  - **Crawler / Crag Crawler** — fast, low-health melee pursuer with no wind-up.
  - **Shadow Demon** — slower, heavier hitter with a telegraphed wind-up.
  - **Shadow Lord (Boss)** — large health pool, an on-screen health bar, and a telegraphed ground-slam that emits an expanding damage ring (shockwave).
- **Wind-up telegraphs** — Demons and the Boss flash white, freeze position, then strike.
- **Boss slam warning** — a pulsing red ground ring plus an audible warning tone precede each slam.
- Directional hit confirmation using a dot-product facing test, with per-target hit timers.
- **Floating damage numbers** pop up over the player whenever damage is taken.
- **Screen shake** on every impactful event (hits, dashes, slams, teleports, enemy deaths).
- Damage-taken history for the last 10 seconds (surfaced in the debug overlay).

### Realms
- **Mystic Forest Valley** — grass patches, a flowing curved river with animated shimmer, a winding dirt path, trees, mossy boulders, and drifting firefly motes. Green/cyan crystal palette.
- **Craggy Mountain Peaks** — layered 3D-shaded mountain ridges, a sky horizon fog band, a bottomless canyon gorge with a glowing magma vein, wooden/stone bridges, rocky cliff steppes, jagged crag obstacles, and blowing snow/wind motes. Purple/orange-red crystal palette.
- Distinct realm-specific ambient tinting (vignette) and weather behaviour per realm.
- Physical collision with trees, boulders, and crags, plus a mountain-top boundary that prevents walking off the peaks.

### Teleport Pods (Marvel-style Gateway)
- Animated gateway built from persistent, reused geometry (allocation-free per frame).
- Layered elliptical glow, rotating vortex energy disc, three counter-rotating ring arcs, a white-hot rim, a pulsing core, and an orbiting rune gem.
- Rim sparks emitted with gravity and drag for a cinematic energy effect.
- Opening animation and burst shock ring on each teleport, with a re-entry cooldown.

### Audio (100% Procedural)
- All sound effects are synthesized at startup — no audio files are shipped.
- Includes: sword swing whoosh, hit crunch, crystal chime, dash swoosh, boss slam rumble, boss slam warning ping, and teleport warp.
- An ambient drone BGM buffer is generated but currently disabled (kept for optional use).

### Progression & Persistence
- Collect all 4 crystals **and** defeat every enemy to trigger victory.
- Persistent **high score** stored to disk (`highscore.txt`).
- Append-only **match history** recording hero name, score, crystals, realm, and outcome (`history.txt`).
- **Hall of Heroes** dashboard with four aggregate stat badges, a battle-log table of the most recent runs, and victory/fallen outcome badges.

### UI / UX
- **Glassmorphic login screen** with hero name entry (14-character limit), a live blinking text cursor, a placeholder prompt, and hover-reactive buttons.
- Hero avatar emblem and high-score record display on the login screen.
- **In-game HUD** — health bar with numeric readout and heal indicator, dash charge meter with ready/dashing status, realm badge, hero profile badge, and a score / high-score / crystal tracker card.
- On-screen controls prompt bar along the bottom.
- **Victory / defeat modals** with contextual actions (restart, dashboard, logout) and a "NEW RECORD" flag.
- **Debug overlay** (toggled with `F3`) showing smoothed FPS, player HP, dash cooldown, and damage taken in the last 10 seconds.

### Presentation
- **Y-sorted rendering** so entities correctly overlap by depth.
- Particle system for impacts, pickups, healing, and ambient effects with lifetime fade-out.
- FX sparks, dash ghost trails, boss shockwave rings, and radial lighting around the player.
- Anti-aliased rendering at 8x with additive blending for all energy effects.
- Allocation-aware effects — fixed-size pools are used for floating numbers (64) and damage history (64), and the portal buffers are mutated in place.

---

## Controls

| Action              | Keys                                        |
| ------------------- | ------------------------------------------- |
| Move                | `W` `A` `S` `D` or Arrow Keys               |
| Attack (sword)      | `Space`                                     |
| Dash / Roll (i-frames) | `Left Shift`, `Right Shift`, or `Left Ctrl` |
| Enter Realm / Confirm | `Enter` (login screen)                    |
| Hall of Heroes      | `H`                                         |
| Toggle Debug Overlay | `F3`                                       |
| Pause Menu          | `Esc` or `P` (during play)                  |
| Restart (after end) | `R`                                         |
| Logout to login     | `L` (after end)                             |
| Close / Back        | `Esc` on screen, exit when `Esc`           |

> Tip: Dash grants brief invulnerability — use it to phase through boss shockwaves.

---

## Gameplay Loop

1. Enter your hero name on the login screen and select **Enter Realm**.
2. Explore the current realm, collecting crystals and fighting enemies.
3. Step into the teleport pod to travel to the other realm.
4. Defeat **all enemies** and collect **all 4 crystals** across both realms to win.
5. Results are saved to the high-score and match-history files.
6. Review past runs in the **Hall of Heroes**, then restart or log out.

---

## Build Instructions

### Prerequisites
- A C++17-capable compiler (tested with MinGW-w64 GCC).
- SFML 3 development libraries (headers + linkable libraries).

### Compile

From the `Shardfall` directory:

```powershell
C:\mingw142\bin\g++.exe main.cpp -o shardfall.exe -std=c++17 `
  -IC:\SFML\include -LC:\SFML\lib `
  -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system
```

Adjust the compiler and SFML paths to match your installation.

### Notes
- On Windows, the required SFML runtime DLLs must sit next to `shardfall.exe` (they are bundled in this repository).
- The `-d` variants of the DLLs (e.g. `sfml-graphics-d-3.dll`) are the debug builds; the release executable uses the non-`-d` DLLs.

---

## Running the Game

Double-click `run.bat` in the project root, or run directly:

```powershell
D:\Game\Shardfall\shardfall.exe
```

The launcher `run.bat` changes into the game directory and starts the executable:

```bat
@echo off
cd /d "%~dp0Shardfall"
start "" "shardfall.exe"
```

---

## Project Structure

```
D:\Game\
|-- README.md                  This document
|-- run.bat                    Convenience launcher
|-- Shardfall\
    |-- main.cpp               Complete game source
    |-- shardfall.exe          Compiled executable
    |-- run.bat                Directory-local launcher
    |-- highscore.txt          Persisted best score
    |-- history.txt            Persisted match history
    |-- settings.txt           Persisted player settings
    |-- hints_seen.txt         First-run hint flags per hero
    |-- sfml-*-3.dll           SFML runtime (release)
    |-- sfml-*-d-3.dll         SFML runtime (debug)
    |-- libgcc_s_seh-1.dll     GCC runtime
    |-- libstdc++-6.dll        C++ standard library runtime
    |-- libwinpthread-1.dll    POSIX threads runtime
```

---

## Technical Architecture

The game is organized as a single-file program with clearly separated systems.

### Core Systems

| System             | Responsibility                                                                 |
| ------------------ | ------------------------------------------------------------------------------ |
| Game loop          | SFML event pump + fixed-timestep update + render pipeline                       |
| State machine      | `LoginScreen` and `Playing` states driven by `GameState`                        |
| Entity model       | Plain structs (`Enemy`, `Crystal`, `Tree`, `Boulder`, `MountainPeak`, ...)      |
| Rendering          | Collects draw callbacks into a `Renderable` list and Y-sorts before drawing     |
| Teleport pod       | `TeleportPod` with persistent `sf::VertexArray` geometry, reused each frame      |
| Particles / FX     | `Particle`, `GhostTrail`, `Shockwave`, `PortalSpark` pools with lifetime decay  |
| Audio              | `generateProceduralAudio` synthesizes all buffers at startup                    |
| Persistence        | Plain-text `highscore.txt` and tab-delimited `history.txt`                      |

### Design Highlights
- **Zero external assets** — audio via waveform synthesis, art via SFML primitives (`ConvexShape`, `VertexArray`, `CircleShape`).
- **Allocation-aware effects** — the teleport pod and vortex buffers are built once and mutated in place; per-frame allocations are avoided in hot paths.
- **Depth-correct compositing** — every world object provides a sort key so foreground and background elements overlap naturally.
- **Additive-blended energy** — glows, arcs, and sparks use additive blending for a luminous, cinematic look.

### Key Constants
- Player base speed: `250 px/s`; dash speed: `680 px/s`.
- Dash details: `1.2 s` cooldown, `0.22 s` dash duration, `0.12 s` i-frames.
- Auto-heal: `+12 HP` every `3.5 s`.
- Attack cooldown: `0.35 s`; attack window: `0.22 s`.
- Enemy wind-up: Demon `0.3 s`, Boss melee `0.4 s`.
- Boss slam interval: `3.4 s`; slam telegraph: `0.7 s`; shockwave damage: `16`.
- Boss melee damage: `16`.
- Crystal pickup: `+25` score and `+20 HP`.
- Score rewards: Crawler `20`, Demon `40`, Boss `150`.

---

## Save Data

| File            | Format                                                        | Purpose                          |
| --------------- | ------------------------------------------------------------- | -------------------------------- |
| `highscore.txt` | Single integer                                                | Best score across all runs       |
| `history.txt`   | `name<TAB>score<TAB>crystals<TAB>realm<TAB>outcome` per line  | Full match history for the archive |
| `settings.txt`  | `key=value` lines (`volume`, `screenShake`, `showHints`)       | Player-adjustable settings        |
| `hints_seen.txt`| One hero name per line                                          | First-run hint flags per hero     |

All files are created automatically on first run and stored in the game directory. Existing saves carry over unchanged after a rename.

---

## Credits

- **Engine / Framework:** SFML 3
- **Audio:** Procedurally synthesized at runtime
- **Art:** Procedurally drawn from vector primitives

Developed as a self-contained demonstration of modern C++ game systems.

---

## Quick Build & Run

Run these two commands from the game folder in PowerShell:

```powershell
C:\mingw142\bin\g++.exe main.cpp -o shardfall.exe -std=c++17 -IC:\SFML\include -LC:\SFML\lib -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system
.\shardfall.exe
```