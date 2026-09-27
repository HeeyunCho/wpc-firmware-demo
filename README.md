# wpc-firmware-demo

Demo **system under test** for the [V-Model Agent](https://heeyun.dev/v-model): a small, realistic
embedded C99 codebase for an in-vehicle **Qi2 wireless charging module (WCM)**. Its GitHub Actions
run posts JUnit results to the agent, which maps each test onto a V-Model test case, records the run
as ASPICE verification evidence and, on failure, opens a Bug and starts the triage loop.

> Not production firmware. Calibration values are plausible but illustrative.

## Modules

| Module | Header | Responsibility |
|---|---|---|
| `fod_detector` | `include/fod_detector.h` | Foreign object detection: Q-factor (pre-transfer) and debounced power-loss (during transfer) |
| `thermal_manager` | `include/thermal_manager.h` | Coil derating curve (100 % → 40 % between 45–60 °C), shutdown at 70 °C, resume ≤ 55 °C, sensor-fault shutdown |
| `nfc_guard` | `include/nfc_guard.h` | NFC key-card detected → pause power transfer; resume after 3 absent polls |
| `can_status` | `include/can_status.h` | `WCM_Status` CAN frame (0x3A5) packing/unpacking, alive counter, checksum |
| `phone_reminder` | `include/phone_reminder.h` | Phone-left-behind warning on ignition off + driver door open |

## Build & test

```sh
make test            # builds with -std=c99 -Wall -Wextra -Werror -pedantic, runs all tests
                     # writes build/junit.xml, exits non-zero on any failure
make clean
```

Requires only a C99 compiler and `make`. The test harness (`test/th.[ch]`) is a ~150-line
self-contained runner that emits JUnit XML.

## Test id convention

Each JUnit `<testcase>` has `classname` = module name and `name` = test function name. The V-Model
agent's test case attribute `automated_test_id` is `<classname>.<name>`, for example
`thermal_manager.test_shutdown_at_threshold`. See `TEST_IDS.md` for the full list.

## Demo: deliberate bug (CI → Bug → triage loop)

`include/wpc_config.h` holds `WPC_DEMO_THERMAL_BUG`. When it is `1`, `thermal_manager` uses `>`
instead of `>=` for the 70 °C shutdown threshold, so at exactly 70 °C the coil keeps delivering 40 %
power. `thermal_manager.test_shutdown_at_threshold` then fails.

Three ways to switch it on:

1. **Locally:** `make test DEMO_BUG=1`
2. **One CI run, no commit:** Actions → *ci* → *Run workflow* → `demo_bug = 1`
   (or `gh workflow run ci.yml -f demo_bug=1`)
3. **Committed regression:** set `#define WPC_DEMO_THERMAL_BUG 1` in `include/wpc_config.h`, push.
   Revert to `0` (the "fix") and push again to show the loop closing with a green run.

## CI → V-Model agent

`.github/workflows/ci.yml` runs on push, pull request and manual dispatch:

1. `make test` (the job goes red when a test fails)
2. uploads `build/junit.xml` as the `junit-xml` artifact
3. POSTs `{ repo, sha, run_url, junit_xml }` as JSON to `vars.VMODEL_INGEST_URL` with header
   `x-ci-secret: secrets.VMODEL_CI_SECRET`. The step runs even after a test failure, is skipped when
   the variable is unset, and never fails the build if the endpoint is unreachable.

Configure:

```sh
gh variable set VMODEL_INGEST_URL --body "https://heeyun.dev/v-model/api/ci/ingest"
gh secret set VMODEL_CI_SECRET      # value = CI_INGEST_SECRET of the V-Model deployment
```
