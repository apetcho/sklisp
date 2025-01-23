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
    }else if(op==ADD || op==SUB || op==MUL){
        const char* ops[] = {
            [ADD] = "add",
            [SUB] = "sub",
            [MUL] = "mul",
        };
        SKL_EXPECT_MIN_LEN(self, 1, skl_new_symbol(ops[op]));
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
    SKL_DOC("Perform negation or substraction operation.");
    return _arithm(SUB, self);
}

// -*-
static Self _fn_div(Self self){
    SKL_DOC("Perform division operation.");
    return _arithm(DIV, self);
}

// -*-
static Self _num_cmp(CmpOp op, Self self){
    // EQ, LT, LE, GT, GE
    const char* ops[] = {
        [EQ] = "eq",
        [LT] = "lt",
        [LE] = "le",
        [GT] = "gt",
        [GE] = "ge",
    };
    SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol(ops[op]));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_NUMBER(lhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(lhs));
    }
    if(!SKL_IS_NUMBER(rhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(rhs));
    }
    int result = 0;
    int invr = 1;
    if(SKL_IS_NUMBER(lhs) && SKL_IS_INTEGER(rhs)){
        result = mpz_cmp(SKL_DEREF_INTEGER(lhs), SKL_DEREF_INTEGER(rhs));
    }else if(SKL_IS_FLOAT(lhs) && SKL_IS_FLOAT(rhs)){
        result = mpf_cmp(SKL_DEREF_FLOAT(lhs), SKL_DEREF_FLOAT(rhs));
    }else if(SKL_IS_INTEGER(lhs) && SKL_IS_FLOAT(rhs)){
        // swap and handle below
        Self tmp = rhs;
        rhs = lhs;
        lhs = tmp;
        invr = -1;
    }
    if(SKL_IS_FLOAT(lhs) && SKL_IS_INTEGER(rhs)){
        // convert down
        Self val = skl_new_float(0.0);
        mpf_set_z(SKL_DEREF_FLOAT(val), SKL_DEREF_INTEGER(rhs));
        result = mpf_cmp(SKL_DEREF_FLOAT(lhs),SKL_DEREF_FLOAT(val));
        skl_delete(val);
    }
    result *= invr;
    switch(op){
    case EQ:
        result = (0 == result);
        break;
    case LT:
        result = (result < 0);
        break;
    case LE:
        result = (result <= 0);
        break;
    case GT:
        result = (result > 0);
        break;
    case GE:
        result = (result >= 0);
        break;
    }
    return result ? sklisp.True : sklisp.Nil;
}

// -*-
static Self _fn_num_eq(Self self){
    SKL_DOC("Compare two numbers by =.");
    return _num_cmp(EQ, self);
}

// -*-
static Self _fn_num_lt(Self self){
    SKL_DOC("Compare two numbers by <.");
    return _num_cmp(LT, self);
}

// -*-
static Self _fn_num_le(Self self){
    SKL_DOC("Compare two numbers by <=.");
    return _num_cmp(LE, self);
}

// -*-
static Self _fn_num_gt(Self self){
    SKL_DOC("Compare two numbers by >.");
    return _num_cmp(GT, self);
}

// -*-
static Self _fn_num_ge(Self self){
    SKL_DOC("Compare two numbers by >=.");
    return _num_cmp(GE, self);
}

// -*-
static Self _fn_mod(Self self){
    SKL_DOC("Return modulo of arguments");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("%"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_NUMBER(lhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(lhs));
    }
    if(!SKL_IS_NUMBER(rhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(lhs));
    }
    if(SKL_IS_FLOAT(lhs) || SKL_IS_FLOAT(rhs)){
        double x = skl_to_float(lhs);
        double y = skl_to_float(rhs);
        x = fmod(x, y);
        return skl_new_float(x);
    }
    long x = skl_to_integer(lhs);
    long y = skl_to_integer(rhs);
    Self result = skl_new_integer((x % y));
    return result;
}

// -*-
static Self _fn_abs(Self self){
    SKL_DOC("Returns the absolute value of argument");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("abs"));
    Self arg = SKL_CAR(self);
    if(!SKL_IS_NUMBER(arg)){
        SKL_THROW(sklisp.TypeError, arg);
    }
    Self result = NULL;
    if(SKL_IS_INTEGER(arg)){
        long x = skl_to_integer(arg);
        result = skl_new_integer((x < 0 ? -x : x));
        // SKL_INC_RC(result);
        return result;
    }
    double x = skl_to_float(arg);
    result = skl_new_float((x < 0 ? -x : x));
    // SKL_INC_RC(result);
    return result;
}

