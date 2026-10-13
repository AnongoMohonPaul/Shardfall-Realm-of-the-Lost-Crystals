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
- [Recent Updates / Changelog](#recent-updates--changelog)
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
| Source files    | 1 (`main.cpp`, ~6200 lines)              |
| External assets | None (audio synthesized, art procedural) |

---

## Recent Updates / Changelog

This section summarises the most recent changes to Shardfall. Only completed and
shipped features are listed.

### Home Menu & Navigation (new)
- A new **Home menu** is now the first screen. It offers **Play**, **Hall of Heroes**,
  **Settings**, **How to Play**, and **Quit**.
- Navigate with the keyboard (`Up`/`Down` or `W`/`S`, then `Enter`/`Space`) or the mouse
  (hover to highlight a button, left-click to confirm).
- The menu shows the current **best score** and the game **version**.
- **Play** opens the existing hero-name login, which is now pre-filled with the name from
  your last run. `Esc` on the login screen returns to the Home menu.
- After a run, the **Logout** action now returns to the Home menu (previously the login screen).
- **Settings** and **How to Play** currently open a "Coming soon" placeholder screen with a
  Back button (`Esc` also returns). These screens are not implemented yet.

### Giant Fireball Hazard (new)
- Added to **Craggy Mountain Peaks** only. At random intervals a rising warning tone and a
  pulsing ground marker telegraph a fireball, which then roars across the map.
- A fireball damages the player heavily, damages enemies lightly, and does not harm the boss.
  It explodes when it leaves the play area and can be dashed through for a perfect dodge.
- Events can occasionally launch **two** fireballs in a staggered pair.
- The hazard pauses during the boss's phase-change invulnerability window.
- Trails leave embers and drifting smoke, and the launch/impact shows an expanding shock ring
  (all drawn from fixed-size pools with no per-frame allocation).

### Enemy Huts (new)
- Added solid, hand-placed **huts** to both realms — wooden cottages in the Forest and stone
  huts in the Mountains.
- Huts block movement (for the player and enemies) and are drawn in correct depth order.

### Wizards & Mini Fireballs (new)
- Huts spawn robe-wearing **wizards** over time (each hut runs its own timer, with a cap on
  how many can be alive at once).
- Wizards keep their distance, face the player, and lob **mini fireballs** from range after a
  short, telegraphed wind-up (with a staff-tip flare and a faint aim line).
- Wizards are fragile (one or two sword hits) and give a small flat score with no combo.
- Mini fireballs **burst on obstacles** (huts, trees, boulders, crags) and can be dashed
  through for a perfect dodge. They are active in both realms.
- On victory, any living wizards fade out with a small smoke puff (no score).

### Character & Combat Animation (improved)
- The hero now has a proper **walk cycle**, an **idle breathing** motion, and a three-phase
  **sword-swing animation** (anticipation, strike, recovery) with a fading **sword trail**.
- Regular enemies (crawler, charger, demon) gained walk and idle animation, and wizards have
  their own emerge, hover, and fade visuals.
- These are visual-only changes and do not affect gameplay or physics.

### Audio (new)
- Added procedural sound effects for the giant fireball (warning, launch whoosh, impact boom)
  and for wizard mini fireballs (warning chirp, launch whoosh).
- As before, all audio is synthesized at startup with no audio files shipped.

### Other Improvements
- Added a generic `circleHitsObstacle` helper (no allocations) used to detect when projectiles
  strike world geometry.
- Refactored obstacle movement to account for huts alongside trees, boulders, and crags.
- The source (`main.cpp`) has grown to roughly 6,200 lines.

---

## Features

### Player
- Knight hero composed of procedurally drawn parts (legs, torso, helmet, lantern) with directional facing.
- **Animated walk cycle** — legs alternate in a sine-driven step while moving, with a subtle idle upper-body breathing motion while standing still.
- **Carried lantern** that orbits the hero based on facing direction.
- **Sword attack** with a slash-arc VFX, a forward cone hit test (dot-product facing check), knockback, and a cooldown. The swing animates through three phases (anticipation, strike, recovery) and leaves a fading sword trail.
- **Dash roll** with invulnerability frames (i-frames), a cooldown meter, and randomized ghost-trail afterimages.
- **Auto-healing** — regenerates `3 HP/s` starting `4 s` after last taking damage, in addition to crystal pickups.
- Health system with three-tier damage colouring (green > 60, amber > 30, red otherwise).

### Combat
- Five enemy archetypes:
  - **Crawler / Crag Crawler** — fast, low-health melee pursuer with no wind-up.
  - **Shadow Demon** — slower, heavier hitter with a telegraphed wind-up.
  - **Charger** (Mountain realm) — telegraphs a straight charge, then blitzes at high speed; hitting an obstacle ends the charge and stuns it.
  - **Wizard** — spawns from huts over time, keeps its distance, and lobs ranged mini fireballs; fragile and worth a small flat score.
  - **Shadow Lord (Boss)** — large health pool, an on-screen health bar, and a telegraphed ground-slam that emits an expanding damage ring (shockwave).
- **Ranged mini fireballs** — wizards charge briefly, then fire a small fireball that bursts on world obstacles and can be dashed through for a perfect dodge.
- **Boss phases** — three phases by HP (100–60%, 60–30%, below 30%): slams get faster each phase, a second wider shockwave ring is added from phase 2 onward, and the boss gains a `+20%` speed boost in phase 3. Each phase change triggers a brief invulnerable roar pause.
- **Wind-up telegraphs** — Demons and the Boss flash white, freeze position, then strike.
- **Boss slam warning** — a pulsing red ground ring plus an audible warning tone precede each slam.
- Directional hit confirmation using a dot-product facing test, with per-target hit timers.
- **Floating damage numbers** pop up over the player whenever damage is taken.
- **Screen shake** on every impactful event (hits, dashes, slams, teleports, enemy deaths).
- Damage-taken history for the last 10 seconds (surfaced in the debug overlay).

### Combat Feel
- **Hitstop** — short freeze-frames on impact (`0.06 s` on enemy hits, `0.08 s` on player hits) for weighty feedback.
- **Input buffering** — attack and dash presses are buffered for `0.15 s`, so they fire automatically the moment the cooldown ends.
- Dash i-frames (`0.12 s`) and post-damage regeneration (`4 s` delay) round out the responsive combat model.

### Realms
- **Mystic Forest Valley** — grass patches, a flowing curved river with animated shimmer, a winding dirt path, trees, mossy boulders, wooden cottages (huts), and drifting firefly motes. Green/cyan crystal palette.
- **Craggy Mountain Peaks** — layered 3D-shaded mountain ridges, a sky horizon fog band, a bottomless canyon gorge with a glowing magma vein, wooden/stone bridges, rocky cliff steppes, jagged crag obstacles, stone huts, and blowing snow/wind motes. Purple/orange-red crystal palette.
- **Giant fireball hazard** (Craggy Mountain Peaks only) — a warning tone and a pulsing path marker telegraph a fireball that then crosses the map, damaging the player and enemies.
- Distinct realm-specific ambient tinting (vignette) and weather behaviour per realm.
- Physical collision with trees, boulders, crags, and huts, plus a mountain-top boundary that prevents walking off the peaks.

### Teleport Pods (Arcane Gateway)
- Animated gateway built from persistent, reused geometry (allocation-free per frame).
- Layered elliptical glow, rotating vortex energy disc, three counter-rotating ring arcs, a white-hot rim, a pulsing core, and an orbiting rune gem.
- Rim sparks emitted with gravity and drag for a cinematic energy effect.
- Opening animation and burst shock ring on each teleport, with a re-entry cooldown.

### Audio (100% Procedural)
- All sound effects are synthesized at startup — no audio files are shipped.
- Includes: sword swing whoosh, hit crunch, crystal chime, dash swoosh, boss slam rumble, boss slam warning ping, teleport warp, giant-fireball warning / launch / impact, and wizard mini-fireball warning / launch.
- An ambient drone BGM buffer is generated but currently disabled (kept for optional use).

### Scoring
- Crystal pickup: `+25`.
- Enemy kills: Crawler/Charger `+20`, Shadow Demon `+40`, Wizard `+5`, Shadow Lord (Boss) `+150`.
- **Kill combo** — a kill within `3 s` of the previous one raises the multiplier (`x1.2`, `x1.4`, … up to `x2.0`); it resets after `3 s` without a kill or when taking damage.
- **Perfect dodge** — `+10` when dash i-frames absorb an attack.
- **Victory bonus** — `+300` for winning.
- **No-damage bonus** — up to `+500` for finishing untouched (`+250` under 50 damage taken, `+100` under 100).
- **Time bonus** — `+2` per second under `300 s` when you win.
- The high score is saved at the end of each run.

### Progression & Persistence
- Collect all 4 crystals **and** defeat every enemy to trigger victory.
- Persistent **high score** stored to disk (`highscore.txt`).
- Append-only **match history** recording hero name, score, crystals, realm, and outcome (`history.txt`).
- **Hall of Heroes** dashboard with four aggregate stat badges, a battle-log table of the most recent runs, and victory/fallen outcome badges.

### UI / UX
- **Home menu** — the first screen, offering Play, Hall of Heroes, Settings, How to Play, and Quit. Navigate by keyboard or mouse, and see the current best score and version.
- **Glassmorphic login screen** with hero name entry (14-character limit), a live blinking text cursor, a placeholder prompt, and hover-reactive buttons. The name is pre-filled from your last run.
- **Settings** and **How to Play** open a "Coming soon" placeholder screen with a Back button (not implemented yet).
- Hero avatar emblem and high-score record display on the login screen.
- **In-game HUD** — health bar with numeric readout and heal indicator, dash charge meter with ready/dashing status, realm badge, hero profile badge, and a score / high-score / crystal tracker card.
- On-screen controls prompt bar along the bottom.
- **First-run hints** — short on-screen prompts for Move, Attack, Dash, and Pause appear during the first `30 s` of a new hero's first run, each dismissing once the action is used.
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
| Home Menu: Navigate | `Up`/`Down` or `W`/`S`, then `Enter`/`Space`; or hover + left click |
| Enter Realm / Confirm | `Enter` (login screen)                    |
| Hall of Heroes      | `H`                                         |
| Toggle Debug Overlay | `F3`                                       |
| Pause Menu          | `Esc` or `P`: Pause menu (Resume, Volume, Screen Shake, Quit to Login) |
| Pause: Navigate     | `Up`/`Down` or `W`/`S` + `Enter`: navigate & confirm |
| Pause: Adjust       | `Left`/`Right`: change Volume / Screen Shake |
| Restart (after end) | `R`                                         |
| Logout to Home menu | `L` (after end)                             |
| Close / Back        | `Esc` on screen, exit from the Home menu   |

> Tip: Dash grants brief invulnerability — use it to phase through boss shockwaves.

---

## Gameplay Loop

1. From the **Home menu**, choose **Play** and enter your hero name on the login screen, then select **Enter Realm**.
2. Explore the current realm, collecting crystals and fighting enemies, and dodging the giant fireballs in the Mountains.
3. Step into the teleport pod to travel to the other realm.
4. Defeat **all enemies** and collect **all 4 crystals** across both realms to win.
5. Results are saved to the high-score and match-history files.
6. Review past runs in the **Hall of Heroes**, then restart or log out to the Home menu.

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
| Game loop          | SFML event pump + capped 60 FPS, delta-time update + render pipeline            |
| State machine      | `HomeMenu`, `LoginScreen`, `Playing`, `Settings`, and `HowToPlay` states driven by `GameState` |
| Entity model       | Plain structs (`Enemy`, `Wizard`, `Hut`, `Fireball`, `Crystal`, `Tree`, `Boulder`, `MountainPeak`, ...) |
| Rendering          | Collects draw callbacks into a `Renderable` list and Y-sorts before drawing     |
| Teleport pod       | `TeleportPod` with persistent `sf::VertexArray` geometry, reused each frame      |
| Particles / FX     | `Particle`, `GhostTrail`, `Shockwave`, `PortalSpark`, `FireParticle`, `ExplosionRing` pools with lifetime decay |
| Audio              | `generateProceduralAudio` synthesizes all buffers at startup                    |
| Persistence        | Plain-text `highscore.txt` and tab-delimited `history.txt`                      |

### Design Highlights
- **Zero external assets** — audio via waveform synthesis, art via SFML primitives (`ConvexShape`, `VertexArray`, `CircleShape`).
- **Allocation-aware effects** — the teleport pod and vortex buffers, along with the fixed-size fireball, ember/smoke, and wizard pools, are built once and reused; per-frame allocations are avoided in hot paths.
- **Depth-correct compositing** — every world object provides a sort key so foreground and background elements overlap naturally.
- **Additive-blended energy** — glows, arcs, and sparks use additive blending for a luminous, cinematic look.

### Key Constants
- Player base speed: `250 px/s`; dash speed: `680 px/s`.
- Dash details: `1.2 s` cooldown, `0.22 s` dash duration, `0.12 s` i-frames.
- Auto-heal: `3 HP/s` starting `4 s` after the last damage taken.
- Attack cooldown: `0.35 s`; attack window: `0.22 s`.
- Enemy wind-up: Demon `0.3 s`, Boss melee `0.4 s`.
- Boss slam interval: `3.4 s`; slam telegraph: `0.7 s`; shockwave damage: `16`.
- Boss melee damage: `16`.
- Crystal pickup: `+25` score and `+20 HP`.
- Score rewards: Crawler `20`, Demon `40`, Wizard `5`, Boss `150`.
- Giant fireball (Mountains): `480 px/s`, player damage `30`, warning `0.8 s`.
- Wizard mini fireball: `300 px/s`, player damage `8`, cooldown `2.5–4.0 s`, telegraph `0.5 s`.

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