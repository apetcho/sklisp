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
    SKL_UNUSED(result);
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
    SKL_DOC("Return car element of cons cell.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("car"));
    if(SKL_CAR(self) == sklisp.Nil){ return sklisp.Nil; }
    if(!SKL_IS_LIST(SKL_CAR(self))){
        SKL_THROW(sklisp.TypeError, SKL_CAR(self));
    }
    return SKL_INC_RC(SKL_CAR(SKL_CAR(self)));
}

// -*-
static Self _fn_list(Self self){
    SKL_DOC("Return arguments as a list.");
    return SKL_INC_RC(self);
}

// -*-
static Self _fn_special_if(Self self){
    SKL_DOC("If conditional special form.");
    SKL_EXPECT_MIN_LEN(self, 2, sklisp.InvalidArgNumberError);
    Self cond = skl_eval(SKL_CAR(self));
    if(cond != sklisp.Nil){
        skl_delete(cond);
        return skl_eval(SKL_CAR(SKL_CDR(self)));
    }
    return skl_eval_body(SKL_CDR(SKL_CDR(self)));
}

// -*-
static Self _fn_special_cond(Self self){
    SKL_DOC("Eval car of each argument until one is true. The eval cdr of that argument.");
    Self ptr = self;
    while(ptr != sklisp.Nil){
        if(!SKL_IS_CONS(ptr)){
            SKL_THROW(sklisp.InvalidListError, SKL_INC_RC(self));
        }
        Self pair = SKL_CAR(ptr);
        if(!SKL_IS_CONS(pair)){
            SKL_THROW(sklisp.TypeError, SKL_INC_RC(pair));
        }
        if(!SKL_IS_LIST(SKL_CDR(pair))){
            SKL_THROW(sklisp.InvalidListError, SKL_INC_RC(pair));
        }
        if(SKL_CDR(pair)==sklisp.Nil){
            return SKL_INC_RC(SKL_CAR(pair));
        }
        if(SKL_CDR(SKL_CDR(pair)) != sklisp.Nil){
            SKL_THROW(skl_new_symbol("invalid-form"), SKL_INC_RC(pair));
        }
        Self test = skl_eval(SKL_CAR(pair));
        if(test != sklisp.Nil){
            skl_delete(test);
            return skl_eval(SKL_CAR(SKL_CDR(pair)));
        }
        ptr = SKL_CDR(ptr);
    }
    return sklisp.Nil;
}

// -*-
static Self _fn_special_progn(Self self){
    SKL_DOC("Eval each argument and return the eval of the last");
    return skl_eval_body(self);
}

// -*-
static Self _fn_special_let(Self self){
    SKL_DOC("Create a variable bindings in a new scope, and eval body in that scope.");

    // verify structure
    if(!SKL_IS_LIST(SKL_CAR(self))){
        SKL_THROW(skl_new_symbol("invalid-let-form"), SKL_INC_RC(self));
    }

    Self vars = SKL_CAR(self);
    while(vars != sklisp.Nil){
        Self entry = SKL_CAR(vars);
        if(!SKL_IS_LIST(entry)){
            SKL_THROW(skl_new_symbol("invalid-let-form"), SKL_INC_RC(self));
        }
        if(!SKL_IS_SYMBOL(SKL_CAR(entry))){
            SKL_THROW(skl_new_symbol("invalid-let-form"), SKL_INC_RC(self));
        }
        vars = SKL_CDR(vars);
    }
    Self ptr = NULL;
    ptr = vars = SKL_CAR(self);
    int len = 0;
    while(ptr != sklisp.Nil){
        Self pair = SKL_CAR(ptr);
        Self val = skl_eval(SKL_CAR(SKL_CDR(pair)));
        if(val == sklisp.Error){
            // undo scoping
            ptr = vars;
            while(len){
                skl_symtab_pop(SKL_CAR(SKL_CAR(ptr)));
                ptr = SKL_CDR(ptr);
                len--;
            }
            return sklisp.Error;
        }
        skl_symtab_push(SKL_CAR(pair), val);
        skl_delete(val);
        ptr = SKL_CDR(ptr);
        len++;
    }

    Self result = skl_eval_body(SKL_CDR(self));
    ptr = vars;
    while(ptr != sklisp.Nil){
        Self pair = SKL_CAR(ptr);
        skl_symtab_pop(SKL_CAR(pair));
        ptr = SKL_CDR(ptr);
    }
    return result;
}

