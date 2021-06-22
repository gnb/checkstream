#include "c_unit_fw.h"
#include "common.h"

const char *argv0 = "tcommon.c";


void test_parse_length()
{
#define CANARY  ~0ULL
#define TESTCASE(_str, _expected_return, _expected_l) \
    { \
        uint64_t l = CANARY; \
        assert_equals(parse_length((_str), &l), _expected_return); \
        assert_equals(l, _expected_l); \
    }

    /* null and empty strings fail and don't update the l value */
    TESTCASE(NULL, false, CANARY);
    TESTCASE("", false, CANARY);

    /* non-numeric strings fail and don't update the l value */
    TESTCASE("foo", false, CANARY);
    TESTCASE("^&#", false, CANARY);

    /* bad units and junk after the unit */
    TESTCASE("3 ", false, CANARY);
    TESTCASE("3b", false, CANARY);      /* the bb/BB units need both characters */
    TESTCASE("3B", false, CANARY);
    TESTCASE("3gb", false, CANARY);
    TESTCASE("3giga", false, CANARY);
    TESTCASE("3gxyz", false, CANARY);
    TESTCASE("3x", false, CANARY);
    TESTCASE("3xyz", false, CANARY);

    /* basic numbers */
    TESTCASE("0", true, 0);
    TESTCASE("1", true, 1);
    TESTCASE("9", true, 9);
    TESTCASE("10", true, 10);
    TESTCASE("1234", true, 1234);

    /* numbers near significant boundaries */
    TESTCASE("2147483648", true, 2147483648);              /* 2^30 */
    TESTCASE("4294967296", true, 4294967296);              /* 2^31 */
    TESTCASE("8589934592", true, 8589934592);              /* 2^32 */
    TESTCASE("2199023255552", true, 2199023255552);        /* 2^40 */
    TESTCASE("2251799813685248", true, 2251799813685248);  /* 2^50 */

    /* decimal numbers with units */
    TESTCASE("3bb", true, 1536);
    TESTCASE("3BB", true, 1536);
    TESTCASE("3k", true, 3072);
    TESTCASE("3K", true, 3072);
    TESTCASE("3m", true, 3145728);
    TESTCASE("3M", true, 3145728);
    TESTCASE("3g", true, 3221225472);
    TESTCASE("3G", true, 3221225472);
    TESTCASE("3t", true, 3298534883328);
    TESTCASE("3T", true, 3298534883328);

    /* hexadecimal numbers with units */
    /* the bb/BB unit cannot work with hex numbers as it is itself a sequence of valid hex digits */
    TESTCASE("0x3bb", true, 955);
    TESTCASE("0x3BB", true, 955);
    TESTCASE("0x3k", true, 3072);
    TESTCASE("0x3K", true, 3072);
    TESTCASE("0x3m", true, 3145728);
    TESTCASE("0x3M", true, 3145728);
    TESTCASE("0x3g", true, 3221225472);
    TESTCASE("0x3G", true, 3221225472);
    TESTCASE("0x3t", true, 3298534883328);
    TESTCASE("0x3T", true, 3298534883328);

#undef TESTCASE
#undef CANARY
}


void test_parse_tag()
{
#define CANARY  0xaaU
#define TESTCASE(_str, _expected_return, _expected_t) \
    { \
        uint8_t t = CANARY; \
        assert_equals(parse_tag((_str), &t), _expected_return); \
        assert_equals(t, _expected_t); \
    }

    /* null and empty strings fail and don't update the l value */
    TESTCASE(NULL, false, CANARY);
    TESTCASE("", false, CANARY);

    /* basic numbers */
    TESTCASE("0", true, 0);
    TESTCASE("1", true, 1);
    TESTCASE("9", true, 9);
    TESTCASE("10", true, 10);

    /* numbers outside range */
    TESTCASE("256", false, CANARY);
    TESTCASE("1234", false, CANARY);
    TESTCASE("12345678", false, CANARY);

    /* numbers near significant boundaries */
    TESTCASE("255", true, 255);

    /* hexadecimal numbers */
    TESTCASE("0x3", true, 3);
    TESTCASE("0x34", true, 52);

#undef TESTCASE
#undef CANARY
}

void test_parse_protocol()
{
#define CANARY  ~0
#define TESTCASE(_str, _expected_return, _expected_p) \
    { \
        int p = CANARY; \
        assert_equals(parse_protocol((_str), &p), _expected_return); \
        assert_equals(p, _expected_p); \
    }

    /* null and empty strings fail and don't update the p value */
    TESTCASE(NULL, false, CANARY);
    TESTCASE("", false, CANARY);

    /* valid values */
    TESTCASE("tcp", true, IPPROTO_TCP);

    /* invalid values */
    TESTCASE("udp", false, CANARY);
    TESTCASE("foobar", false, CANARY);

#undef TESTCASE
#undef CANARY
}

void test_parse_tcp_port()
{
#define CANARY  0xaaaa
#define TESTCASE(_str, _expected_return, _expected_p) \
    { \
        uint16_t p = CANARY; \
        assert_equals(parse_tcp_port((_str), &p), _expected_return); \
        assert_equals(p, _expected_p); \
    }

    /* null and empty strings fail and don't update the p value */
    TESTCASE(NULL, false, CANARY);
    TESTCASE("", false, CANARY);

    /* basic numbers */
    TESTCASE("1", true, 1);
    TESTCASE("9", true, 9);
    TESTCASE("10", true, 10);

    /* numbers outside range */
    TESTCASE("-1", false, CANARY);
    TESTCASE("0", false, CANARY);
    TESTCASE("65536", false, CANARY);
    TESTCASE("12345678", false, CANARY);

    /* junk after the number */
    TESTCASE("1 ", false, CANARY);
    TESTCASE("1x", false, CANARY);
    TESTCASE("1xyz", false, CANARY);

    /* numbers near significant boundaries */
    TESTCASE("65535", true, 65535);

    /* hexadecimal numbers */
    TESTCASE("0x3", true, 3);
    TESTCASE("0x34", true, 52);

#undef TESTCASE
#undef CANARY
}

void test_iec_sizestr(void)
{
#define TESTCASE(_size, _expected_result) \
    { \
        char _actual[128]; \
        static const char _expected[] = _expected_result; \
        assert_str_equals(iec_sizestr(_size, _actual, sizeof(_actual)), _expected); \
        assert_str_equals(_actual, _expected); \
    }

    TESTCASE(0, "0 B");
    TESTCASE(512, "512 B");
    TESTCASE(1024, "1 KiB");
    TESTCASE(3072, "3 KiB");
    TESTCASE(3145728, "3 MiB");
    TESTCASE(3221225472, "3 GiB");
    TESTCASE(3298534883328, "3 TiB");

    /* iec_sizestr() will use an internal buffer by default, thread safety be damned */
    assert_str_equals(iec_sizestr(3072, NULL, 0), "3 KiB");

#undef TESTCASE
}

void test_tail(void)
{
    /* tail() is not NULL-safe :( */
    assert_str_equals(tail(""), "");
    assert_str_equals(tail("tofu"), "tofu");
    assert_str_equals(tail("/tofu/seitan"), "seitan");
    assert_str_equals(tail("/tofu/seitan/shoreditch"), "shoreditch");
}

