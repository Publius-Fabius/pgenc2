#include "pgenc/buf.h"
#include "test.h"
#include <stdio.h>
#include <ctype.h>

void test_init(void);
void test_putc_getc(void);
void test_tell_seek(void);
void test_put(void);
void test_get(void);
void test_match_str(void);
void test_match_char(void);
void test_peek_utf8(void);
void test_get_utf8(void);
void test_match_utf8(void);
void test_request_advance(void);
void test_claim_consume(void);

bool utf8_pred(const uint32_t, void *);

void test_init(void)
{
    puts("it properly initializes");
    char bytes[8];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 8);
    test(buf.base == buf.offset);
    test(buf.base == buf.end);
    test(buf.fence == (unsigned char*)(bytes + 8));
}

void test_putc_getc(void)
{
    puts("it puts and gets unsigned char");
    char bytes[2];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 2);
    test(pgc_buf_put_char(&buf, '1') == 0);
    test(pgc_buf_put_char(&buf, '2') == 0);
    test(pgc_buf_put_char(&buf, '3') == -1);
    test(pgc_buf_get_char(&buf) == '1');
    test(pgc_buf_get_char(&buf) == '2');
    test(pgc_buf_put_char(&buf, 'a') == 0);
    test(pgc_buf_put_char(&buf, 'b') == 0);
    test(pgc_buf_get_char(&buf) == 'a');
    test(pgc_buf_get_char(&buf) == 'b');
}

void test_tell_seek(void)
{
    puts("it tells and seeks");
    char bytes[2];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 2);
    test(pgc_buf_put_char(&buf, '1') == 0);
    test(pgc_buf_put_char(&buf, '2') == 0);
    test(pgc_buf_get_char(&buf) == '1');
    test(pgc_buf_tell(&buf) == 1);
    test(pgc_buf_seek(&buf, 0) == 0);
    test(pgc_buf_tell(&buf) == 0);
    test(pgc_buf_seek(&buf, 2) == 0);
    test(pgc_buf_seek(&buf, 3) == -1);
    test(pgc_buf_put_char(&buf, 'x') == 0);
    test(pgc_buf_put_char(&buf, 'y') == 0);
    test(pgc_buf_put_char(&buf, 'z') == -1);
    test(pgc_buf_seek(&buf, 3) == 0);
    test(pgc_buf_get_char(&buf) == 'y');
    test(pgc_buf_get_char(&buf) == -1);
    test(pgc_buf_put_char(&buf, 'z') == 0);
    test(pgc_buf_tell(&buf) == 4);
    test(pgc_buf_get_char(&buf) == 'z');
    test(pgc_buf_get_char(&buf) == -1);
    test(pgc_buf_seek(&buf, 1) == -1);
}

void test_put(void)
{
    puts("it reads binary data via put");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    test(pgc_buf_put(&buf, "cat", 3) == 0);
    test(pgc_buf_get_char(&buf) == 'c');
    test(pgc_buf_get_char(&buf) == 'a');
    test(pgc_buf_get_char(&buf) == 't');
    test(pgc_buf_put(&buf, "dog", 3) == 0);
    test(pgc_buf_get_char(&buf) == 'd');
    test(pgc_buf_get_char(&buf) == 'o');
    test(pgc_buf_get_char(&buf) == 'g');
    test(pgc_buf_tell(&buf) == 6); 
    test(pgc_buf_put(&buf, "hi", 2) == 0);
    test(pgc_buf_get_char(&buf) == 'h');
    test(pgc_buf_get_char(&buf) == 'i');
    test(pgc_buf_tell(&buf) == 8);
    test(pgc_buf_put(&buf, "hat", 3) == 0);
    test(pgc_buf_put(&buf, " ", 1) == -1);
    test(pgc_buf_get_char(&buf) == 'h');
    test(pgc_buf_get_char(&buf) == 'a');
    test(pgc_buf_get_char(&buf) == 't');
}

void test_get(void) 
{
    puts("it writes binary data via get");
    char bytes[3];
    char tmp[3] = {0,0,0};
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    test(pgc_buf_put(&buf, "hi", 2) == 0);
    test(pgc_buf_get(&buf, tmp, 2) == 2);
    test(strcmp(tmp, "hi") == 0);
    test(pgc_buf_get(&buf, tmp, 2) == 0);
    test(pgc_buf_put(&buf, "go", 2) == 0);
    tmp[1] = 0; 
    test(pgc_buf_get(&buf, tmp, 1) == 1);
    test(strcmp(tmp, "g") == 0);
    test(pgc_buf_get(&buf, tmp, 2) == 1);
    test(strcmp(tmp, "o") == 0);
    test(pgc_buf_put(&buf, "n", 1) == 0);
    test(pgc_buf_put(&buf, "o", 1) == 0);
    test(pgc_buf_get(&buf, tmp, 3) == 2);
    test(strcmp(tmp, "no") == 0);
}

void test_match_str(void)
{
    puts("it matches strings by equality");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    test(pgc_buf_put(&buf, "hi", 2) == 0);
    test(!pgc_buf_match_str(&buf, "ha", 2));
    test(pgc_buf_match_str(&buf, "his", 3) == -1);
    test(pgc_buf_match_str(&buf, "hi", 2));
    test(pgc_buf_tell(&buf) == 2);
}

void test_match_char(void)
{
    puts("it matches char by set membership");
    char bytes[3];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 3);
    struct pgc_cset set;
    pgc_cset_from(&set, isalnum);
    test(pgc_buf_put(&buf, "h2%", 3) == 0);
    test(pgc_buf_match_char(&buf, &set)); 
    test(pgc_buf_match_char(&buf, &set));
    test(!pgc_buf_match_char(&buf, &set));
    test(pgc_buf_seek(&buf, 3) == 0);
    test(pgc_buf_match_char(&buf, &set) == -1);
}

