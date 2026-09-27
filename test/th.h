/**
 * @file th.h
 * @brief Tiny C99 unit-test harness that writes JUnit XML.
 *
 * JUnit mapping (consumed by the V-Model agent's CI ingest):
 *   <testcase classname="<suite>" name="<test function>">
 * The V-Model test case attribute automated_test_id is "<suite>.<test function>",
 * e.g. "thermal_manager.test_shutdown_at_threshold".
 */
#ifndef TH_H
#define TH_H

#include <stdint.h>

typedef void (*th_fn)(void);

void th_begin_suite(const char *suite);
void th_run(const char *name, th_fn fn);
void th_fail(const char *file, int line, const char *msg);
int  th_finish(const char *junit_path); /* returns number of failures */

/* Current test status: set by th_fail, checked by the assertion macros. */
extern int th_current_failed;

#define TH_RUN(fn) th_run(#fn, fn)

#define TH_ASSERT(cond)                                                    \
    do {                                                                   \
        if (!(cond)) {                                                     \
            th_fail(__FILE__, __LINE__, "assertion failed: " #cond);       \
            return;                                                        \
        }                                                                  \
    } while (0)

#define TH_ASSERT_EQ_INT(expected, actual)                                 \
    do {                                                                   \
        long long th_e_ = (long long)(expected);                           \
        long long th_a_ = (long long)(actual);                             \
        if (th_e_ != th_a_) {                                              \
            char th_buf_[256];                                             \
            th_format_eq(th_buf_, sizeof th_buf_, #actual, th_e_, th_a_);  \
            th_fail(__FILE__, __LINE__, th_buf_);                          \
            return;                                                        \
        }                                                                  \
    } while (0)

void th_format_eq(char *buf, unsigned long n, const char *expr, long long e, long long a);

#endif /* TH_H */
