#ifdef __linux__
#define _GNU_SOURCE     /* to get asprintf() declaration */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <memory.h>
#include "c_unit_fw.h"

extern const struct c_unit_test _c_unit_tests[];
extern const int _c_unit_ntests;
static const struct c_unit_test *current_test;
static int current_test_index;
static jmp_buf jmp;
#define PASS 0          /* this needs to be zero for setjmp() semantics */
#define FAIL 1
#define SKIP 2

static void tap_header(int ntests)
{
    printf("1..%d\n", ntests);
}

static void tap_test_ended(int status, const char *reason, const char *filename, int lineno)
{
    static const char * const status_strings[3] = { "ok", "not ok", "skip" };

    char *reason_buf = NULL;
    if (reason)
    {
        if (filename && *filename)
            asprintf(&reason_buf, "%s at %s:%d", reason, filename, lineno);
        else
            reason_buf = strdup(reason);
    }
    else if (filename && *filename)
    {
        asprintf(&reason_buf, "at %s:%d", filename, lineno);
    }

    printf(
        "%s %d - %s%s%s\n",
        status_strings[status],
        current_test_index,
        current_test ? current_test->name : "unknown",
        reason_buf ? " # " : "",
        reason_buf ? reason_buf : "");
    free(reason_buf);
}

void _c_unit_assert_failedv(const char *filename, int lineno, const char *message, ...)
{
    char *message_buf = NULL;
    va_list args;
    va_start(args, message);
    vasprintf(&message_buf, message, args);
    va_end(args);

    tap_test_ended(FAIL, message_buf, filename, lineno);
    free(message_buf);
    longjmp(jmp, FAIL);
}

void _c_unit_skip(const char *filename, int lineno, const char *message)
{
    tap_test_ended(SKIP, message, filename, lineno);
    longjmp(jmp, SKIP);
}

int main(int argc, char **argv)
{
    tap_header(_c_unit_ntests);

    int nfailed = 0;
    int npassed = 0;
    for (int i = 0 ; _c_unit_tests[i].name ; i++)
    {
        const struct c_unit_test *t = &_c_unit_tests[i];

        if (t->setup_func)
            t->setup_func();

        current_test_index = i+1;
        current_test = t;
        memset(jmp, 0, sizeof(jmp));
        switch (setjmp(jmp))
        {
        case 0:     /* setjmp() returned naturally */
            t->test_func();
            npassed++;
            tap_test_ended(PASS, NULL, NULL, 0);
            break;
        case FAIL:  /* setjmp() returned from longjmp */
            nfailed++;
            /* FAIL status already reported */
            break;
        case SKIP:  /* setjmp() returned from longjmp */
            /* SKIP status already reported */
            break;
        }
        current_test_index = 0;
        current_test = 0;

        if (t->teardown_func)
            t->teardown_func();
    }
    return (nfailed ? 1 : 0);
}