void test_peek_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;

    uint32_t v = 0;

    char *sym1 = "A";   // 1 byte - 0x41
    puts("it decodes a single byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, sym1, 1) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 1);
    test(v == 0x41);
     
    char *sym2 = "£";   // 2 bytes - 0x00A3
    puts("it decodes a 2 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, sym2, 2) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 2);
    test(v == 0xA3);

    char *sym3 = "❮";   // 3 bytes - 0x276E
    puts("it decodes a 3 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, sym3, 3) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 3);
    test(v == 0x276E);

    char *sym4 = "𝐀";   // 4 bytes - 0x1D400 
    puts("it decodes a 4 byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, sym4, 4) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 4);
    test(v == 0x1D400);

    // 2-byte: Overlong encoding of 'A' (0x41)
    // Correct is 0x41. Improper is 0xC1 0x81.
    uint8_t bad_2b[] = {0xC1, 0x81}; 
    puts("it fails when passing an improperly encoded 2 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, bad_2b, 2) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);

    // 3-byte: Invalid continuation byte
    // Sequence starts like 3-byte, but 2nd byte isn't 10xxxxxx wants 0x80-0xBF
    uint8_t bad_3b[] = {0xE2, 0x20, 0xA2}; 
    puts("it fails when passing an improperly encoded 3 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, bad_3b, 3) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);

    // 4-byte: Out of Unicode range (> 0x10FFFF)
    // 0xF4 0xBF 0xBF 0xBF is valid, 0xF5 0x80 0x80 0x80 is out of bounds.
    uint8_t bad_4b[] = {0xF5, 0x80, 0x80, 0x80};
    puts("it fails when passing an improperly encoded 4 byte utf8");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, bad_4b, 4) == 0);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);

    // 3-byte: Surrogate U+D800
    // Pattern: 1110 1101, 10 100000, 10 000000
    uint8_t bad_surrogate[] = {0xED, 0xA0, 0x80}; 
    puts("it fails on UTF-16 surrogate range");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, bad_surrogate, 3);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);

    // Starts a 4-byte sequence but only provides 2 bytes
    uint8_t truncated_4b[] = {0xF0, 0x9F}; 
    puts("it handles truncated multi-byte sequences");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, truncated_4b, 2);
    test(pgc_buf_peek_utf8(&buf, &v) == -1);
    
    // A lone continuation byte (0x80)
    uint8_t lone_cont[] = {0x80}; 
    puts("it fails on a lone continuation byte");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, lone_cont, 1);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);

    // Overlong NUL (3-byte version of 0x00)
    uint8_t overlong_nul[] = {0xE0, 0x80, 0x80}; 
    puts("it fails on overlong encoded NUL");
    pgc_buf_init(&buf, bytes, 16);
    pgc_buf_put(&buf, overlong_nul, 3);
    test(pgc_buf_peek_utf8(&buf, &v) == 0);
}

void test_get_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;

    uint32_t v = 0;

    char *str1 = "£❮𝐀1";

    puts("it decodes a single byte utf8 character");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, str1, strlen(str1)) == 0);
    test(pgc_buf_get_utf8(&buf, &v) > 0);
    test(v == 0xA3);
    test(pgc_buf_get_utf8(&buf, &v) > 0);
    test(v == 0x276E);
    test(pgc_buf_get_utf8(&buf, &v) > 0);
    test(v == 0x1D400);
    test(pgc_buf_get_utf8(&buf, &v) == 1);
    test(v == '1');
    test(pgc_buf_get_utf8(&buf, &v) == -1);
}

bool utf8_pred(const uint32_t value, void *state)
{
    (void)state;
    if (value > 0xA3) return false;
    return true;
}

void test_match_utf8(void)
{
    char bytes[16];
    struct pgc_buf buf;

    char *str1 = "£1❮𝐀";

    puts("it can match utf8 characters with a predicate");
    pgc_buf_init(&buf, bytes, 16);
    test(pgc_buf_put(&buf, str1, strlen(str1)) == 0);
    test(pgc_buf_match_utf8(&buf, utf8_pred, NULL) > 0);
    test(pgc_buf_match_utf8(&buf, utf8_pred, NULL) > 0);
    test(pgc_buf_match_utf8(&buf, utf8_pred, NULL) == 0);
}

void test_request_advance(void)
{
    char bytes[4];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 4);
    puts("it can request a readable region and advance");
    test(pgc_buf_put(&buf, "12", 2) == 0);
    test(pgc_buf_get_char(&buf) == '1');
    char *win = pgc_buf_request(&buf, 3);
    test(win != NULL);
    win[0] = '3';
    win[1] = '4';
    win[2] = '5';
    pgc_buf_advance(&buf, 3);
    test(pgc_buf_get_char(&buf) == '2');
    test(pgc_buf_get_char(&buf) == '3');
    test(pgc_buf_get_char(&buf) == '4');
    test(pgc_buf_get_char(&buf) == '5');
}

void test_claim_consume(void)
{
    char bytes[4];
    struct pgc_buf buf;
    pgc_buf_init(&buf, bytes, 4);
    puts("it can claim a chunk and consume it");
    test(pgc_buf_put(&buf, "1234", 4) == 0);
    test(pgc_buf_get_char(&buf) == '1');
    char *win = pgc_buf_claim(&buf, 3);
    test(win != NULL);
    test(win[0] == '2');
    test(win[1] == '3');
    pgc_buf_consume(&buf, 2);
    test(pgc_buf_get_char(&buf) == '4');
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
