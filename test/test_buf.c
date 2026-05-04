#include "pgenc/buf.h"
#include <stdio.h>
#include <ctype.h>

static void test_init(void)
{
    puts("it properly initializes");
    char bytes[8];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 8);
    pgc_test(buf.base == buf.offset);
    pgc_test(buf.base == buf.end);
    pgc_test(buf.fence == (unsigned char*)(bytes + 8));
}

static void test_putc_getc(void)
{
    puts("it puts and gets unsigned char");
    char bytes[2];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 2);
    pgc_test(pgc_buf_put_char(&buf, '1') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, '2') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, '3') == PGC_BOFLO);
    pgc_test(pgc_buf_get_char(&buf) == '1');
    pgc_test(pgc_buf_get_char(&buf) == '2');
    pgc_test(pgc_buf_put_char(&buf, 'a') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, 'b') == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == 'a');
    pgc_test(pgc_buf_get_char(&buf) == 'b');
}

static void test_tell_seek(void)
{
    puts("it tells and seeks");
    char bytes[2];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 2);
    pgc_test(pgc_buf_put_char(&buf, '1') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, '2') == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == '1');
    pgc_test(pgc_buf_tell(&buf) == 1);
    pgc_test(pgc_buf_seek(&buf, 0) == PGC_OK);
    pgc_test(pgc_buf_tell(&buf) == 0);
    pgc_test(pgc_buf_seek(&buf, 2) == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, 'x') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, 'y') == PGC_OK);
    pgc_test(pgc_buf_put_char(&buf, 'z') == PGC_BOFLO);
    pgc_test(pgc_buf_seek(&buf, 3) == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == 'y');
    pgc_test(pgc_buf_get_char(&buf) == PGC_BUFLO);
    pgc_test(pgc_buf_put_char(&buf, 'z') == PGC_OK);
    pgc_test(pgc_buf_tell(&buf) == 4);
    pgc_test(pgc_buf_get_char(&buf) == 'z');
    pgc_test(pgc_buf_get_char(&buf) == PGC_BUFLO);
    pgc_test(pgc_buf_seek(&buf, 1) == PGC_ISEEK);
}

static void test_put(void)
{
    puts("it reads binary data via put");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    pgc_test(pgc_buf_put(&buf, "cat", 3) == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == 'c');
    pgc_test(pgc_buf_get_char(&buf) == 'a');
    pgc_test(pgc_buf_get_char(&buf) == 't');
    pgc_test(pgc_buf_put(&buf, "dog", 3) == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == 'd');
    pgc_test(pgc_buf_get_char(&buf) == 'o');
    pgc_test(pgc_buf_get_char(&buf) == 'g');
    pgc_test(pgc_buf_tell(&buf) == 6); 
    pgc_test(pgc_buf_put(&buf, "hi", 2) == PGC_OK);
    pgc_test(pgc_buf_get_char(&buf) == 'h');
    pgc_test(pgc_buf_get_char(&buf) == 'i');
    pgc_test(pgc_buf_tell(&buf) == 8);
    pgc_test(pgc_buf_put(&buf, "hat", 3) == PGC_OK);
    pgc_test(pgc_buf_put(&buf, " ", 1) == PGC_BOFLO);
    pgc_test(pgc_buf_get_char(&buf) == 'h');
    pgc_test(pgc_buf_get_char(&buf) == 'a');
    pgc_test(pgc_buf_get_char(&buf) == 't');
}

