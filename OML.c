// ordinate manipulation language

#include <stdio.h>      /* for printf */
#include <stdlib.h>     /* for malloc, realloc */
#include <inttypes.h>   /* for int64_t */
#include <string.h>     /* for memcpy */
#include <unistd.h>     /* for write */
#include <time.h>       /* for time */
#include <limits.h>     /* for UINT_MAX */
#include <math.h>       /* for log, pow */
#include <ctype.h>      /* for isalpha, isalnum, etc. */
#include <stdbool.h>    /* for true, false, bool */

#include "twister.h" /* for randomMT */
#include "msdelay.h" /* for ms_delay */

#define INITIAL_STACK_CAPACITY (16)
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#ifdef __x86_64__
    #define INT64_TO_PTR(x)(STACK*)x
#else
    #define INT64_TO_PTR(x)(STACK*)(int)x
#endif
// #define printf(...) printf("\x1b[33m[%s::%i]\x1b[0m ", __FUNCTION__, __LINE__);printf(__VA_ARGS__)

typedef struct STACK {
    size_t capacity, size;
    int64_t* data;
} STACK;

char* read_file(char* name, size_t* out_size) {
    FILE* file = fopen(name, "r");
    if(!file) {
        fprintf(stderr, "Error: no such file %s\n", name);
        return NULL;
    }
    size_t length;
    char* contents;
    *out_size = 0;
    // printf("[debug] start\n");
    if(fseeko(file, 0, SEEK_END) != 0) {
        // printf("[debug<seek_end] failed\n");
        return NULL;
    }
    // printf("[debug] finding length");
    length = ftello(file);
    if(fseeko(file, 0, SEEK_SET) != 0) {
        // printf("[debug<seek_set] failed\n");
        return NULL;
    }
    // printf("[debug] %"PRId64"\n", length);
    contents = malloc(length * sizeof(char));
    fread(contents, sizeof(char), length, file);
    fclose(file);
    *out_size = length;
    return contents;
}

STACK stack_init(void) {
    STACK res = { INITIAL_STACK_CAPACITY, 0, NULL };
    res.data = malloc(sizeof(int64_t) * res.capacity);
    // if(!res.data)
        // return NULL;
    return res;
}

void stack_destroy(STACK* stk) {
    free(stk->data);
}

int stack_resize(STACK* stk) {
    void* temp = realloc(stk->data, sizeof(int64_t) * stk->capacity);
    if(!temp) {
        return 0;
    }
    stk->data = temp;
    return 1;
}

int stack_push(STACK* stk, int64_t val) {
    stk->data[stk->size++] = val;
    if(stk->size >= stk->capacity) {
        stk->capacity *= 2;
        return stack_resize(stk);
    }
    return 1;
}

int stack_unshift(STACK* stk, int64_t val) {
    stk->size++;
    if(stk->size >= stk->capacity) {
        stk->capacity *= 2;
        if(!stack_resize(stk)) {
            return 0;
        }
    }
    memmove(stk->data + 1, stk->data, stk->size * sizeof(int64_t));
    stk->data[0] = val;
    return 1;
}

int64_t stack_pop(STACK* stk) {
    if(stk->size == 0)
        return 0;
    
    int64_t res = stk->data[--stk->size];
    
    return res;
}

int64_t stack_shift(STACK* stk) {
    int64_t val = stk->data[0];
    stk->size--;
    memmove(stk->data, stk->data + 1, stk->size * sizeof(int64_t));
    return val;
}

int64_t stack_pop_from(STACK* stk, size_t index) {
    int64_t val = stk->data[index];
    stk->size--;
    size_t to_move = stk->size - index;
    memmove(stk->data + index, stk->data + index + 1, to_move * sizeof(int64_t));    
    return val;
}

int64_t stack_peek(STACK* stk) {
    if(stk->size == 0)
        return 0;
    return stk->data[stk->size - 1];
}

