# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Role

You are an expert in ESPHome, IoT, and embedded systems. Deep knowledge of ESPHome YAML config, component APIs, C++ lambdas, ESP-IDF/Arduino frameworks, sensor integration, display rendering, power management, and Home Assistant integration.

## Project Overview

Monorepo of ESPHome configs for several ESP32 projects, sharing `config/packages/base.package.yaml` for wifi, OTA, API, and logging.

- **einkframe** — mono 7.5" e-paper dashboard. [specs/einkframe/ARCHITECTURE.md](specs/einkframe/ARCHITECTURE.md)
- **photoframe** — color 7.3" e-paper photo frame. [specs/photoframe/ARCHITECTURE.md](specs/photoframe/ARCHITECTURE.md)
- **jukebox** — ESP32-C6 + RC522 NFC reader

## Commands

ESPHome runs via Docker through Make targets. All default to `CONFIG=einkframe.yaml`.

```bash
make compile    # Compile
make upload     # Upload to device
make logs       # Stream logs
make run        # Compile + upload + logs
make dashboard  # Web dashboard
make validate   # Validate config
make clean      # Clean build
make test       # Native C++ unit tests (einkframe only)

make run CONFIG=photoframe.yaml   # target another project
```

`compose.yaml` tracks `ghcr.io/esphome/esphome:latest`, but Docker caches it. photoframe needs ESPHome ≥ 2026.8.0 — if validation fails with confusing schema errors, run `docker compose pull esphome`.

## Workflow

We use a spec-driven workflow. All specs live in `specs/`.

### Task Structure

Each task gets a folder under `specs/<project>/tasks/`, numbered sequentially:

```
specs/photoframe/tasks/00-project-setup/
  task-plan.md      # Created together, iterated until agreed
  task-summary.md   # Written after completion
```

### Planning Phase

- Create `task-plan.md` together. Iterate until plan is agreed on
- Plans contain decisions and key implementation details, kept short and readable — no big code blocks
- **DO NOT implement anything until the plan is complete and agreed on**

### Implementation Phase

- Execute the agreed plan

### Summary Phase

- Create `task-summary.md` with actions taken, lessons learned
- Note if the project's `specs/<project>/ARCHITECTURE.md` needs updating (and update it if so)

## Tone

Concise. Brevity over grammar. No fluff.
