#include "sklisp.h"
#include<string.h>
#include<errno.h>
#include<assert.h>

// -*-
static Self _fn_docstr(Self self){
    SKL_DOC("Return doc-string for builtin functions or special forms.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("doc-str"));
    Self fun = SKL_CAR(self);
    bool evald = false;
    if(SKL_IS_SYMBOL(fun)){
        evald = true;
        fun = skl_eval(fun);
    }
    if(fun->kind != BUILTIN && fun->kind == SPECIAL){
        if(evald){ skl_delete(fun); }
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(fun));
    }
    Fun fn = SKL_VALUE_FUN(fun);
    Self doc = fn(sklisp.docstr);
    if(evald){
        skl_delete(fun);
    }
    return doc;
}

// -*-
static Self _fn_apply(Self self){
    SKL_DOC("Apply function to a list");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("apply"));
    Self fn = SKL_CAR(self);
    Self argv = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_LIST(argv)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(argv));
    }
    return skl_apply(fn, argv);
}

// -*-
static Self _fn_special_and(Self self){
    SKL_DOC("Evaluate each argument until one return nil.");
    Self result = sklisp.True;
    Self ptr = self;
    while(SKL_IS_CONS(ptr)){
        skl_delete(result);
        result = skl_eval(SKL_CAR(ptr));
        SKL_CHECK(result);
        if(result == sklisp.Nil){ return sklisp.Nil; }
        ptr = SKL_CDR(ptr);
    }
    if(ptr != sklisp.Nil){
        SKL_THROW(sklisp.InvalidListError, SKL_INC_RC(self));
    }
    return SKL_INC_RC(result);
}

// -*-
static Self _fn_special_or(Self self){
    SKL_DOC("Evalue each argument until one argument doesn't return nil.");
    Self result = sklisp.Nil;
    Self ptr = self;
    while(SKL_IS_CONS(ptr)){
        result = skl_eval(SKL_CAR(ptr));
        SKL_CHECK(result);
        if(result != sklisp.Nil){ return SKL_INC_RC(result); }
        ptr = SKL_CDR(ptr);
    }
    if(ptr != sklisp.Nil){
        SKL_THROW(sklisp.InvalidListError, SKL_INC_RC(self));
    }
    return sklisp.Nil;
}

