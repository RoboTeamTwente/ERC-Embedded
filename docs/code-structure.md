# Code Structure

## Architecture in one sentence

Each board's `main.c` is only an orchestrator: it initializes the runtime, spawns tasks, and hands all behavior to components.

## Core design contract

This repository enforces a strict split between entrypoints and components:

- `src/<board>/main.c` is the process entrypoint for that board target.
- `components/common/*` contains reusable logic shared by multiple boards.
- `components/<board>/*` contains board-specific logic.
- `components/<board>/firmware/*` contains generated/vendor firmware and MCU integration files.

The most important rule is:

> `main.c` should not contain domain logic. It should only wire systems together and start tasks.

## Repository layout

```text
erc/
|- src/
|  |- arm_board/main.c
|  |- driving_board/main.c
|  |- sensor_board/main.c
|  |- network_board/main.c
|  |- debugging_board/main.c
|
|- components/
|  |- common/                    # Shared modules used across boards
|  |- arm_board/                 # Arm-board-specific modules
|  |- driving_board/             # Driving-board-specific modules
|  |- sensor_board/              # Sensor-board-specific modules
|  |- network_board/             # Network-board-specific modules
|  |- debugging_board/           # Debugging-board-specific modules
|
|- test/
|  |- common/
|  |- arm_board/
|  |- driving_board/
|  |- sensor_board/
|  |- debugging_board/
|
|- scripts/                      # Utility scripts (for codegen/post-processing)
|- platformio.ini                # Build environments and board filters
```

## Entrypoint responsibilities (`src/<board>/main.c`)

`main.c` should only do these steps:

1. Perform mandatory low-level startup (HAL/clock/cache/MPU/RTOS init as needed).
2. Initialize infrastructure dependencies (GPIO, UART, timers, networking stack wrappers).
3. Spawn one or more RTOS tasks.
4. Start scheduler/kernel.
5. Never own feature behavior directly.

### What should NOT live in `main.c`

- Sensor processing algorithms.
- Business/control logic.
- Packet parsing/dispatch policy.
- Device-specific behavior beyond startup wiring.
- Long loops that implement feature behavior directly.

If logic grows beyond basic startup and task creation, move it into an appropriate component module and call that module from the task.

## Component responsibilities (`components/*`)

Components own behavior. Tasks should delegate into components instead of implementing logic inline.

Examples:

- Sensor behavior belongs in `components/sensor_board/*` (GPS/IMU/pH/sensor basics).
- Driving behavior belongs in `components/driving_board/*` (calculator/motor/parser/simulink wrappers).
- Debug UI/diagnostics behavior belongs in `components/debugging_board/*`.
- Reusable infrastructure belongs in `components/common/*` (result types, logging helpers, queues, dispatch helpers, etc).

## Execution model

Use this mental model for every board:

```text
main.c
  -> platform/runtime init
  -> create task(s)
  -> each task calls component API(s)
  -> component modules perform the real work
```

## Testing structure

Tests are organized by ownership, mirroring components:

- `test/common/*` for shared modules.
- `test/sensor_board/*` for sensor board modules.
- `test/driving_board/*` for driving board modules.
- `test/debugging_board/*` for debugging board modules.

Keep tests close to component boundaries, not entrypoint wiring.

For detailed testing conventions and commands, see `docs/unit-testing.md`.