void stack_display(STACK t) {
    fflush(stdout);
    for(size_t i = t.size - 1; i < t.size; --i) {
        printf("%"PRId64"\n", t.data[i]);
    }
}

void stack_push_int_array(STACK* stk, int64_t* arr, size_t size) {
    for(size_t i = 0; i < size; i++) {
        stack_push(stk, arr[i]);
    }
}

void stack_clear(STACK* stk) {
    stk->size = 0;
}

bool stdin_remaining() {
    ungetc(getchar(), stdin);
    return feof(stdin) == 0;
}

int64_t random_between(int64_t lower, int64_t upper) {
    double scale = randomMT() * 1.0 / UINT_MAX;
    return (int64_t)(scale * (upper - lower)) + lower;
}

int64_t factorial(int64_t n) {
    if(n < 0) {
        return 0;
    }
    else if(n <= 1) {
        return 1;
    }
    else {
        int64_t prod = n;
        while(n --> 1) {
            prod *= n;
        }
        return prod;
    }
}

// modified from https://stackoverflow.com/a/6641357/4119004
int64_t icbrt(int64_t n) {
    if(n < 0)
        return -icbrt(-n);
    int64_t s;
    uint64_t x, y, b, y2;
    
    x = n;
    y2 = 0;
    y = 0;
    for(s = 30; s >= 0; s = s - 3) {
        y2 = 4*y2;
        y = 2*y;
        b = (3*(y2 + y) + 1) << s;
        if (x >= b) {
            x = x - b;
            y2 = y2 + 2*y + 1;
            y = y + 1;
        }
    }
    return y;
}

// from https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_.28base_2.29
int64_t isqrt(int64_t num) {
    int64_t res = 0;
    int64_t bit = 1 << 30; // The second-to-top bit is set
 
    // "bit" starts at the highest power of four <= the argument.
    while (bit > num)
        bit >>= 2;
        
    while (bit != 0) {
        if (num >= res + bit) {
            num -= res + bit;
            res = (res >> 1) + bit;
        }
        else
            res >>= 1;
        bit >>= 2;
    }
    return res;
}

int OUTPUT_BASE = 10;
int INPUT_BASE = 10;
char ALPHABET[] = "0123456789abcdefghijklmnopqrstuvwxyz";

int is_power(int64_t num, int64_t base) {
    while(num % base == 0)
        num /= base;
    return num == 1;
}

int64_t* to_base(int64_t n, int64_t base, size_t* out_size) {
    double ratio = round(log(n) / log(base));
    // printf("%"PRId64" / %"PRId64"\n", n, base);

    *out_size = 1 + ratio;
    // *out_size = 1 + (log(n + 1) / log(base));
    size_t j = *out_size;
    int64_t* temp = malloc(j * sizeof(int64_t));
    while(n > 0) {
        temp[--j] = n % base;
        n /= base;
    }
    if(j) {
        *out_size -= j;
        memmove(temp, temp + j, *out_size * sizeof(int64_t));
    }
    return temp;
}

#define to_output_base(a, b) to_base(a, OUTPUT_BASE, b)

void print_int(int64_t n) {
    fflush(stdout);
    if(n < 0) {
        putchar('-');
        fflush(stdout);
        print_int(-n);
    } 
    else if(n == 0) {
        printf("0");
    }
    else if(n == OUTPUT_BASE) {
        printf("10");
    }
    else if(OUTPUT_BASE == 10) {
        printf("%"PRId64, n);
    }
    else if(OUTPUT_BASE == 16) {
        printf("%"PRIx64, n);
    }
    else if(OUTPUT_BASE == 1) {
        fprintf(stderr, "unary isn't really a base...\n");
        int64_t t = n;
        char* temp = malloc(n * sizeof(char));
        while(t --> 0) {
            temp[t] = '1';
        }
        write(1, temp, n);
        free(temp);
    }
    else if(OUTPUT_BASE <= 36) {
        // while(
        //todo:base conversion
        size_t digit_count;
        int64_t* digits = to_output_base(n, &digit_count);
        char* temp = malloc(digit_count * sizeof(char));
        for(size_t i = 0; i < digit_count; i++) {
            temp[i] = ALPHABET[digits[i]];
        }
        write(1, temp, digit_count);
        free(temp);
        free(digits);
    }
    else {
        printf("output in base %i: %"PRId64, OUTPUT_BASE, n); 
    }
    fflush(stdout);
}