static void test_get(void) 
{
    puts("it writes binary data via get");
    char bytes[3];
    char tmp[3] = {0,0,0};
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    pgc_test(pgc_buf_put(&buf, "hi", 2) == PGC_OK);
    pgc_test(pgc_buf_get(&buf, tmp, 2) == 2);
    pgc_test(strcmp(tmp, "hi") == 0);
    pgc_test(pgc_buf_get(&buf, tmp, 2) == PGC_OK);
    pgc_test(pgc_buf_put(&buf, "go", 2) == PGC_OK);
    tmp[1] = 0; 
    pgc_test(pgc_buf_get(&buf, tmp, 1) == 1);
    pgc_test(strcmp(tmp, "g") == 0);
    pgc_test(pgc_buf_get(&buf, tmp, 2) == 1);
    pgc_test(strcmp(tmp, "o") == 0);
    pgc_test(pgc_buf_put(&buf, "n", 1) == PGC_OK);
    pgc_test(pgc_buf_put(&buf, "o", 1) == PGC_OK);
    pgc_test(pgc_buf_get(&buf, tmp, 3) == 2);
    pgc_test(strcmp(tmp, "no") == 0);
    pgc_test(pgc_buf_put(&buf, "ya", 2) == PGC_OK);
    pgc_test(pgc_buf_get(&buf, tmp, 3) == 2);
    pgc_test(strcmp(tmp, "ya") == 0);
    pgc_test(pgc_buf_put(&buf, "hi", 2) == PGC_OK);
    pgc_test(pgc_buf_get(&buf, tmp, 2) == 2);
    pgc_test(strcmp(tmp, "hi") == 0);
}

static void test_match_str(void)
{
    puts("it matches strings by equality");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    pgc_test(pgc_buf_put(&buf, "hi", 2) == PGC_OK);
    pgc_test(pgc_buf_match_str(&buf, "ha", 2) == PGC_NOMAT);
    pgc_test(pgc_buf_match_str(&buf, "his", 3) == PGC_BUFLO);
    pgc_test(pgc_buf_match_str(&buf, "hi", 2) == PGC_OK);
    pgc_test(pgc_buf_tell(&buf) == 2);
}

static void test_match_char(void)
{
    puts("it matches char by set membership");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    struct pgc_cset set;
    pgc_cset_from(&set, isalnum);
    pgc_test(pgc_buf_put(&buf, "h2%", 3) == PGC_OK);
    pgc_test(pgc_buf_match_set(&buf, &set) == PGC_OK); 
    pgc_test(pgc_buf_match_set(&buf, &set) == PGC_OK);
    pgc_test(pgc_buf_match_set(&buf, &set) == PGC_NOMAT);
    pgc_test(pgc_buf_seek(&buf, 3) == 0);
    pgc_test(pgc_buf_match_set(&buf, &set) == PGC_BUFLO);
}

static void test_peek_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;

    uint32_t v = 0;

    char *sym1 = "A";   // 1 byte - 0x41
    puts("it decodes a single byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, sym1, 1) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == 1);
    pgc_test(v == 0x41);
     
    char *sym2 = "£";   // 2 bytes - 0x00A3
    puts("it decodes a 2 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, sym2, 2) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == 2);
    pgc_test(v == 0xA3);

    char *sym3 = "❮";   // 3 bytes - 0x276E
    puts("it decodes a 3 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, sym3, 3) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == 3);
    pgc_test(v == 0x276E);

    char *sym4 = "𝐀";   // 4 bytes - 0x1D400 
    puts("it decodes a 4 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, sym4, 4) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == 4);
    pgc_test(v == 0x1D400);

    // 2-byte: Overlong encoding of 'A' (0x41)
    // Correct is 0x41. Improper is 0xC1 0x81.
    uint8_t bad_2b[] = {0xC1, 0x81}; 
    puts("it fails when passing an improperly encoded 2 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, bad_2b, 2) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);

    // 3-byte: Invalid continuation byte
    // Sequence starts like 3-byte, but 2nd byte isn't 10xxxxxx wants 0x80-0xBF
    uint8_t bad_3b[] = {0xE2, 0x20, 0xA2}; 
    puts("it fails when passing an improperly encoded 3 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, bad_3b, 3) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);

    // 4-byte: Out of Unicode range (> 0x10FFFF)
    // 0xF4 0xBF 0xBF 0xBF is valid, 0xF5 0x80 0x80 0x80 is out of bounds.
    uint8_t bad_4b[] = {0xF5, 0x80, 0x80, 0x80};
    puts("it fails when passing an improperly encoded 4 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    pgc_test(pgc_buf_put(&buf, bad_4b, 4) == PGC_OK);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);

    // 3-byte: Surrogate U+D800
    // Pattern: 1110 1101, 10 100000, 10 000000
    uint8_t bad_surrogate[] = {0xED, 0xA0, 0x80}; 
    puts("it fails on UTF-16 surrogate range");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, bad_surrogate, 3);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);

    // Starts a 4-byte sequence but only provides 2 bytes
    uint8_t truncated_4b[] = {0xF0, 0x9F}; 
    puts("it handles truncated multi-byte sequences");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, truncated_4b, 2);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_BUFLO);
    
    // A lone continuation byte (0x80)
    uint8_t lone_cont[] = {0x80}; 
    puts("it fails on a lone continuation byte");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, lone_cont, 1);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);

    // Overlong NUL (3-byte version of 0x00)
    uint8_t overlong_nul[] = {0xE0, 0x80, 0x80}; 
    puts("it fails on overlong encoded NUL");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, overlong_nul, 3);
    pgc_test(pgc_buf_peek_utf8(&buf, &v) == PGC_IUTF8);
}

