# Unit Testing

## Purpose

Unit tests in this repository validate component behavior, not entrypoint wiring.

- Test logic in `components/common/*` and `components/<board>/*`.
- Keep `src/<board>/main.c` focused on startup and task orchestration.
- Treat tests as ownership-based: each component should have tests in the matching `test/<owner>/` path.

## Test stack

This project uses PlatformIO's unit-test runner with Unity.

- Unity lifecycle is used in each test binary:
  - `UNITY_BEGIN()`
  - `RUN_TEST(...)`
  - `UNITY_END()`
- Global Unity output hooks are provided by:
  - `test/unity_config.h`
  - `test/unity_config.c`
- Current Unity output implementation initializes UART and writes each output character over COM.

## Test layout

Tests are grouped by ownership and module:

```text
test/
|- common/
|  |- test_bucketed_pqueue/
|  |- test_kv_pool/
|- sensor_board/
|  |- test_gps_sensor/
|  |- test_imu_sensor/
|  |- test_ph_sensor/
|  |- test_sensor_basics/
|- driving_board/
|  |- test_calculator/
|  |- test_motor/
|- debugging_board/
|  |- test_input_handler/
|- unity_config.c
|- unity_config.h
```

Naming convention:

- Folder: `test/<owner>/test_<module>/`
- File: `test_<behavior>.c` or `test_<module>.c`
- Function: `test_<expected_behavior>()`

## Running tests

Run all tests for an environment:

```bash
pio test -e sensor_board
```

Run all tests across environments:

```bash
pio test
```

Run one test directory:

```bash
pio test -e sensor_board -f test_imu_sensor
```

## Environment test selection (`platformio.ini`)

Test scope is controlled per environment using `test_filter`.

Current configuration:

- `env:sensor_board` -> `test_filter = sensor_board/*`
- `env:driving_board` -> `test_filter = driving_board/*`
- `env:debugging_board` -> `test_filter = common/*`

When adding tests, make sure the target environment's `test_filter` includes that path, otherwise the tests will not run in that environment.

## Writing a new unit test

### 1) Place it by ownership

If you add code in `components/common/kv_pool`, place tests under `test/common/test_kv_pool/`.

### 2) Use Unity structure

```c
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_example_behavior(void) {
    TEST_ASSERT_TRUE(1);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_example_behavior);
    return UNITY_END();
}
```

### 3) Assert behavior, not implementation

- Prefer public API calls over internal-state poking.
- Cover success paths and error paths.
- Use precise assertions (`TEST_ASSERT_EQUAL`, `TEST_ASSERT_FLOAT_WITHIN`, etc).

### 4) Keep tests deterministic

- Avoid hidden dependencies on prior test state.
- Reset fixtures in `setUp`.
- Use bounded delays/timeouts only when unavoidable.

## What to test

- Public functions in component modules.
- Input validation and error handling.
- Boundary values and invalid arguments.
- State transitions and invariants.

## What not to test as unit tests

- Full board startup flows in `main.c`.
- End-to-end hardware integration behavior.
- Long-running system orchestration across multiple components.

Those belong in integration/system testing, not component unit tests.

## PR checklist

Before merging changes that touch component logic:

- [ ] New/changed component behavior has corresponding tests.
- [ ] Tests live under the correct owner path in `test/`.
- [ ] Error paths are covered, not only happy paths.
- [ ] Test names describe expected behavior clearly.
- [ ] Environment `test_filter` includes the new tests.

## Troubleshooting

- No tests run: check `test_filter` for the selected `-e` environment.
- Test file ignored: ensure folder naming matches `test_<module>` and sits under `test/`.
- No Unity output: verify UART/COM setup in `test/unity_config.c` and board connection settings.
