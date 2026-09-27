/**
 * @file wpc_config.h
 * @brief Build-time configuration of the WCM firmware.
 *
 * DEMO SWITCH
 * -----------
 * WPC_DEMO_THERMAL_BUG injects a deliberate off-by-one into the thermal
 * shutdown boundary (thermal_manager.c). With it enabled, the coil keeps
 * transferring derated power at exactly WPC_THERMAL_SHUTDOWN_C instead of
 * shutting down, and the unit test
 *   thermal_manager.test_shutdown_at_threshold
 * fails. This drives the V-Model agent's "CI failure -> Bug -> triage" loop.
 *
 * Toggle with either:
 *   make test DEMO_BUG=1            (command line, no source change)
 *   change the default below to 1   (commit + push -> red CI run)
 */
#ifndef WPC_CONFIG_H
#define WPC_CONFIG_H

#ifndef WPC_DEMO_THERMAL_BUG
#define WPC_DEMO_THERMAL_BUG 0
#endif

#endif /* WPC_CONFIG_H */
