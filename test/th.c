#include "th.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TH_MAX_TESTS 256
#define TH_MSG_LEN   512

typedef struct {
    const char *suite;
    const char *name;
    double      seconds;
    int         failed;
    char        message[TH_MSG_LEN];
} th_result_t;

static th_result_t results[TH_MAX_TESTS];
static int         n_results;
static const char *current_suite = "unknown";
static th_result_t *current;
int th_current_failed;

void th_begin_suite(const char *suite)
{
    current_suite = suite;
}

void th_format_eq(char *buf, unsigned long n, const char *expr, long long e, long long a)
{
    snprintf(buf, (size_t)n, "%s: expected %lld but was %lld", expr, e, a);
}

void th_fail(const char *file, int line, const char *msg)
{
    th_current_failed = 1;
    if (current != NULL && !current->failed) {
        current->failed = 1;
        snprintf(current->message, sizeof current->message, "%s:%d: %s", file, line, msg);
    }
}

void th_run(const char *name, th_fn fn)
{
    clock_t start;

    if (n_results >= TH_MAX_TESTS) {
        fprintf(stderr, "th: too many tests\n");
        exit(2);
    }
    current = &results[n_results++];
    memset(current, 0, sizeof *current);
    current->suite = current_suite;
    current->name = name;
    th_current_failed = 0;

    start = clock();
    fn();
    current->seconds = (double)(clock() - start) / CLOCKS_PER_SEC;

    printf("%s %s.%s%s%s\n", current->failed ? "FAIL" : "PASS", current->suite, name,
           current->failed ? "  -- " : "", current->failed ? current->message : "");
    current = NULL;
}

static void xml_escape(FILE *f, const char *s)
{
    for (; *s != '\0'; s++) {
        switch (*s) {
        case '&': fputs("&amp;", f); break;
        case '<': fputs("&lt;", f); break;
        case '>': fputs("&gt;", f); break;
        case '"': fputs("&quot;", f); break;
        case '\'': fputs("&apos;", f); break;
        default: fputc(*s, f); break;
        }
    }
}

int th_finish(const char *junit_path)
{
    int i, j, total_fail = 0;
    double total_time = 0.0;
    FILE *f;

    for (i = 0; i < n_results; i++) {
        total_fail += results[i].failed;
        total_time += results[i].seconds;
    }

    f = fopen(junit_path, "w");
    if (f == NULL) {
        perror(junit_path);
        return total_fail > 0 ? total_fail : 1;
    }

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<testsuites name=\"wpc-firmware-demo\" tests=\"%d\" failures=\"%d\" time=\"%.6f\">\n",
            n_results, total_fail, total_time);

    /* One <testsuite> per module, in order of first appearance. */
    for (i = 0; i < n_results; i++) {
        int first = 1, tests = 0, fails = 0;
        double t = 0.0;
        for (j = 0; j < i; j++) {
            if (strcmp(results[j].suite, results[i].suite) == 0) {
                first = 0;
                break;
            }
        }
        if (!first) {
            continue;
        }
        for (j = i; j < n_results; j++) {
            if (strcmp(results[j].suite, results[i].suite) == 0) {
                tests++;
                fails += results[j].failed;
                t += results[j].seconds;
            }
        }
        fprintf(f, "  <testsuite name=\"");
        xml_escape(f, results[i].suite);
        fprintf(f, "\" tests=\"%d\" failures=\"%d\" errors=\"0\" skipped=\"0\" time=\"%.6f\">\n",
                tests, fails, t);
        for (j = i; j < n_results; j++) {
            const th_result_t *r = &results[j];
            if (strcmp(r->suite, results[i].suite) != 0) {
                continue;
            }
            fprintf(f, "    <testcase classname=\"");
            xml_escape(f, r->suite);
            fprintf(f, "\" name=\"");
            xml_escape(f, r->name);
            fprintf(f, "\" time=\"%.6f\"", r->seconds);
            if (r->failed) {
                fprintf(f, ">\n      <failure message=\"");
                xml_escape(f, r->message);
                fprintf(f, "\" type=\"AssertionError\">");
                xml_escape(f, r->message);
                fprintf(f, "</failure>\n    </testcase>\n");
            } else {
                fprintf(f, "/>\n");
            }
        }
        fprintf(f, "  </testsuite>\n");
    }
    fprintf(f, "</testsuites>\n");
    fclose(f);

    printf("\n%d tests, %d failures -> %s\n", n_results, total_fail, junit_path);
    return total_fail;
}
