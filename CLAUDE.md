# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Role

You are an expert in ESPHome, IoT, and embedded systems. Deep knowledge of ESPHome YAML config, component APIs, C++ lambdas, ESP-IDF/Arduino frameworks, sensor integration, display rendering, power management, and Home Assistant integration.

## Project Overview

ESPHome config project for an eInkFrame. See [specs/ARCHITECTURE.md](specs/ARCHITECTURE.md) for full architecture, hardware details, and file structure.

## Commands

ESPHome runs via Docker through Make targets:

```bash
make compile    # Compile
make upload     # Upload to device
make logs       # Stream logs
make run        # Compile + upload + logs
make dashboard  # Web dashboard
make validate   # Validate config
make clean      # Clean build
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