int is_valid_in_char(int c) {
    if(INPUT_BASE <= 10)
        return '0' <= c && c < ALPHABET[INPUT_BASE];
    else
        return '0' <= c && (c <= '9' || c < ALPHABET[INPUT_BASE]);
}

int char_to_in_digit(int c) {
    return c <= '9' ? c - '0' : c - ALPHABET[10] + 10;
}
int64_t input_int(void) {
    char temp = getchar();
    ungetc(temp, stdin);
    int64_t ret = 0;
    if(INPUT_BASE == 10) {
        scanf(" %"SCNd64, &ret);
    }
    else {
        // fixed width is fine, since numbers can only be so long
        char number_str[80] = {0};
        char* format = malloc(14 * sizeof(char));
        int sign = -1;
        char dummy_sign;
        if(INPUT_BASE < 9) {
            strcpy(format, " %[-]%[0-?]");
            format[9] = ALPHABET[INPUT_BASE-1];
            // puts(format);
        }
        else {
            strcpy(format, " %[-]%[0-9a-?A-?]");
            format[12] = ALPHABET[INPUT_BASE-1];
            format[15] = toupper(ALPHABET[INPUT_BASE-1]);
        }
                
        if(scanf(format, &dummy_sign, &number_str) != 2) {
            // printf("dummy sign = '%c'/%i\n", dummy_sign, dummy_sign);
            sign = 1;
            format += 4;
            *format = ' ';
            scanf(format, &number_str);
            format -= 4;
        }
        for(int pos = 0; number_str[pos]; pos++) {
            ret *= INPUT_BASE;
            ret += char_to_in_digit(number_str[pos]);
        }
        ret *= sign;
        free(format);
    }
    return ret;
}

typedef struct OML {
    STACK stk;
    STACK stk_stk;
    STACK reg_stk[256];
    int64_t vars[256];
    char* code;
    size_t i, size, sub_stk_size;
} OML;

