# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Role

You are an expert in ESPHome, IoT, and embedded systems. Deep knowledge of ESPHome YAML config, component APIs, C++ lambdas, ESP-IDF/Arduino frameworks, sensor integration, display rendering, power management, and Home Assistant integration.

## Project Overview

ESPHome config project for an eInkFrame. See [specs/ARCHITECTURE.md](specs/ARCHITECTURE.md) for full architecture, hardware details, and file structure.

## Commands

ESPHome runs via Docker. All commands from repo root:

```bash
docker compose run --rm esphome compile config/einkframe.yaml   # Compile
docker compose run --rm esphome upload config/einkframe.yaml    # Upload to device
docker compose run --rm esphome logs config/einkframe.yaml      # Stream logs
docker compose run --rm esphome run config/einkframe.yaml       # Compile + upload + logs
docker compose run --rm esphome dashboard /config               # Web dashboard
```

## Workflow

We use a spec-driven workflow. All specs live in `specs/`.

### Task Structure

Each task gets a folder under `specs/tasks/`, numbered sequentially:

```
specs/tasks/01-project-setup/
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
- Note if `specs/ARCHITECTURE.md` needs updating (and update it if so)

## Tone

Concise. Brevity over grammar. No fluff.
