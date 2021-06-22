#ifndef __c_unit_fw_h__
#define __c_unit_fw_h__ 1

#include <stdarg.h>

#define _c_unit_stringify(x)                #x
#define _c_unit_safe_string(x)              ((x) ? (x) : "")
#define _c_unit_safe_string_format_args(x)  (x) ? "\"" : "", (x) ? (x) : "(null)", (x) ? "\"" : ""

extern void _c_unit_assert_failedv(const char *filename, int lineno, const char *message, ...)
    __attribute__((noreturn));
extern void _c_unit_skip(const char *filename, int lineno, const char *message)
    __attribute__((noreturn));

#define assert_equals(_actual, _expected) \
    do { \
        __typeof__(_actual) _a = (_actual); \
        __typeof__(_expected) _e = (_expected); \
        if (_a != _e) \
            _c_unit_assert_failedv(__FILE__, __LINE__, \
                "assert_equals(%s=%lld, %s=%lld)", \
                _c_unit_stringify(_actual), _a, \
                _c_unit_stringify(_expected), _e); \
    } while(0)

#define assert_str_equals(_actual, _expected) \
    do { \
        const char *_a = (_actual); \
        const char *_e = (_expected); \
        if (strcmp(_c_unit_safe_string(_a), _c_unit_safe_string(_e))) \
            _c_unit_assert_failedv(__FILE__, __LINE__, \
                "assert_str_equals(%s=%s%s%s, %s=%s%s%s)", \
                _c_unit_stringify(_actual), \
                _c_unit_safe_string_format_args(_a), \
                _c_unit_stringify(_expected), \
                _c_unit_safe_string_format_args(_e)); \
    } while(0)

#define assert_true(_actual) \
    do { \
        __typeof__(_actual) _a = (_actual); \
        if (!_a) \
            _c_unit_assert_failedv(__FILE__, __LINE__, "assert_true(%s)", _c_unit_stringify(_actual)); \
    } while(0)

#define skip() \
    _c_unit_skip(__FILE__, __LINE__)
#define skipm(_message) \
    _c_unit_skip(__FILE__, __LINE__, (_message))

struct c_unit_test
{
    const char *name;
    void (*test_func)(void);
    void (*setup_func)(void);
    void (*teardown_func)(void);
};

#endif /* __c_unit_fw_h__ */