void OML_exec_cmd(OML* inst, char cur) {
    STACK* res = &inst->stk;
    if(cur == ' ') {
        // no-op
    }
    else if(cur == '!') {
        int64_t a = stack_pop(res);
        stack_push(res, factorial(a));
    }
    else if(cur == '"') {
        inst->i++;
        size_t start = inst->i;
        while(inst->i < inst->size) {
            if(inst->code[inst->i] == '"') {
                if(inst->i + 1 >= inst->size
                || inst->code[inst->i + 1] != '"') {
                    break;
                }
            }
            inst->i++;
        }
        size_t end = inst->i;
        for(size_t j = end - 1; j >= start; --j) {
            stack_push(res, inst->code[j]);
        }
        stack_push(res, end - start);
        // write(1, inst->code + start, inst->i - start);
    }
    else if(cur == '#') {
        print_int(stack_pop(res));
    }
    else if(cur == '$') {
        stack_pop(res);
    }
    else if(cur == '%') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a % b);
    }
    else if(cur == '&') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a & b);
    }
    else if(cur == '\'') {
        char c = inst->code[++inst->i];
        stack_push(res, (int64_t)c);
    }
    else if(cur == '(') {
        // if not tos, go to next )
        if(!stack_peek(res)) {
            int depth = 1;
            while(depth) {
                inst->i++;
                if(inst->code[inst->i] == '(') {
                    depth++;
                }
                else if(inst->code[inst->i] == ')') {
                    depth--;
                }
            }
        }
    }
    else if(cur == ')') {
        // if tos, go to previous (
        if(stack_peek(res)) {
            int depth = 1;
            while(depth) {
                inst->i--;
                if(inst->code[inst->i] == '(') {
                    depth--;
                }
                else if(inst->code[inst->i] == ')') {
                    depth++;
                }
            }
        }
    }
    else if(cur == '*') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a * b);
    }
    else if(cur == '+') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a + b);
    }
    else if(cur == ',') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, b);
        stack_push(res, a);
    }
    else if(cur == '-') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a - b);
    }
    else if(cur == '.') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a / b);
        stack_push(res, a % b);
    }
    else if(cur == '/') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a / b);
    }
    else if(cur >= '0' && cur <= '9') {
        stack_push(res, cur - '0');
    }
    else if(cur == ':') {
        int64_t a = stack_pop(res);
        stack_push(res, a);
        stack_push(res, a);
    }
    else if(cur == ';') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a);
        stack_push(res, b);
        stack_push(res, a);
    }
    else if(cur == '<') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a < b);
    }
    else if(cur == '=') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a == b);
    }
    else if(cur == '>') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a > b);
    }
    else if(cur == '?') {
        int64_t a = stack_pop(res);
        stack_push(res, random_between(0, a));
    }
    else if(cur == '@') {
        int64_t c = stack_pop(res);
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, c);
        stack_push(res, a);
        stack_push(res, b);
    }
    else if('A' <= cur && cur <= 'F') {
        stack_push(res, cur - 'A' + 10);
    }
    else if(cur == 'G') {
        stack_push(res, 64);
    }
    else if(cur == 'H') {
        stack_push(res, 256);
    }
    else if(cur == 'I') {
        stack_push(res, 100);
    }
    else if(cur == 'J') {
        stack_push(res, 1000);
    }
    else if(cur == 'K') {
        int64_t n = stack_pop(res);
        int64_t c = n;
        while(c --> 0) {
            stack_push(res, res->data[res->size - n]);
        }
    }
    else if(cur == 'L') {
        stack_clear(res);
    }
    else if(cur == 'M') {
        int64_t a = stack_pop(res);
        stack_push(res, icbrt(a));
    }
    else if(cur == 'N') {
        int64_t a = stack_pop(res);
        stack_push(res, isqrt(a));
    }
    else if(cur == 'O') {
        stack_display(*res);
    }
    else if(cur == 'P') {
        INPUT_BASE = stack_pop(res);
    }
    else if(cur == 'Q') {
        OUTPUT_BASE = stack_pop(res);
    }
    else if(cur == 'R') {
        int64_t count = stack_pop(res);
        STACK temp = stack_init();
        for(int64_t i = 0; i < count; i++) {
            stack_push(&temp, stack_pop(res));
        }
        for(int64_t i = 0; i < count; i++) {
            stack_push(res, temp.data[i]);
        }
        stack_destroy(&temp);
    }
    else if(cur == 'S') {
        stack_push(res, 16);
    }
    // from https://stackoverflow.com/a/12700533/4119004
    else if(cur == 'T') {
        int64_t x, y, pow;
        y = stack_pop(res);
        x = stack_pop(res);
        pow = 10;
        while(y >= pow)
            pow *= 10;
        stack_push(res, x * pow + y);
    }
    else if(cur == 'U') {
        int64_t n = stack_pop(res);
        size_t bit_size;
        int64_t* bits = to_base(n, 2, &bit_size);
        stack_push_int_array(res, bits, bit_size);
        free(bits);
    }
    else if(cur == 'V') {
        int64_t n, *digits;
        size_t digit_count;
        n = stack_pop(res);
        digits = to_output_base(n, &digit_count);
        for(size_t i = 0; i < digit_count; i++) {
            stack_push(res, digits[i]);
        }
        free(digits);
    }
    else if(cur == 'W') {
        int64_t stream, count, i;
        stream = stack_pop(res);
        count = i = stack_pop(res);
        char* temp = malloc(count * sizeof(char));
        while(i --> 0) {
            temp[i] = stack_pop(res);
        }
        write(stream, temp, count);
        free(temp);
    }
    else if(cur == 'X') {
        int64_t x = stack_pop(res);
        stack_push(res, x);
        stack_push(res, x);
        stack_push(res, x);
    }
    else if(cur == 'Y') {
        int64_t n = stack_pop(res);
        for(int64_t i = 0; i < n; i++) {
            stack_push(res, i);
        }
    }
    else if(cur == 'Z') {
        int64_t top = stack_pop(res);
        stack_unshift(res, top);
    }
    else if(cur == '[') {
        // inst->stk_stk
        int64_t count = stack_pop(res);
        size_t size = res->size;
        for(int64_t i = 0; i < size - count; i++) {
            stack_push(&inst->stk_stk, stack_shift(res));
        }
        stack_push(&inst->stk_stk, size - count);
        inst->sub_stk_size++;
    }
    else if(cur == '\\') {
        STACK temp = stack_init();
        while(res->size) {
            stack_push(&temp, stack_pop(res));
        }
        while(temp.size) {
            stack_unshift(res, stack_pop(&temp));
        }
        stack_destroy(&temp);
    }
    else if(cur == ']') {
        int64_t count = stack_pop(&inst->stk_stk);
        while(count --> 0) {
            stack_unshift(res, stack_pop(&inst->stk_stk));
        }
        inst->sub_stk_size--;
    }
    else if(cur == '^') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a ^ b);
    }
    else if(cur == '^') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a ^ b);
    }
    else if(cur == '_') {
        int64_t a = stack_pop(res);
        stack_push(res, -a);
    }
    else if(cur == 'a') {
        int64_t k = stack_pop(res);
        int64_t n = stack_pop(res);
        stack_push(res, n ^ (1ull << k));
    }
    else if(cur == 'b') {
        int64_t index = stack_pop(res);
        index = res->size - index - 1;
        stack_push(res, res->data[index]);
    }
    else if(cur == 'c') {
        int64_t index = stack_pop(res);
        index = res->size - index - 1;
        int64_t entry = stack_pop_from(res, index);
        stack_push(res, entry);
    }
    else if(cur == 'd') {
        int64_t top = stack_pop(res);
        res->size = 0;
        stack_push(res, top);
    }
    else if(cur == 'f') {
        unsigned char ident = inst->code[++inst->i];
        inst->vars[ident] = stack_pop(res);
    }
    else if(cur == 'g') {
        unsigned char ident = inst->code[++inst->i];
        stack_push(res, inst->vars[ident]);
    }
    else if(cur == 'h') {
        stack_push(res, input_int());
    }
    // read line
    else if(cur == 'i') {
        int c = 1;
        size_t size = 0;
        while((c = getchar()) != 10 && c != EOF) {
            stack_push(res, c);
            size++;
        }
        if(c == 10) {
            size++;
            stack_push(res, c);
        }
        stack_push(res, size);
        OML_exec_cmd(inst, 'R');
        stack_push(res, size);
    }
    else if(cur == 'j') {
        stack_push(res, getchar());
    }
    
    else if(cur == 'l') {
        stack_push(res, res->size);
    }
    
    else if(cur == 'm') {
        int64_t n = stack_pop(res);
        stack_push(res, n * n * n);
    }
    
    else if(cur == 'n') {
        int64_t n = stack_pop(res);
        stack_push(res, n * n);
    }
    
    else if(cur == 'o') {
        int64_t a = stack_pop(res);
        putchar((char) a);
    }
    
    else if(cur == 'p') {
        stack_push(res, INPUT_BASE);
    }
    
    else if(cur == 'q') {
        stack_push(res, OUTPUT_BASE);
    }
    
    else if(cur == 'r') {
        stack_push(res, inst->sub_stk_size);
    }
    
    else if(cur == 's') {
        size_t size = stack_pop(res);
        char* str = malloc(size * sizeof(char));
        for(size_t i = 0; i < size; i++) {
            str[i] = (char)stack_pop(res);
        }
        fflush(stdout);
        write(1, str, size);
        free(str);
    }
    
    else if(cur == 't') {
        unsigned char ident = inst->code[++inst->i];
        STACK* reg = &inst->reg_stk[ident];
        stack_push(reg, stack_pop(res));
    }
    
    else if(cur == 'u') {
        unsigned char ident = inst->code[++inst->i];
        STACK* reg = &inst->reg_stk[ident];
        stack_push(res, stack_pop(reg));
    }
    
    else if(cur == 'x') {
        int64_t repeater = stack_pop(res);
        int64_t repetend = stack_pop(res);
        while(repeater --> 0) {
            stack_push(res, repetend);
        }
    }
    
    else if(cur == 'z') {
        int64_t bot = stack_shift(res);
        stack_push(res, bot);
    }
    
    else if(cur == '{') {
        if(!stack_pop(res)) {
            int depth = 1;
            while(depth) {
                inst->i++;
                if(inst->code[inst->i] == '{') {
                    depth++;
                }
                else if(inst->code[inst->i] == '}') {
                    depth--;
                }
            }
        }
    }
    else if(cur == '|') {
        int64_t b = stack_pop(res);
        int64_t a = stack_pop(res);
        stack_push(res, a | b);
    }
    
    else if(cur == '~') {
        int64_t a = stack_pop(res);
        stack_push(res, ~a);
    }
    
    // extended function6
    else if(cur == 'e') {
        unsigned char ident = inst->code[++inst->i];
        if(ident == '!') {
            int64_t a = stack_pop(res);
            stack_push(res, !a);
        }
        else if(ident == '#') {
            int64_t a = stack_pop(res);
            print_int(a);
            puts("");
        }
        else if(ident == '<') {
            int64_t b = stack_pop(res);
            int64_t a = stack_pop(res);
            stack_push(res, a >= b);
        }
        else if(ident == '=') {
            int64_t b = stack_pop(res);
            int64_t a = stack_pop(res);
            stack_push(res, a != b);
        }
        else if(ident == '>') {
            int64_t b = stack_pop(res);
            int64_t a = stack_pop(res);
            stack_push(res, a <= b);
        }
        else if(ident == '\\') {
            while(inst->i < inst->size && inst->code[inst->i] != '\n') {
                inst->i++;
            }
        }
        else if(ident == 'A') {
            int64_t n = stack_pop(res);
            stack_push(res, isalpha(n) != 0);
        }
        else if(ident == 'C') {
            int64_t n = stack_pop(res);
            stack_push(res, toupper(n));
        }
        else if(ident == 'c') {
            int64_t n = stack_pop(res);
            stack_push(res, tolower(n));
        }
        else if(ident == 'e') {
            // set read flag as a test
            stack_push(res, stdin_remaining());
        }
        else if(ident == 'i') {
            while(stdin_remaining()) {
                stack_push(res, input_int());
            }
            OML_exec_cmd(inst, '\\');
        }
        else if(ident == 'm') {
            STACK* addr = malloc(sizeof(addr));
            *addr = stack_init();
            stack_push(res, (int)addr);
        }
        else if(ident == 'n') {
            int64_t n = stack_pop(res);
            STACK* tmp = INT64_TO_PTR(stack_pop(res));
            for(int64_t c = n; c > 0; --c) {
                stack_push(tmp, res->data[res->size - c]);
            }
            for(int64_t c = n; c > 0; --c) {
                stack_pop(res);
            }
            stack_push(res, (int) tmp);
        }
        else if(ident == 'o') {
            STACK* tmp = INT64_TO_PTR(stack_peek(res));
            stack_display(*tmp);
        }
        else if(ident == 'p') {
            int64_t n = stack_pop(res);
            STACK* tmp = (STACK*)(int) stack_pop(res);
            stack_push(res, (int) tmp);
            stack_push(tmp, n);
        }
        else if(ident == 'q') {
            STACK* tmp = (STACK*)(int) stack_pop(res);
            int64_t n = stack_pop(tmp);
            stack_push(res, n);
            stack_push(res, (int) tmp);
        }
        else if(ident == '~') {
            exit(stack_pop(res));
        }
    }
}