static void test_get_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;

    uint32_t v = 0;

    char *str1 = "£❮𝐀1";

    puts("it decodes a single byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, str1, strlen(str1)); 
    pgc_test(pgc_buf_get_utf8(&buf, &v) > 0);
    pgc_test(v == 0xA3);
    pgc_test(pgc_buf_get_utf8(&buf, &v) > 0);
    pgc_test(v == 0x276E);
    pgc_test(pgc_buf_get_utf8(&buf, &v) > 0);
    pgc_test(v == 0x1D400);
    pgc_test(pgc_buf_get_utf8(&buf, &v) == 1);
    pgc_test(v == '1');
    pgc_test(pgc_buf_get_utf8(&buf, &v) == PGC_BUFLO);
}

static void test_match_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;
    
    struct pgc_utf8_range rs[] = {
        { 0xA2, 0x3000 }
    };
    char *str1 = "£❮1𝐀";

    puts("it can match utf8 characters with a predicate");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, str1, strlen(str1));
    pgc_test(pgc_buf_match_utf8(&buf, rs, 1) > 0);
    pgc_test(pgc_buf_match_utf8(&buf, rs, 1) > 0);
    pgc_test(pgc_buf_match_utf8(&buf, rs, 1) == PGC_NOMAT);
    pgc_test(pgc_buf_get_char(&buf) == '1');
    pgc_test(pgc_buf_match_utf8(&buf, rs, 1) == PGC_NOMAT);
    pgc_test(pgc_buf_test_str(&buf, "𝐀", 4) == PGC_OK);
}

static void test_request_advance(void)
{
    char bytes[4];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 4);
    puts("it can request a readable region and advance");
    pgc_buf_put(&buf, "12", 2);
    pgc_test(pgc_buf_get_char(&buf) == '1');
    char *win = pgc_buf_request(&buf, 3);
    pgc_test(win != NULL);
    win[0] = '3';
    win[1] = '4';
    win[2] = '5';
    pgc_buf_advance(&buf, 3);
    pgc_test(pgc_buf_get_char(&buf) == '2');
    pgc_test(pgc_buf_get_char(&buf) == '3');
    pgc_test(pgc_buf_get_char(&buf) == '4');
    pgc_test(pgc_buf_get_char(&buf) == '5');
}

static void test_claim_consume(void)
{
    char bytes[4];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 4);
    puts("it can claim a chunk and consume it");
    pgc_buf_put(&buf, "1234", 4);
    pgc_test(pgc_buf_get_char(&buf) == '1');
    char *win = pgc_buf_claim(&buf, 3);
    pgc_test(win != NULL);
    pgc_test(win[0] == '2');
    pgc_test(win[1] == '3');
    pgc_buf_consume(&buf, 2);
    pgc_test(pgc_buf_get_char(&buf) == '4');
}

int main(int argc, char **args) 
{
    (void)argc;
    (void)args;
    puts("testing buf");
    test_init();
    test_putc_getc();
    test_tell_seek();
    test_put();
    test_get();
    test_match_str();
    test_match_char();
    test_peek_utf8();
    test_get_utf8();
    test_match_utf8();
    test_request_advance();
    test_claim_consume();
    puts("all buf tests passed");
}
