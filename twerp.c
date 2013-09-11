#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>

#include "twerp.h"






tw_ptr_t
tw_exec(tw_t *T, tw_ptr_t exp, tw_ptr_t env)
{

#define NEXT(OP) op = OP; continue;

  register tw_int_t op = OP_DISPATCH;
  int i;
  tw_err_t err;
  tw_ptr_t *car, *atmp, *btmp, *ctmp;

  T->Env = env;
  T->Exp = exp;

  push(T, T->Clink, OP_DONE);

  for (;;) {
    switch (op) {
    case OP_DISPATCH:
      if (T->Exp == T->NIL) {
        T->Val = T->NIL;
        NEXT(OP_POPJ_RET);
      }
      else if (ISSYM(T->Exp)) {
        /* lookup longjmps on error */
        T->Val = lookup(T, T->Exp);
        NEXT(OP_POPJ_RET);
      }
      else if (ISSELFEV(T->Exp)) {
        T->Val = T->Exp;
        NEXT(OP_POPJ_RET);
      }
      else if (ISCONS(T->Exp)) {
        car = tw_car(T->Exp);
        if (car == T->QUOTE) {
          T->Val = tw_cdr(T->Exp);
          NEXT(OP_POPJ_RET);
        }
        else if (car == T->IF) {
          tmp = tw_cdr(T->Exp);
          if (ISCONS(tmp)) {
            T->Exp = tw_car(T->Exp);
            push(T, T->Clink, T->Env);
            push(T, T->Clink, tw_cdr(tmp));
            push(T, T->Clink, OP_IF_DECIDE);
          }
          else {
            /* TODO: ERROR: malformed if */
          }

          NEXT(OP_OP_DISPATCH);
        }
        else if (car == T->FN || car == T->MAC) {
          tmp = tw_cdr(T->Exp);
          if (ISCONS(tmp)) {
            atmp = tw_car(tmp); /* params */
            if (ISCONS(atmp)) {
              btmp = tw_cdr(tmp);
              if (ISCONS(btmp)) {
                /* tw_mk_closure longjmps's on error */
                T->Val = tw_mk_closure(T, atmp, btmp, T->Env);
              }
              else {
                /* TODO: ERROR: malformed closure */
              }

              if (car == T->MAC) {
                MARK_MACRO(T->Val);
              }

              NEXT(OP_POPJ_RETURN);
            }
          }
        }
        else if (tw_cdr(T->Exp) == T->NIL) {
          push(T, T->Clink, OP_APPLY_NO_ARGS);
          T->Exp = tw_car(T->Exp);
        }
        else {
          push(T, T->Clink, T->Env);
          push(T, T->Clink, T->Exp);
          push(T, T->Clink, OP_ARGS);
          T->Exp = tw_car(T->Exp);
        }
        NEXT(OP_DISPATCH);
      }
    case OP_IF_DECIDE:
      T->Exp = pop(T, T->Clink);
      T->Env = pop(T, T->Clink);
      if (T->Val == T->NIL) {
        T->Exp = tw_cdr(T->Exp);
      }
      else {
        T->Exp = tw_car(T->Exp);
      }
      NEXT(OP_DISPATCH);
    case OP_APPLY_NO_ARGS:
      T->Args = T->NIL;
      NEXT(OP_APPLY);
    case OP_ARGS:
      /* Args are pushed in reverse order, which is sort of fine...
       * In all actuality, though, we'd rather zip the bindings
       * in the environment before apply.
       */
      T->Exp = pop(T, T->Clink);
      T->Env = pop(T, T->Clink);
      push(T, T->Clink, T->Val);
      T->Exp = tw_cdr(T->Exp);
      T->Args = T->NIL;
      NEXT(OP_ARGS_1);
    case OP_ARGS_1:
      if (tw_cdr(T->Exp) == T->NIL) {
        push(T, T->Clink, T->Args);
        push(T, T->Clink, OP_LAST_ARG);
      }
      else {
        push(T, T->Clink, T->Env);
        push(T, T->Clink, T->Exp);
        push(T, T->Clink, T->Args);
        push(T, T->Clink, OP_ARGS_2);
      }
      T->Exp = tw_car(T->Exp);
      NEXT(OP_DISPATCH);
    case OP_ARGS_2:
      T->Args = pop(T, T->Clink);
      T->Exp = pop(T, T->Clink);
      T->Env = pop(T, T->Clink);
      T->Args = tw_cons(T, T->Val, T->Args);
      T->Exp = tw_cdr(T->Exp);
      NEXT(OP_ARGS_1);
    case OP_LAST_ARG:
      T->Args = pop(T, T->Clink);
      T->Args = tw_cons(T, T->Val, T->Args);
      T->Val = pop(T, T->Clink);
      NEXT(OP_APPLY);
    case OP_APPLY:
      if (ISPRIM(T->Val)) {
        T->Val = tw_apply_prim(T, T->Val, T->Args);
        NEXT(OP_POPJ_RET);
      }
      else if (ISCLOS(T->Val)) {
        T->Env = extend(T, CDR(T->Val), CAR(CAR(T->Val)), T->Args);
        T->Exp = CDR(CAR(T->Val));
        NEXT(OP_DISPATCH);
      }
      /* TODO: Error: unable to apply this */
    case OP_POPJ_RET:
      T->Exp = pop(T, T->Clink);
      op = (tw_int_t) T->Exp;
    case OP_DONE:
    default:
      goto done;
    }
  }
 done:
  return T->Val;

#undef NEXT

}