OML OML_init(char* str, size_t size) {
    OML inst;
    inst.stk = stack_init();
    inst.stk_stk = stack_init();
    inst.code = str;
    inst.i = 0;
    inst.size = size;
    inst.sub_stk_size = 0;
    for(int i = 0; i < 256; i++) {
        inst.reg_stk[i] = stack_init();
    }
    return inst;
}

void OML_run(OML* inst) {
    while(inst->i < inst->size) {
        char cur = inst->code[inst->i];
        OML_exec_cmd(inst, cur);
        inst->i++;
    }
    inst->i = 0;
}

OML OML_exec(char* str, size_t size) {
    OML inst = OML_init(str, size);
    OML_run(&inst);
    return inst;
}

#define COLOR_RESET  "\x1b[0m"
#define COLOR_HEADER(x) "\x1b[33m" x COLOR_RESET
#define COLOR_CODE(x) "\x1b[1;34m" x COLOR_RESET

void show_help(char* file_name) {
    eprintf("[[ OML - Ordinal Manipulation Language ]]\n");
    eprintf(COLOR_HEADER("== Usage ==\n"));
    eprintf(COLOR_CODE("%s [args] <code>\n"), file_name);
    eprintf(COLOR_HEADER("== Arguments ==\n"));
    eprintf("  -?   show this help page\n");
    eprintf("  -b   treat the input base as binary initially\n");
    eprintf("  -f   read program from file `<code>' instead\n");
    eprintf("  -h   treat the input base as hexadecimal initially\n");
    eprintf("  -n   execute the program over the numbers of stdin\n");
    eprintf(COLOR_HEADER("== About ==\n"));
    eprintf("OML is a language similar to dc with its primary data type being the integer.\n");
    eprintf("Like in dc, all numbers are stored on the `stack', to which integers are added\n");
    eprintf("or removed. OML is a tool that can perform arithmetic calculations and generic\n");
    eprintf("algorithms, with some effort. While esoteric in nature, it still can provide\n");
    eprintf("succinct utilities.\n");
    eprintf(COLOR_HEADER("== Examples ==\n"));
    eprintf("Convert hexadecimal to decimal: "COLOR_CODE("%s -hn")"\n", file_name);
    eprintf("Convert hexadecimal to binary: "COLOR_CODE("%s -hn 2Q")"\n", file_name);
    eprintf("N-th fibonacci: "COLOR_CODE("%s '01h(Z:@+z1-)\\d'")"\n", file_name);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        eprintf("Error: insufficient arguments passed to %s.", argv[0]);
        return 1;
    }
    char* prog = "";
    size_t prog_len;
    bool from_file = false, over_numbers = false;
    for(int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if(arg[0] == '-') {
            arg++;
            while(*arg) {
                if(*arg == 'f')
                    from_file = true;
                else if(*arg == 'n')
                    over_numbers = true;
                else if(*arg == 'h')
                    INPUT_BASE = 16;
                else if(*arg == 'b')
                    INPUT_BASE = 2;
                else if(*arg == '?') {
                    show_help(argv[0]);
                    return -1;
                }
                arg++;
            }
        }
        else {
            prog = arg;
        }
    }
    if(from_file) {
        prog = read_file(prog, &prog_len);
    }
    else {
        prog_len = strlen(prog);
    }
    srand(ms_delay());
    seedMT(rand());
    OML res;
    if(over_numbers) {
        res = OML_init(prog, prog_len);
        while(!feof(stdin)) {
            int64_t n = input_int();
            stack_push(&res.stk, n);
            OML_run(&res);
            print_int(stack_pop(&res.stk));
            putchar('\n');
            stack_clear(&res.stk);
        }
    }
    else {
        res = OML_exec(prog, prog_len);
        stack_display(res.stk);
    }
}