// -*-
static Self _fn_max(Self self){
    SKL_DOC("Returns the maximum value of two or more numbers");
    SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol("max"));
    Self arg = SKL_CAR(self);
    if(!SKL_IS_NUMBER(arg)){
        SKL_THROW(sklisp.TypeError, arg);
    }
    struct Num{
        bool iflag;
        union{
            long inum;
            double fnum;
        };
    }val = {.iflag=true, .inum=0};
    if(SKL_IS_INTEGER(arg)){
        val.inum = skl_to_integer(arg);
        val.iflag = true;
    }else{
        val.fnum = skl_to_float(arg);
        val.iflag = false;
    }
    self = SKL_CDR(self);
    while(self != sklisp.Nil){
        arg = SKL_CAR(self);
        if(!SKL_IS_NUMBER(arg)){
            SKL_THROW(sklisp.TypeError, arg);
        }
        if(SKL_IS_NUMBER(arg)){
            long x = skl_to_integer(arg);
            if(val.iflag){
                val.inum = (val.inum < x) ? x : val.inum;
                val.iflag = true;
            }else{
                val.iflag = false;
                val.fnum = (val.fnum < x) ? (double)x : val.fnum;
            }
        }else{
            double x = skl_to_float(arg);
            val.iflag = false;
            val.fnum = (val.fnum < x) ? x : val.fnum;
        }
        self = SKL_CDR(self);
    }
    Self result = NULL;
    if(val.iflag){
        result = skl_new_integer(val.inum);
    }else{
        result = skl_new_float(val.fnum);
    }
    // SKL_INC_RC(result);
    return result;
}

// -*-
static Self _fn_min(Self self){
    SKL_DOC("Returns the minimum value of two or more numbers");
    SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol("min"));
    Self arg = SKL_CAR(self);
    if(!SKL_IS_NUMBER(arg)){
        SKL_THROW(sklisp.TypeError, arg);
    }
    struct Num{
        bool iflag;
        union{
            long inum;
            double fnum;
        };
    }val = {.iflag=true, .inum=0};
    if(SKL_IS_INTEGER(arg)){
        val.inum = skl_to_integer(arg);
        val.iflag = true;
    }else{
        val.fnum = skl_to_float(arg);
        val.iflag = false;
    }
    self = SKL_CDR(self);
    while(self != sklisp.Nil){
        arg = SKL_CAR(self);
        if(!SKL_IS_NUMBER(arg)){
            SKL_THROW(sklisp.TypeError, arg);
        }
        if(SKL_IS_NUMBER(arg)){
            long x = skl_to_integer(arg);
            if(val.iflag){
                val.inum = (val.inum > x) ? x : val.inum;
                val.iflag = true;
            }else{
                val.iflag = false;
                val.fnum = (val.fnum > x) ? (double)x : val.fnum;
            }
        }else{
            double x = skl_to_float(arg);
            val.iflag = false;
            val.fnum = (val.fnum > x) ? x : val.fnum;
        }
        self = SKL_CDR(self);
    }
    Self result = NULL;
    if(val.iflag){
        result = skl_new_integer(val.inum);
    }else{
        result = skl_new_float(val.fnum);
    }
    // SKL_INC_RC(result);
    return result;
}

