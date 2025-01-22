#include "sklisp.h"
#include<math.h>
#include<tgmath.h>
#include<limits.h>
#include<float.h>


typedef enum { ADD, SUB, MUL, DIV } ArithOp;
typedef enum { EQ, LT, LE, GT, GE } CmpOp;

// -*-
static Self _arithm(ArithOp op, Self self){
    if(op==DIV){
        SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol("div"));
    }
    bool imode = true;
    Self ival = sklisp.Nil;
    Self fval = sklisp.Nil;
    Self val = skl_new_float(0.0);
    switch(op){
    case SUB:
    case ADD:
        ival = skl_new_integer(0);
        fval = skl_new_float(0.0);
        break;
    case MUL:
    case DIV:
        ival = skl_new_integer(1);
        fval = skl_new_float(1.0);
        break;
    }
    if(op==SUB || op == DIV){
        Self num = SKL_CAR(self);
        if(SKL_IS_FLOAT(num)){
            imode = false;
            mpf_set(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
        }else if(SKL_IS_INTEGER(num)){
            mpf_set_z(SKL_DEREF_FLOAT(ival), SKL_DEREF_INTEGER(num));
            mpz_set(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(num));
        }else{
            skl_delete(ival);
            skl_delete(fval);
            skl_delete(val);
            SKL_THROW(sklisp.TypeError, SKL_INC_RC(num));
        }

        if(op==SUB && SKL_CDR(self) == sklisp.Nil){
            if(imode){
                skl_delete(fval);
                skl_delete(val);
                mpz_neg(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(ival));
                return ival;
            }else{
                skl_delete(ival);
                skl_delete(val);
                mpf_neg(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval));
                return fval;
            }
        }

        self = SKL_CDR(self);
    }

    while(self != sklisp.Nil){
        Self num = SKL_CAR(self);
        if(!SKL_IS_NUMBER(num)){
            skl_delete(ival);
            skl_delete(fval);
            skl_delete(val);
            SKL_THROW(sklisp.TypeError, SKL_INC_RC(num));
        }

        // check divide by zero
        if(op == DIV){
            double x = 0.0;
            if(SKL_IS_FLOAT(num)){ x = skl_to_float(num); }
            else{ x = skl_to_integer(num); }
            if(x == 0){
                skl_delete(ival);
                skl_delete(fval);
                skl_delete(val);
                SKL_THROW(skl_new_symbol("divide-by-zero"), SKL_INC_RC(num));
            }
        }
        if(imode){
            if(SKL_IS_FLOAT(num)){
                imode = false;
                mpf_set_z(SKL_DEREF_FLOAT(fval), SKL_DEREF_INTEGER(ival));
                // fall-though to int-mode (i.e imode)
            }else if(SKL_IS_INTEGER(num)){
                switch(op){
                case ADD:
                    mpz_add(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(SKL_CAR(self)));
                    break;
                case MUL:
                    mpz_mul(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(SKL_CAR(self)));
                    break;
                case SUB:
                    mpz_sub(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(SKL_CAR(self)));
                    break;
                case DIV:
                    mpz_div(SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(ival), SKL_DEREF_INTEGER(SKL_CAR(self)));
                    break;
                }
            }
        }
        if(!imode){
            if(SKL_IS_FLOAT(num)){
                switch(op){
                case ADD:
                    mpf_add(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case MUL:
                    mpf_mul(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case SUB:
                    mpf_sub(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case DIV:
                    mpf_div(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                }
            }else if(SKL_IS_INTEGER(num)){
                // convert to float and add.
                mpf_set_z(SKL_DEREF_FLOAT(val), SKL_DEREF_INTEGER(num));
                switch(op){
                case ADD:
                    mpf_add(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case MUL:
                    mpf_mul(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case SUB:
                    mpf_sub(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                case DIV:
                    mpf_div(SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(fval), SKL_DEREF_FLOAT(num));
                    break;
                }
            }
        }
        self = SKL_CDR(self);
    }
    skl_delete(val);

    // detroy whater went unused
    if(imode){
        skl_delete(fval);
        return ival;
    }
    skl_delete(ival);
    return fval;
}

// -*-
static Self _fn_add(Self self){
    SKL_DOC("Compute the sum of two or more numbers");
    return _arithm(ADD, self);
}

// -*-
static Self _fn_mul(Self self){
    SKL_DOC("Compute the producte of two or more numbers");
    return _arithm(MUL, self);
}

// -*-
static Self _fn_sub(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_div(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _num_cmp(CmpOp op, Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_num_eq(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_num_lt(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_num_le(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_num_gt(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_num_ge(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_mod(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_abs(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_max(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_min(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_isnan(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_isinf(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_isfinite(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_exp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_exp2(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_expm1(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_log(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_log10(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_log2(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_log1p(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_pow(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_sqrt(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_cbrt(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_hypot(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_sin(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_cos(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_tan(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_asin(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_acos(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_atan(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_atan2(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_sinh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_cosh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_tanh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_asinh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_acosh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_atanh(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_erf(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_erfc(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_tgamma(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_lgamma(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_ceil(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_floor(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_trunc(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_round(Self self){
    //! @todo
    return NULL;
}

// -*-
void skl_init_math(void){
    //! @todo
    SKL_SYMBOL_SET(skl_new_symbol("PI"), skl_new_float(3.141592653589793));
    SKL_SYMBOL_SET(skl_new_symbol("E"), skl_new_float(2.718281828459045));
    SKL_SYMBOL_SET(skl_new_symbol("Int::MIN"), skl_new_integer(LONG_MIN));
    SKL_SYMBOL_SET(skl_new_symbol("Int::MAX"), skl_new_integer(LONG_MAX));
    SKL_SYMBOL_SET(skl_new_symbol("Float::MIN"), skl_new_float(DBL_MIN));
    SKL_SYMBOL_SET(skl_new_symbol("Float::MAX"), skl_new_float(DBL_MAX));
    SKL_SYMBOL_SET(skl_new_symbol("Float::EPSILON"), skl_new_float(DBL_EPSILON));

    SKL_SYMBOL_SET(skl_new_symbol("+"), skl_new_fun(_fn_add));
    SKL_SYMBOL_SET(skl_new_symbol("-"), skl_new_fun(_fn_sub));
    SKL_SYMBOL_SET(skl_new_symbol("*"), skl_new_fun(_fn_mul));
    SKL_SYMBOL_SET(skl_new_symbol("/"), skl_new_fun(_fn_div));
    SKL_SYMBOL_SET(skl_new_symbol("%"), skl_new_fun(_fn_mod));
    SKL_SYMBOL_SET(skl_new_symbol("="), skl_new_fun(_fn_num_eq));
    SKL_SYMBOL_SET(skl_new_symbol("<"), skl_new_fun(_fn_num_lt));
    SKL_SYMBOL_SET(skl_new_symbol(">"), skl_new_fun(_fn_num_gt));
    SKL_SYMBOL_SET(skl_new_symbol("<="), skl_new_fun(_fn_num_le));
    SKL_SYMBOL_SET(skl_new_symbol(">="), skl_new_fun(_fn_num_ge));

    SKL_SYMBOL_SET(skl_new_symbol("abs"), skl_new_fun(_fn_abs));
    SKL_SYMBOL_SET(skl_new_symbol("max"), skl_new_fun(_fn_max));
    SKL_SYMBOL_SET(skl_new_symbol("min"), skl_new_fun(_fn_min));
    SKL_SYMBOL_SET(skl_new_symbol("nan?"), skl_new_fun(_fn_isnan));
    SKL_SYMBOL_SET(skl_new_symbol("inf?"), skl_new_fun(_fn_isinf));
    SKL_SYMBOL_SET(skl_new_symbol("finite?"), skl_new_fun(_fn_isfinite));
    SKL_SYMBOL_SET(skl_new_symbol("exp"), skl_new_fun(_fn_exp));
    SKL_SYMBOL_SET(skl_new_symbol("exp2"), skl_new_fun(_fn_exp2));
    SKL_SYMBOL_SET(skl_new_symbol("expm1"), skl_new_fun(_fn_expm1));
    SKL_SYMBOL_SET(skl_new_symbol("log"), skl_new_fun(_fn_log));
    SKL_SYMBOL_SET(skl_new_symbol("ln"), skl_new_fun(_fn_log));
    SKL_SYMBOL_SET(skl_new_symbol("log10"), skl_new_fun(_fn_log10));
    SKL_SYMBOL_SET(skl_new_symbol("log2"), skl_new_fun(_fn_log2));
    SKL_SYMBOL_SET(skl_new_symbol("pow"), skl_new_fun(_fn_pow));
    SKL_SYMBOL_SET(skl_new_symbol("sqrt"), skl_new_fun(_fn_sqrt));
    SKL_SYMBOL_SET(skl_new_symbol("cbrt"), skl_new_fun(_fn_cbrt));
    SKL_SYMBOL_SET(skl_new_symbol("hypot"), skl_new_fun(_fn_hypot));
    SKL_SYMBOL_SET(skl_new_symbol("sin"), skl_new_fun(_fn_sin));
    SKL_SYMBOL_SET(skl_new_symbol("cos"), skl_new_fun(_fn_cos));
    SKL_SYMBOL_SET(skl_new_symbol("tan"), skl_new_fun(_fn_tan));
    SKL_SYMBOL_SET(skl_new_symbol("asin"), skl_new_fun(_fn_asin));
    SKL_SYMBOL_SET(skl_new_symbol("acos"), skl_new_fun(_fn_acos));
    SKL_SYMBOL_SET(skl_new_symbol("atan"), skl_new_fun(_fn_atan));
    SKL_SYMBOL_SET(skl_new_symbol("atan2"), skl_new_fun(_fn_atan2));
    SKL_SYMBOL_SET(skl_new_symbol("sinh"), skl_new_fun(_fn_sinh));
    SKL_SYMBOL_SET(skl_new_symbol("cosh"), skl_new_fun(_fn_cosh));
    SKL_SYMBOL_SET(skl_new_symbol("tanh"), skl_new_fun(_fn_tanh));
    SKL_SYMBOL_SET(skl_new_symbol("asinh"), skl_new_fun(_fn_asinh));
    SKL_SYMBOL_SET(skl_new_symbol("acosh"), skl_new_fun(_fn_acosh));
    SKL_SYMBOL_SET(skl_new_symbol("atanh"), skl_new_fun(_fn_atanh));
    SKL_SYMBOL_SET(skl_new_symbol("erf"), skl_new_fun(_fn_erf));
    SKL_SYMBOL_SET(skl_new_symbol("erfc"), skl_new_fun(_fn_erfc));
    SKL_SYMBOL_SET(skl_new_symbol("tgamma"), skl_new_fun(_fn_tgamma));
    SKL_SYMBOL_SET(skl_new_symbol("lgamma"), skl_new_fun(_fn_lgamma));
    SKL_SYMBOL_SET(skl_new_symbol("ceil"), skl_new_fun(_fn_ceil));
    SKL_SYMBOL_SET(skl_new_symbol("floor"), skl_new_fun(_fn_floor));
    SKL_SYMBOL_SET(skl_new_symbol("trunc"), skl_new_fun(_fn_trunc));
    SKL_SYMBOL_SET(skl_new_symbol("round"), skl_new_fun(_fn_round));
}