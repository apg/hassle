#ifndef _TWERP_H_
#define _TWERP_H_

#ifdef __LP64__
typedef uint64_t tw_ptr_t;
typedef int64_t tw_int_t;
#else
typedef uint32_t tw_ptr_t;
typedef int32_t tw_int_t;
#endif

#define TAGBITS 5

#define ISINT(x) ((x & 1) == 0)
#define ISSYM(x) ((x & 30) == TW_SYM)
#define ISCONS(x) ((x & 30) == TW_CONS)
#define ISSTR(x) ((x & 30) == TW_STR)
#define ISREAL(x) ((x & 30) == TW_REAL)
#define ISVEC(x) ((x & 30) == TW_VEC)
#define ISCLOS(x) ((x & 30) == TW_CLOS)
#define ISMAC(x) ((x & 30) == TW_MAC)
#define ISPRIM(x) ((x & 30) == TW_PRIM)
#define ISDATA(x) ((x & 30) == TW_DATA)

#define INTOF(x) (tw_int_t)(x >> 1)
#define PTROF(x) (void *)((x) & ~(tw_int_t)0x3)
#define CONSOF(x) ((tw_cell_t *)PTROF(x))->cons
#define STROF(x) ((tw_cell_t *)PTROF(x))->str
#define REALOF(x) ((tw_cell_t *)PTROF(x))->real
#define PRIMOF(x) ((tw_cell_t *)PTROF(x))->prim
#define DATAOF(x) ((tw_cell_t *)PTROF(x))->data

/* Pointer types have tags that end in 1
   Integers store a 0 at the least significant bit */
typedef enum types {
  TW_INT=0, TW_SYM=3, TW_CONS=5, TW_STR=7, TW_REAL=9,
  TW_CLOS=11, TW_MAC=13, TW_PRIM=15, TW_USR=17
} tw_type_t;

typedef struct cons {
  tw_ptr_t car;
  tw_ptr_t cdr;
} tw_cons_t;

typedef struct str {
  tw_int_t len;
  char *data;
} tw_str_t;

typedef struct prim {
  tw_int_t arity;
  tw_ptr_t (*func)(tw_t *, tw_ptr_t args);
} tw_prim_t;

typedef struct data {
  tw_int_t size;
  void *data;
  void (*finalizer)(tw_t *, tw_data_t *);
  void (*visit)(tw_t *, tw_data_t *);
} tw_data_t;

typedef struct cell {
  tw_int_t flags;
  union {
    double flonum;
    tw_cons_t cons;
    tw_str_t str;
    tw_prim_t prim;
    tw_data_t data;
  }
} tw_cell_t;

struct symtab_t {
  tw_int_t size;
  tw_int_t i;
  tw_cell_t **cells;
};

/* make them odd so they look like tw_int_t's */
typedef enum tw_opcode {
  OP_DISPATCH=1, OP_IF_DECIDE=3, OP_APPLY_NO_ARGS=5, OP_ARGS=7,
  OP_ARGS_1=9, OP_ARGS_2=11, OP_LAST_ARG=13, OP_APPLY=15, 
  OP_POPJ_RET=17, OP_DONE=19
} tw_opcode_t;

typedef struct tw_context {
  tw_int_t Clink_size;
  tw_int_t Clink_sp;
  tw_ptr_t *Clink; /* pointer only because it's an array */
  tw_ptr_t Args;
  tw_ptr_t Exp;
  tw_ptr_t Env;
  struct symtab_t *Symtab;
  tw_ptr_t NIL;
  tw_ptr_t QUOTE;
  tw_ptr_t IF;
  tw_ptr_t FN;
  tw_ptr_t MAC;
} tw_t;

#endif
