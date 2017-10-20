#ifndef OML_INCL
#define OML_INCL
#include <inttypes.h>   /* for int64_t */
#include <stdbool.h>    /* for true, false, bool */

/* colors */
#define COLOR_RESET     "\x1b[0m"
#define COLOR_HEADER(x) "\x1b[33m" x COLOR_RESET
#define COLOR_SUB_HEADER(x) "\x1b[35m" x COLOR_RESET
#define COLOR_CODE(x)   "\x1b[1;34m" x COLOR_RESET

typedef struct STACK {
    size_t capacity, size;
    int64_t* data;
} STACK;

typedef struct OML {
    STACK stk;
    STACK stk_stk;
    STACK reg_stk[256];
    int64_t vars[256];
    char* code;
    size_t i, size, sub_stk_size;
} OML;

int     OUTPUT_BASE = 10;
int     INPUT_BASE  = 10;
char    ALPHABET[]  = "0123456789abcdefghijklmnopqrstuvwxyz";

/* stack methods */
STACK   stack_init              (void);
STACK   stack_from              (STACK);
int     stack_push              (STACK*, int64_t);
int     stack_resize            (STACK*);
int     stack_unshift           (STACK*, int64_t);
void    stack_display           (STACK);
void    stack_clear             (STACK*);
void    stack_destory           (STACK*);
void    stack_push_int_array    (STACK*, int64_t*, size_t);
int64_t stack_pop               (STACK*);
int64_t stack_shift             (STACK*);
int64_t stack_pop_from          (STACK*, size_t);
int64_t stack_peek              (STACK*);

/* generic function */
void    show_help       (char*);
bool    stdin_remaining (void);
char*   read_file       (char*, size_t*);

#define to_output_base(a, b) to_base(a, OUTPUT_BASE, b)
double      random_scale    (void);
int         is_power        (int64_t, int64_t);
int64_t     ipow            (int64_t, int64_t);
int64_t     icbrt           (int64_t);
int64_t     isqrt           (int64_t);
int64_t     factorial       (int64_t);
int64_t     random_between  (int64_t, int64_t);
int64_t*    to_base         (int64_t, int64_t, size_t*);

int     is_valid_in_char    (int);
int     char_to_in_digit    (int);
void    print_int           (int64_t);
double  fpart               (double);
int64_t input_int           (void);

/* OML functions */
OML     OML_init            (char*, size_t);
OML     OML_exec            (char*, size_t);
void    OML_run             (OML*);
void    OML_diagnostic      (OML*);
void    OML_exec_cmd        (OML*, char);
void    OML_exec_str_stk    (OML*, char*, STACK);
void    OML_exec_str_args   (OML*, char*, size_t, ...);
void    OML_exec_str        (OML*, char*);
#endif