// -*-
static Self _fn_cons(Self self){
    SKL_DOC("Construct a new cons cell, given car and cdr.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("cons"));
    return skl_new_cons(
        SKL_INC_RC(SKL_CAR(self)),
        SKL_INC_RC(SKL_CAR(SKL_CDR(self)))
    );
}

// -*-
static Self _fn_special_quote(Self self){
    SKL_DOC("Return argument unevaluated.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("quote"));
    return SKL_INC_RC(SKL_CAR(self));
}

// -*-
static Self _fn_special_lambda(Self self){
    SKL_DOC("create an anonymous function.");
    if(!skl_is_function_form(self)){
        SKL_THROW(skl_new_symbol("invalid-function-form"), SKL_INC_RC(self));
    }
    return skl_new_cons(sklisp.lambda, SKL_INC_RC(self));
}

// -*-
static Self _fn_special_defun(Self self){
    SKL_DOC("Define a new function");
    if(!SKL_IS_SYMBOL(SKL_CAR(self)) || !skl_is_function_form(SKL_CDR(self))){
        SKL_THROW(skl_new_symbol("invalid-function-form"), SKL_INC_RC(SKL_CDR(self)));
    }
    Self fn = skl_new_cons(sklisp.lambda, SKL_INC_RC(SKL_CDR(self)));
    SKL_SYMBOL_UPDATE(SKL_CAR(self), fn);
    return SKL_INC_RC(SKL_CAR(self));
}

// -*-
static Self _fn_special_defmacro(Self self){
    SKL_DOC("Define a new macro");
    if(!SKL_IS_SYMBOL(SKL_CAR(self)) || !skl_is_function_form(SKL_CDR(self))){
        SKL_THROW(skl_new_symbol("invalid-function-form"), SKL_INC_RC(self));
    }
    Self fn = skl_new_cons(sklisp.macro, SKL_INC_RC(SKL_CDR(self)));
    SKL_SYMBOL_UPDATE(SKL_CAR(self), fn);
    return SKL_INC_RC(SKL_CAR(self));
}

// -*-
static Self _fn_cdr(Self self){
    SKL_DOC("Return cdr element of cons cell.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("cdr"));
    if(SKL_CAR(self)==sklisp.Nil){
        return sklisp.Nil;
    }
    if(!SKL_IS_LIST(SKL_CAR(self))){
        SKL_THROW(sklisp.TypeError, SKL_CAR(self));
    }
    return SKL_INC_RC(SKL_CDR(SKL_CAR(self)));
}

// -*-
static Self _fn_car(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_list(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_special_if(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_not(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_special_cond(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_special_progn(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_special_let(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_special_while(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_eval(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_eq(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_eql(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_hash(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_print(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_println(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_set(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_value(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_symbol_name(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_str_concat(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_str_len(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_nullp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_callablep(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_listp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_symbolp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_numberp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_stringp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_integerp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_floatp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vectorp(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_load(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_parse(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_throw(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_catch(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vec_set(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vec_get(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vec_len(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_new_vec(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vec_concat(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_vec_slice(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_refcount(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_eval_depth(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_max_eval_depth(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_exit(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_list_len(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_len(Self self){
    //! @todo
    return NULL;
}

// -*-
static Self _fn_typeof(Self self){
    //! @todo
    return NULL;
}

// -*-
void skl_init_builtins(void){
    SKL_SYMBOL_SET(skl_new_symbol("docstr"), skl_new_fun(_fn_docstr));
    SKL_SYMBOL_SET(skl_new_symbol("apply"), skl_new_fun(_fn_apply));
    SKL_SYMBOL_SET(skl_new_symbol("and"), skl_new_special(_fn_special_and));
    SKL_SYMBOL_SET(skl_new_symbol("or"), skl_new_special(_fn_special_or));
    SKL_SYMBOL_SET(skl_new_symbol("quote"), skl_new_special(_fn_special_quote));
    SKL_SYMBOL_SET(skl_new_symbol("lambda"), skl_new_special(_fn_special_lambda));
    SKL_SYMBOL_SET(skl_new_symbol("defun"), skl_new_special(_fn_special_defun));
    SKL_SYMBOL_SET(skl_new_symbol("doefmacro"), skl_new_special(_fn_special_defmacro));
    SKL_SYMBOL_SET(skl_new_symbol("car"), skl_new_fun(_fn_car));
    SKL_SYMBOL_SET(skl_new_symbol("cdr"), skl_new_fun(_fn_cdr));
    SKL_SYMBOL_SET(skl_new_symbol("list"), skl_new_fun(_fn_list));
    SKL_SYMBOL_SET(skl_new_symbol("if"), skl_new_special(_fn_special_if));
    SKL_SYMBOL_SET(skl_new_symbol("not"), skl_new_fun(_fn_not));
    SKL_SYMBOL_SET(skl_new_symbol("progn"), skl_new_special(_fn_special_progn));
    SKL_SYMBOL_SET(skl_new_symbol("let"), skl_new_special(_fn_special_let));
    SKL_SYMBOL_SET(skl_new_symbol("while"), skl_new_special(_fn_special_while));
    SKL_SYMBOL_SET(skl_new_symbol("eval"), skl_new_fun(_fn_eval));
    SKL_SYMBOL_SET(skl_new_symbol("print"), skl_new_fun(_fn_print));
    SKL_SYMBOL_SET(skl_new_symbol("println"), skl_new_fun(_fn_println));
    SKL_SYMBOL_SET(skl_new_symbol("cons"), skl_new_fun(_fn_cons));

    SKL_SYMBOL_SET(skl_new_symbol("set"), skl_new_fun(_fn_set));
    SKL_SYMBOL_SET(skl_new_symbol("value"), skl_new_fun(_fn_value));
    SKL_SYMBOL_SET(skl_new_symbol("symbol-name"), skl_new_fun(_fn_symbol_name));

    SKL_SYMBOL_SET(skl_new_symbol("string-concat"), skl_new_fun(_fn_str_concat));
    SKL_SYMBOL_SET(skl_new_symbol("string-len"), skl_new_fun(_fn_str_len));

    SKL_SYMBOL_SET(skl_new_symbol("eq"), skl_new_fun(_fn_eq));
    SKL_SYMBOL_SET(skl_new_symbol("eql"), skl_new_fun(_fn_eql));
    SKL_SYMBOL_SET(skl_new_symbol("hash"), skl_new_fun(_fn_hash));

    SKL_SYMBOL_SET(skl_new_symbol("null?"), skl_new_fun(_fn_nullp));
    SKL_SYMBOL_SET(skl_new_symbol("nullp"), skl_new_fun(_fn_nullp));
    SKL_SYMBOL_SET(skl_new_symbol("list?"), skl_new_fun(_fn_listp));
    SKL_SYMBOL_SET(skl_new_symbol("listp"), skl_new_fun(_fn_listp));
    SKL_SYMBOL_SET(skl_new_symbol("callable?"), skl_new_fun(_fn_callablep));
    SKL_SYMBOL_SET(skl_new_symbol("callablep"), skl_new_fun(_fn_callablep));
    SKL_SYMBOL_SET(skl_new_symbol("symbol?"), skl_new_fun(_fn_symbolp));
    SKL_SYMBOL_SET(skl_new_symbol("symbolp"), skl_new_fun(_fn_symbolp));
    SKL_SYMBOL_SET(skl_new_symbol("string?"), skl_new_fun(_fn_stringp));
    SKL_SYMBOL_SET(skl_new_symbol("stringp"), skl_new_fun(_fn_stringp));
    SKL_SYMBOL_SET(skl_new_symbol("number?"), skl_new_fun(_fn_numberp));
    SKL_SYMBOL_SET(skl_new_symbol("numberp"), skl_new_fun(_fn_numberp));
    SKL_SYMBOL_SET(skl_new_symbol("integer?"), skl_new_fun(_fn_integerp));
    SKL_SYMBOL_SET(skl_new_symbol("integerp"), skl_new_fun(_fn_integerp));
    SKL_SYMBOL_SET(skl_new_symbol("float?"), skl_new_fun(_fn_floatp));
    SKL_SYMBOL_SET(skl_new_symbol("floatp"), skl_new_fun(_fn_floatp));
    SKL_SYMBOL_SET(skl_new_symbol("vector?"), skl_new_fun(_fn_vectorp));
    SKL_SYMBOL_SET(skl_new_symbol("vectorp"), skl_new_fun(_fn_vectorp));

    SKL_SYMBOL_SET(skl_new_symbol("load"), skl_new_fun(_fn_load));
    SKL_SYMBOL_SET(skl_new_symbol("parse"), skl_new_fun(_fn_parse));

    SKL_SYMBOL_SET(skl_new_symbol("throw"), skl_new_fun(_fn_throw));
    SKL_SYMBOL_SET(skl_new_symbol("catch"), skl_new_fun(_fn_catch));

    SKL_SYMBOL_SET(skl_new_symbol("vector-set"), skl_new_fun(_fn_vec_set));
    SKL_SYMBOL_SET(skl_new_symbol("vector-get"), skl_new_fun(_fn_vec_get));
    SKL_SYMBOL_SET(skl_new_symbol("vector-len"), skl_new_fun(_fn_vec_len));
    SKL_SYMBOL_SET(skl_new_symbol("new-vector"), skl_new_fun(_fn_new_vec));
    SKL_SYMBOL_SET(skl_new_symbol("vector-concat"), skl_new_fun(_fn_vec_concat));
    SKL_SYMBOL_SET(skl_new_symbol("vector-slice"), skl_new_fun(_fn_vec_slice));

    SKL_SYMBOL_SET(skl_new_symbol("refcount"), skl_new_fun(_fn_refcount));
    SKL_SYMBOL_SET(skl_new_symbol("eval-depth"), skl_new_fun(_fn_eval_depth));
    SKL_SYMBOL_SET(skl_new_symbol("max-eval-depth"), skl_new_fun(_fn_max_eval_depth));

    SKL_SYMBOL_SET(skl_new_symbol("exit"), skl_new_fun(_fn_exit));

    SKL_SYMBOL_SET(skl_new_symbol("list-len"), skl_new_fun(_fn_list_len));
    SKL_SYMBOL_SET(skl_new_symbol("len"), skl_new_fun(_fn_len));
    SKL_SYMBOL_SET(skl_new_symbol("typeof"), skl_new_fun(_fn_typeof));

    SKL_SYMBOL_SET(skl_new_symbol("new-channel"), skl_new_fun(skl_new_channel));
    SKL_SYMBOL_SET(skl_new_symbol("channel-recv"), skl_new_fun(skl_channel_receive));
    SKL_SYMBOL_SET(skl_new_symbol("channel-send"), skl_new_fun(skl_channel_send));

}