#include "test.h"
#include "pgenc/codec.h"
#include <stdio.h>
#include <string.h>

static void test_decimal_decoder(void)
{
    puts("it can decode decimal strings");
    uint64_t val;
    pgc_decode("5", 1, 10, &pgc_decimal_decoder, &val);
    test(val == 5ULL);
    pgc_decode("0", 1, 10, &pgc_decimal_decoder, &val);
    test(val == 0ULL);
    pgc_decode("9", 1, 10, &pgc_decimal_decoder, &val);
    test(val == 9ULL);
    pgc_decode("a", 1, 10, &pgc_decimal_decoder, &val);
    test(val == (uint64_t)-1);
    pgc_decode("10", 2, 10, &pgc_decimal_decoder, &val);
    test(val == 10ULL);
    pgc_decode("99", 2, 10, &pgc_decimal_decoder, &val);
    test(val == 99ULL);
    pgc_decode("12345", 5, 10, &pgc_decimal_decoder, &val);
    test(val == 12345ULL);
    pgc_decode("987654321", 9, 10, &pgc_decimal_decoder, &val);
    test(val == 987654321ULL);
    char *ulmax = "18446744073709551615";
    pgc_decode(ulmax, strlen(ulmax), 10, &pgc_decimal_decoder, &val);
    test(val == 18446744073709551615ULL);
}

static void test_hex_decoder(void)
{
    puts("it can decode hexadecimal strings");
    uint64_t val;
    pgc_decode("5", 1, 16, &pgc_hex_decoder, &val);
    test(val == 5ULL);
    pgc_decode("0", 1, 16, &pgc_hex_decoder, &val);
    test(val == 0ULL);
    pgc_decode("9", 1, 16, &pgc_hex_decoder, &val);
    test(val == 9ULL);
    pgc_decode("a", 1, 16, &pgc_hex_decoder, &val);
    test(val == 10ULL);
    pgc_decode("A", 1, 16, &pgc_hex_decoder, &val);
    test(val == 10ULL);
    pgc_decode("f", 1, 16, &pgc_hex_decoder, &val);
    test(val == 15ULL);
    pgc_decode("F", 1, 16, &pgc_hex_decoder, &val);
    test(val == 15ULL);
    pgc_decode("10", 2, 16, &pgc_hex_decoder, &val);
    test(val == 0x10ULL);
    pgc_decode("ff", 2, 16, &pgc_hex_decoder, &val);
    test(val == 0xffULL);
    pgc_decode("FF", 2, 16, &pgc_hex_decoder, &val);
    test(val == 0xFFULL);
    pgc_decode("123ABC", 6, 16, &pgc_hex_decoder, &val);
    test(val == 0x123aBcULL);
    char *ulmax = "ffffFFFFffffFFFF";
    pgc_decode(ulmax, strlen(ulmax), 16, &pgc_hex_decoder, &val);
    test(val == 0xFFFFffffFFFFffffULL);
}

static void test_decimal_encoder(void)
{
    puts("it can encode decimal strings");

    char buf[72];
    for (int x = 0; x < 72; ++x) buf[x] = 0;

    test(pgc_encode(0, 10, &pgc_decimal_encoder, buf, 72) == 1);
    test(strcmp("0", buf) == 0);
    test(pgc_encode(0, 10, &pgc_decimal_encoder, buf, 0) == -1);
    test(pgc_encode(1, 10, &pgc_decimal_encoder, buf, 72) == 1);
    test(strcmp("1", buf) == 0);
    test(pgc_encode(11, 10, &pgc_decimal_encoder, buf, 72) == 2);
    test(strcmp("11", buf) == 0);
    test(pgc_encode(99, 10, &pgc_decimal_encoder, buf, 72) == 2);
    test(strcmp("99", buf) == 0);
    test(pgc_encode(123456789, 10, &pgc_decimal_encoder, buf, 72) == 9);
    test(strcmp("123456789", buf) == 0);
    test(pgc_encode(123456789, 10, &pgc_decimal_encoder, buf, 8) == -1);
    test(pgc_encode(18446744073709551615ULL, 
        10, &pgc_decimal_encoder, buf, 72) == 20);
    test(strcmp("18446744073709551615", buf) == 0);
}

static void test_hex_encoder(void)
{
    puts("it can encode hexadecimal strings");

    char buf[72];
    for (int x = 0; x < 72; ++x) buf[x] = 0;

    test(pgc_encode(0, 16, &pgc_hex_encoder, buf, 72) == 1);
    test(strcmp("0", buf) == 0);
    test(pgc_encode(0, 16, &pgc_hex_encoder, buf, 0) == -1);
    test(pgc_encode(1, 16, &pgc_hex_encoder, buf, 72) == 1);
    test(strcmp("1", buf) == 0);
    test(pgc_encode(10, 16, &pgc_hex_encoder, buf, 72) == 1);
    test(strcmp("A", buf) == 0);
    test(pgc_encode(15, 16, &pgc_hex_encoder, buf, 72) == 1);
    test(strcmp("F", buf) == 0);
    test(pgc_encode(0x10, 16, &pgc_hex_encoder, buf, 72) == 2);
    test(strcmp("10", buf) == 0);
    test(pgc_encode(0xFF, 16, &pgc_hex_encoder, buf, 72) == 2);
    test(strcmp("FF", buf) == 0);
    test(pgc_encode(0x1234ABCD, 16, &pgc_hex_encoder, buf, 72) == 8);
    test(strcmp("1234ABCD", buf) == 0);
    test(pgc_encode(0xFFFFFFFF1234ABCD, 16, &pgc_hex_encoder, buf, 72) == 16);
    test(strcmp("FFFFFFFF1234ABCD", buf) == 0);
    test(pgc_encode(0xFFFFFFFFFFFFFFFF, 16, &pgc_hex_encoder, buf, 72) == 16);
    test(strcmp("FFFFFFFFFFFFFFFF", buf) == 0);
}

int main(int argc, char **args)
{
    (void)argc;
    (void)args;
    test_decimal_decoder();
    test_hex_decoder();
    test_decimal_encoder();
    test_hex_encoder();
}