// -*-
static Self _fn_special_while(Self self){
    SKL_DOC("Continually evaluate body until first argument evals nil.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("while"));
    Self result = sklisp.Nil;
    Self cond = SKL_CAR(self);
    Self body = SKL_CDR(self);
    Self condVal = NULL;
    while((condVal = skl_eval(cond)) != sklisp.Nil){
        skl_delete(result);
        skl_delete(condVal);
        SKL_CHECK(condVal);
        result = skl_eval_body(body);
    }
    return result;
}

// -*-
static Self _fn_eval(Self self){
    return skl_eval_body(self);
}

// -*-
static Self _fn_eq(Self self){
    SKL_DOC("Return t if both arguments are the same lisp object.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("eq"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(lhs == rhs){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_eql(Self self){
    SKL_DOC("Return t if both arguments are similiar.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("eql"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(lhs->kind != rhs->kind){ return sklisp.Nil; }
    switch(lhs->kind){
    case INTEGER:
    case FLOAT:
        return skl_num_equal(self);
    case SYMBOL:
    case CONS:
        if(lhs == rhs){ return sklisp.True; }
        break;
    case STRING:
        if(SKL_STRING_LEN(lhs)==SKL_STRING_LEN(rhs)){
            if(memcmp(SKL_STRING(lhs), SKL_STRING(rhs), SKL_STRING_LEN(lhs))==0){
                return sklisp.True;
            }
        }
        break;
    case VEC:
        return sklisp.Nil;
    case BUILTIN:
    case SPECIAL:
        if(SKL_VALUE_FUN(lhs)==SKL_VALUE_FUN(rhs)){ return sklisp.True; }
        break;
    default:
        break;
    }
    return sklisp.Nil;
}

// -*-
static Self _fn_hash(Self self){
    SKL_DOC("Return integer hash of object.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("hash"));
    return skl_new_integer(sklisp_hash(SKL_CAR(self)));
}

// -*-
static Self _fn_print(Self self){
    SKL_DOC("Print an object or sexp in parse-able form.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("print"));
    skl_print(SKL_CAR(self));
    return sklisp.Nil;
}

// -*-
static Self _fn_println(Self self){
    SKL_DOC("Print object or sexp in parse-able form. Print a newline after.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("println"));
    skl_println(SKL_CAR(self));
    return sklisp.Nil;
}

// -*-
static Self _fn_set(Self self){
    SKL_DOC("Store object in symbol.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("set"));
    if(!SKL_IS_SYMBOL(SKL_CAR(self))){
        SKL_THROW(sklisp.TypeError, skl_new_cons(skl_new_symbol("set"), SKL_CAR(self)));
    }
    if(SKL_SYMBOL_IS_CONSTANT(SKL_CAR(self))){
        SKL_THROW(skl_new_symbol("setting-constant"), SKL_CAR(self));
    }
    SKL_SYMBOL_UPDATE(SKL_CAR(self), SKL_CAR(SKL_CDR(self)));
    return SKL_INC_RC(SKL_CAR(SKL_CDR(self)));
}

// -*-
static Self _fn_value(Self self){
    SKL_DOC("Get value store in symbol");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("value"));
    if(!SKL_IS_SYMBOL(SKL_CAR(self))){
        SKL_THROW(sklisp.TypeError, skl_new_cons(skl_new_symbol("value"), SKL_CAR(self)));
    }
    return SKL_INC_RC(SKL_SYMBOL_GET(SKL_CAR(self)));
}

// -*-
static Self _fn_symbol_name(Self self){
    SKL_DOC("Return symbol name as string.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("symbol-name"));
    if(!SKL_IS_SYMBOL(SKL_CAR(self))){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(SKL_CAR(self)));
    }
    return skl_new_string(skl_strdup(SKL_SYMBOL(SKL_CAR(self))));
}

// -*-
static Self _fn_str_concat(Self self){
    SKL_DOC("Concatenate two strings.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("string-concat"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_STRING(lhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(lhs));
    }
    if(!SKL_IS_STRING(rhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(rhs));
    }
    return skl_string_cat(lhs, rhs);
}

// -*-
static Self _fn_str_len(Self self){
    SKL_DOC("Return the len of a given string");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("string-len"));
    self = SKL_CAR(self);
    if(!SKL_IS_STRING(self)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(self));
    }
    return skl_new_integer(SKL_STRING_LEN(self));
}

// -*-
static Self _fn_callablep(Self self){
    SKL_DOC("Return t if object is a callable (i.e a function, lambda, or macro)");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("callable?"));
    if(SKL_IS_CALLABLE(SKL_CAR(self))){
        return sklisp.True;
    }
    return sklisp.Nil;
}

// -*-
static Self _fn_nullp(Self self){
    SKL_DOC("Return t if object is nil.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("null?"));
    if(SKL_CAR(self)==sklisp.Nil){
        return sklisp.True;
    }
    return sklisp.Nil;
}

// -*-
static Self _fn_listp(Self self){
    SKL_DOC("Return t if object is a list");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("list?"));
    if(SKL_IS_LIST(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_symbolp(Self self){
    SKL_DOC("Return t if object is a symbol.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("symbol?"));
    if(SKL_IS_SYMBOL(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_numberp(Self self){
    SKL_DOC("Return t if object is a number.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("number?"));
    if(SKL_IS_NUMBER(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_stringp(Self self){
    SKL_DOC("Return t if object is a string.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("string?"));
    if(SKL_IS_STRING(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_integerp(Self self){
    SKL_DOC("Return t if object is a integer.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("integer?"));
    if(SKL_IS_INTEGER(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_floatp(Self self){
    SKL_DOC("Return t if object is a floating-point number.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("float?"));
    if(SKL_IS_FLOAT(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_vectorp(Self self){
    SKL_DOC("Return t if object is a vector.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("vector?"));
    if(SKL_IS_VECTOR(SKL_CAR(self))){ return sklisp.True; }
    return sklisp.Nil;
}

// -*-
static Self _fn_load(Self self){
    SKL_DOC("Evaluate contents of a file.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("load"));
    Self fname = SKL_CAR(self);
    if(!SKL_IS_STRING(fname)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(fname));
    }
    char* filename = SKL_STRING(fname);
    int rc = stream_load_file(NULL, filename, 0);
    if(!rc){
        SKL_THROW(skl_new_symbol("load-file-error"), SKL_INC_RC(fname));
    }

    return sklisp.True;
}

// -*-
static Self _fn_parse(Self self){
    SKL_DOC("Parse a string into a sexp or list object.");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("parse"));
    Self obj = SKL_CAR(self);
    if(!SKL_IS_STRING(obj)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(obj));
    }
    char* str = SKL_STRING(obj);
    Stream* stream = new_stream(NULL, str, "parse", 0);
    Self sexp = stream_read_sexp(stream);
    delete_stream(stream);
    if(sexp == sklisp.Error){
        SKL_THROW(skl_new_symbol("parse-error"), SKL_INC_RC(obj));
    }
    return sexp;
}

// -*-
static Self _fn_throw(Self self){
    SKL_DOC("Throw an object, and attachment, as an exception.");
    SKL_THROW(SKL_INC_RC(SKL_CAR(self)), SKL_INC_RC(SKL_CAR(SKL_CDR(self))));
    return NULL;
}

// -*-
static Self _fn_catch(Self self){
    SKL_DOC("Catch an exception and return attachement");
    Self _except = skl_eval(SKL_CAR(self));
    SKL_CHECK(_except);
    Self body = SKL_CDR(self);
    Self result = skl_eval_body(body);
    if(result == sklisp.Error){
        if(_except==sklisp.ThrownError){
            skl_delete(_except);
            skl_delete(sklisp.ThrownError);
            return sklisp.AttachError;
        }else{
            return sklisp.Error;
        }
    }
    return result;
}

// -*-
static Self _fn_vec_set(Self self){
    SKL_DOC("Set slot in a vector to object.");
    SKL_EXPECT_LEN(self, 3, skl_new_symbol("vector-set"));
    Self vec = SKL_CAR(self);
    Self idx = SKL_CAR(SKL_CDR(self));
    Self val = SKL_CAR(SKL_CDR(SKL_CDR(self)));
    if(!SKL_IS_VECTOR(vec)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(vec));
    }
    if(!SKL_IS_INTEGER(idx)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(idx));
    }
    return skl_vec_safe_set(vec, idx, val);
}

// -*-
static Self _fn_vec_get(Self self){
    SKL_DOC("Get object stored in vector slot.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("vector-get"));
    Self vec = SKL_CAR(self);
    Self idx = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_VECTOR(vec)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(vec));
    }
    if(!SKL_IS_INTEGER(idx)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(idx));
    }
    return skl_vec_safe_get(vec, idx);
}

// -*-
static Self _fn_vec_len(Self self){
    SKL_DOC("Return the length of the vector");
    SKL_EXPECT_LEN(self, 1, skl_new_symbol("vector-len"));
    Self vec = SKL_CAR(self);
    if(!SKL_IS_VECTOR(vec)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(vec));
    }
    return skl_new_integer(SKL_VECTOR_LEN(vec));
}

// -*-
static Self _fn_new_vec(Self self){
    SKL_DOC("Create a new vector of given len, initialized to given object.");
    SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol("new-vector"));
    Self len = SKL_CAR(self);
    Self init = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_INTEGER(len)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(len));
    }
    return skl_new_vec(skl_to_integer(len), init);
}

// -*-
static Self _fn_vec_concat(Self self){
    SKL_DOC("Concatenate two vectors.");
    SKL_EXPECT_LEN(self, 2, skl_new_symbol("vector-concat"));
    Self lhs = SKL_CAR(self);
    Self rhs = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_VECTOR(lhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(lhs));
    }
    if(!SKL_IS_VECTOR(rhs)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(rhs));
    }
    return skl_vec_concat(lhs, rhs);
}

// -*-
static Self _fn_vec_slice(Self self){
    SKL_DOC("Return a subsection of a vector.");
    SKL_EXPECT_MIN_LEN(self, 2, skl_new_symbol("slice"));
    Self vec = SKL_CAR(self);
    Self beg = SKL_CAR(SKL_CDR(self));
    if(!SKL_IS_VECTOR(vec)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(vec));
    }
    if(!SKL_IS_INTEGER(beg)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(beg));
    }
    long start = skl_to_integer(beg);
    if(start >= (long)SKL_VECTOR_LEN(vec)){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(beg));
    }
    if(start < 0){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(beg));
    }
    if(SKL_CDR(SKL_CDR(self))==sklisp.Nil){
        // to the end.
        return skl_vec_slice(vec, start, -1);
    }

    Self end = SKL_CAR(SKL_CDR(SKL_CDR(self)));
    if(!SKL_IS_INTEGER(end)){
        SKL_THROW(sklisp.TypeError, SKL_INC_RC(end));
    }
    long last = skl_to_integer(end);
    if(last >= (long)SKL_VECTOR_LEN(vec)){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(end));
    }
    if(last < start){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(end));
    }
    return skl_vec_slice(vec, start, last);
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
    SKL_SYMBOL_SET(skl_new_symbol("not"), skl_new_fun(_fn_nullp));
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