// -*-
static Self _fn_isnan(Self self){
    SKL_DOC("Checks whether a number is NaN");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("is-nan"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_integer((long)isnan(x));
}

// -*-
static Self _fn_isinf(Self self){
    SKL_DOC("Checks whether a number is Infinity");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("is-inf"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_integer((long)isinf(x));
}

// -*-
static Self _fn_isfinite(Self self){
    SKL_DOC("Checks whether a number is finite");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("is-finite"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_integer((long)isfinite(x));
}

// -*-
static Self _fn_exp(Self self){
    SKL_DOC("Computes e^x");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("e^x"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(exp(x));
}

// -*-
static Self _fn_exp2(Self self){
    SKL_DOC("Computes 2^x");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("2^x"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(exp2(x));
}

// -*-
static Self _fn_expm1(Self self){
    SKL_DOC("Computes e^x - 1");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("e^x - 1"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(expm1(x));
}

// -*-
static Self _fn_log(Self self){
    SKL_DOC("Computes natural (base-e) logarithm (ln x)");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("(ln x)"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= 0){
        SKL_THROW(sklisp.ValueError, self);
    }
    return skl_new_float(log(x));
}

// -*-
static Self _fn_log10(Self self){
    SKL_DOC("Computes common (base-10) logarithm");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("(log10 x)"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= 0){ SKL_THROW(sklisp.ValueError, self); }
    return skl_new_float(log10(x));
}

// -*-
static Self _fn_log2(Self self){
    SKL_DOC("Computes base-2 logarithm");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("(log2 x)"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= 0){ SKL_THROW(sklisp.ValueError, self); }
    return skl_new_float(log2(x));
}

// -*-
static Self _fn_log1p(Self self){
    SKL_DOC("Computes natural (base-e) logarithm of 1 plus given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("ln(1+x)"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= -1){ SKL_THROW(sklisp.ValueError, self); }
    return skl_new_float(log1p(x));
}

// -*-
static Self _fn_pow(Self self){
    SKL_DOC("Computes a number raised to the given power");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("(x^y)"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_NUMBER(lhs)){
        SKL_THROW(sklisp.TypeError, lhs);
    }
    if(!SKL_IS_NUMBER(rhs)){
        SKL_THROW(sklisp.TypeError, rhs);
    }
    double x = skl_to_float(lhs);
    double y = skl_to_float(rhs);
    return skl_new_float(pow(x, y));
}

// -*-
static Self _fn_sqrt(Self self){
    SKL_DOC("Computes square root of a number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("square-root"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= 0){ SKL_THROW(sklisp.ValueError, self); }
    return skl_new_float(sqrt(x));
}

// -*-
static Self _fn_cbrt(Self self){
    SKL_DOC("Computes cube root");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("(cube-root)"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    if(x <= 0){ SKL_THROW(sklisp.ValueError, self); }
    return skl_new_float(cbrt(x));
}

// -*-
static Self _fn_hypot(Self self){
    SKL_DOC("Computes square root of the sum of the squares of two given numbers");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("(sqrt(x^2 + y^2))"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_NUMBER(lhs)){
        SKL_THROW(sklisp.TypeError, lhs);
    }
    if(!SKL_IS_NUMBER(rhs)){
        SKL_THROW(sklisp.TypeError, rhs);
    }
    double x = skl_to_float(lhs);
    double y = skl_to_float(rhs);
    return skl_new_float(hypot(x, y));
}

// -*-
static Self _fn_sin(Self self){
    SKL_DOC("Computes sine of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("sine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(sin(x));
}

// -*-
static Self _fn_cos(Self self){
    SKL_DOC("Computes cosine of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("cosine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(cos(x));
}

// -*-
static Self _fn_tan(Self self){
    SKL_DOC("Computes tangent of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("tangent"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(tan(x));
}

// -*-
static Self _fn_asin(Self self){
    SKL_DOC("Computes arc sine of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("arc-sine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(asin(x));
}

// -*-
static Self _fn_acos(Self self){
    SKL_DOC("Computes arc cosine of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("arc-cosine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(acos(x));
}

// -*-
static Self _fn_atan(Self self){
    SKL_DOC("Computes arc tangent of a given number");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("arc-tangent"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(atan(x));
}

// -*-
static Self _fn_atan2(Self self){
    SKL_DOC("Computes arc tangent, using signs to determine quadrants");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("arc-tangent2"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_NUMBER(lhs)){
        SKL_THROW(sklisp.TypeError, lhs);
    }
    if(!SKL_IS_NUMBER(rhs)){ SKL_THROW(sklisp.TypeError, rhs); }
    double x = skl_to_float(lhs);
    double y = skl_to_float(rhs);
    return skl_new_float(atan2(x, y));
}

// -*-
static Self _fn_sinh(Self self){
    SKL_DOC("Computes hyperbolic sine");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("hyperbolic-sine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(sinh(x));
}

// -*-
static Self _fn_cosh(Self self){
    SKL_DOC("Computes hyperbolic cosine");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("hyperbolic-cosine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(cosh(x));
}

// -*-
static Self _fn_tanh(Self self){
    SKL_DOC("Computes hyperbolic tangent");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("hyperbolic-tangent"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(tanh(x));
}

// -*-
static Self _fn_asinh(Self self){
    SKL_DOC("Computes inverse hyperbolic sine");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("inverse-hyperbolic-sine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(asinh(x));
}

// -*-
static Self _fn_acosh(Self self){
    SKL_DOC("Computes inverse hyperbolic cosine");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("inverse-hyperbolic-cosine"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(acosh(x));
}

// -*-
static Self _fn_atanh(Self self){
    SKL_DOC("Computes inverse hyperbolic tangent");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("inverse-hyperbolic-tangent"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(atanh(x));
}

// -*-
static Self _fn_erf(Self self){
    SKL_DOC("Computes error function");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("error-function"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(erf(x));
}

// -*-
static Self _fn_erfc(Self self){
    SKL_DOC("Computes complementary error function");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("complementary-error-function"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(erfc(x));
}

// -*-
static Self _fn_tgamma(Self self){
    SKL_DOC("Computes gamma function");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("gamma-function"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(tgamma(x));
}

// -*-
static Self _fn_lgamma(Self self){
    SKL_DOC("Compute natural (base-e) logarithm of the gamma function");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("lgamma-function"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_float(lgamma(x));
}

// -*-
static Self _fn_ceil(Self self){
    SKL_DOC("Compute smallest integer not less than given value");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("*ceil*"));
    self = SKL_CAR(self);
    if(!SKL_IS_NUMBER(self)){
        SKL_THROW(sklisp.TypeError, self);
    }
    double x = skl_to_float(self);
    return skl_new_integer(ceil(x));
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