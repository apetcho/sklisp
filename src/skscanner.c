#include "sklisp.h"
#include<errno.h>
#include<assert.h>
#include<string.h>
#include<signal.h>

static const char* _skl_prelude_[] = {
    [0] = ";;; -*- mode: lisp -*-",
    [1] = ";;; Core definitions for sklisp",
    [2] = "",
    [3] = "Set up require, so we can start pulling external librabries",
    [4] = "",
    [5] = ";; -*---------------*-",
    [6] = ";; -*- (push x xs) -*-",
    [7] = ";; -*---------------*-",
    [8] = "(defmacro push (x xs)",
    [9] = "     \"Push `x` onto list stored at `xs`.\"",
    [10] = "     (list 'set (list 'quote xs) (list 'cons x xs)))",
    [11] = "",
    [12] = ";; -*---------------*-",
    [13] = ";; -*- (equal x y) -*-",
    [14] = ";; -*---------------*-",
    [15] = "(defun equal (x y)",
    [16] = "     \"Return `t` if both `x` and `y` have similar structure and contents.\"",
    [17] = "     (or (eql x y)",
    [18] = "         (and (list? x) (list? y)",
    [19] = "              (equal (car x) (car y))",
    [20] = "              (equal (cdr x) (cdr y)))))",
    [21] = "",
    [22] = ";; -*-----------------*-",
    [23] = ";; -*- (member x xs) -*-",
    [24] = ";; -*-----------------*-",
    [25] = "(defun member (x xs)",
    [26] = "    \"Return non-nil if `x` is in the list `xs`.\"",
    [27] = "    (if (null? xs)",
    [28] = "        nil",
    [29] = "        (if (equal x (car xs))",
    [30] = "            xs",
    [31] = "            (member x (cdr xs)))))",
    [32] = "",
    [33] = ";; -*-----------------*-",
    [34] = ";; -*- (provide lib) -*-",
    [35] = ";; -*-----------------*-",
    [36] = "(defun provide (lib)",
    [37] = "    \"Set library `lib` as already loaded .\"",
    [38] = "    (push lib *provide-list*))",
    [39] = "",
    [40] = ";; -*-----------------*-",
    [41] = ";; -*- (require lib) -*-",
    [42] = ";; -*-----------------*-",
    [43] = "(defun require (lib)",
    [44] = "    \"Bring library functions into current environment.\"",
    [45] = "    (if (member lib *provide-list*)",
    [46] = "        t",
    [47] = "        (load (string-concat",
    [48] = "                *sklroot* \"/lib/\"",
    [49] = "                (symbol-name lib) \".skl\"))))",
    [50] = "",
    [51] = ";; -*-----------------------------------*-",
    [52] = ";; -*- Load up other default libraries -*-",
    [53] = ";; -*-----------------------------------*-",
    [54] = "(require 'list)",
    [55] = "(require 'math)",
    [56] = "(require 'vector)",
    [57] = "(require 'set)",
    [58] = "",
    [59] = ";; -*-------------------*-",
    [60] = ";; -*- (setq name val) -*-",
    [61] = ";; -*-------------------*-",
    [62] = "(defun setq (name val)",
    [63] = "    \"Automatically quote the first argument for `set`.\"",
    [64] = "    (list 'set (list 'quote name) val))",
    [65] = "",
    [66] = ";; -*--------------------*-",
    [67] = ";; -*- (new-symbol str) -*-",
    [68] = ";; -*--------------------*-",
    [69] = "(defun new-symbol (str)",
    [70] = "    \"Create a new symbol from string `str`.\"",
    [71] = "    (if (not (string? str))",
    [72] = "        (throw 'type-error str)",
    [73] = "        (read-string str)))",
    [74] = "",
    [75] = ";; -*--------------------*-",
    [76] = ";; -*- (doc-string obj) -*-",
    [77] = ";; -*--------------------*- ",
    [78] = "(defun doc-string (obj)",
    [79] = "    \"Return documentation string for object.\"",
    [80] = "    (if (symbol? obj)",
    [81] = "        (setq obj (value obj)))",
    [82] = "    (if (list? obj)",
    [83] = "        (if (string? (third obj))",
    [84] = "            (third obj))",
    [85] = "        (docstr obj)))",
    [86] = NULL,
};

#define SKL_ARRAY_LEN(arr)  sizeof(arr)/sizeof(arr[0])
#define SKL_PRELUDE_LEN     SKL_ARRAY_LEN(_skl_prelude_)

// -*-
static bool _interrupt = false;

static void _handle_interrupt(int sig){
    SKL_UNUSED(sig);
    if(!_interrupt && sklisp.interactiveMode){
        _interrupt = true;
        signal(SIGINT, _handle_interrupt);
        // exit(EXIT_SUCCESS);
    }else{
        signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }
}

// -*-
void skl_init(void){
    // install interrupt handler
    signal(SIGINT, _handle_interrupt);
    const char* sklprelude = "sklprelude.skl";
    FILE* fp = fopen(sklprelude, "");
    if(fp == NULL){
        char buffer[256] = {0};
        snprintf(
            buffer, sizeof(buffer)-1,
            "Error: unable to create file '%s'", sklprelude
        );
        fputs(buffer, stderr);
        exit(EXIT_FAILURE);
    }
    for(int i=0; i < SKL_PRELUDE_LEN; i++){
        if(_skl_prelude_[i]==NULL){ break;}
        fputs(_skl_prelude_[i], fp);
    }
    fclose(fp);
    int rc = stream_load_file(NULL, (char*)sklprelude, 0);
    if(!rc){
        char buffer[256] = {0};
        snprintf(
            buffer, sizeof(buffer)-1,
            "Error: unable to load file '%s'", sklprelude
        );
        fputs(buffer, stderr);
        exit(EXIT_FAILURE);
    }
    remove(sklprelude);
}// i.e eval_init

// -*-
static Self _skl_eval_list(Self self){
    if(self == sklisp.Nil){ return sklisp.Nil; }
    if(!SKL_IS_CONS(self)){
        SKL_THROW(sklisp.InvalidListEndError, SKL_INC_RC(self));
    }
    Self car = skl_eval(SKL_CAR(self));
    SKL_CHECK(car);
    Self cdr = _skl_eval_list(SKL_CDR(self));
    if(cdr == sklisp.Error){
        skl_delete(car);
        return sklisp.Error;
    }

    return skl_new_cons(car, cdr);
}

// -*-
Self skl_eval_body(Self body){
    Self result = sklisp.Nil;
    while(body != sklisp.Nil){
        skl_delete(result);
        result = skl_eval(SKL_CAR(body));
        SKL_CHECK(result);
        body = SKL_CDR(body);
    }
    return result;
}

// -*-
Self skl_bind_args(Self vars, Self argv){
    int optMode = 0;
    int len = 0;
    Self _vars = vars;
    while(vars != sklisp.Nil){
        Self var = SKL_CAR(vars);
        if(var == sklisp.optional){
            // Turn on optional mode and continue
            optMode = 1;
            vars = SKL_CDR(vars);
            continue;
        }
        if(var == sklisp.rest){
            // Assign the rest of the list and finish
            vars = SKL_CDR(vars);
            skl_symtab_push(SKL_CAR(vars), argv);
            argv = sklisp.Nil;
            break;
        }else if(!optMode && argv == sklisp.Nil){
            while(len > 0){
                skl_symtab_pop(SKL_CAR(_vars));
                _vars = SKL_CDR(_vars);
                len--;
            }
            SKL_THROW(sklisp.InvalidArgNumberError, sklisp.Nil);
        }else if(optMode && argv == sklisp.Nil){
            skl_symtab_push(var, sklisp.Nil);
        }else{
            Self arg = SKL_CAR(argv);
            skl_symtab_push(var, arg);
            len++;
        }
        vars = SKL_CDR(vars);
        if(argv != sklisp.Nil){
            argv = SKL_CDR(argv);
        }
    }

    // argv should be consumed by now
    if(argv != sklisp.Nil){
        skl_unbind_args(argv);
        SKL_THROW(sklisp.InvalidArgNumberError, sklisp.Nil);
    }
    return sklisp.True;
}// i.e assign_args

// -*-
void skl_unbind_args(Self vars){
    if(vars == sklisp.Nil){ return; }
    Self var = SKL_CDR(vars);
    if(var != sklisp.rest && var != sklisp.optional){
        skl_symtab_pop(var);
    }
    skl_unbind_args(SKL_CDR(vars));
}// i.e unassign_args

// -*-
Self skl_top_eval(Self self){
    sklisp.stackDepth = 0;
    Self result = skl_eval(self);
    if(result == sklisp.Error){
        fputs("SKLisp Error:", stderr);
        Self cell = skl_new_cons(
            sklisp.ThrownError,
            skl_new_cons(sklisp.AttachError, sklisp.Nil)
        );
        skl_println(cell);
        skl_delete(cell);
        return sklisp.Error;
    }
    return result;
}

// -*-
Self skl_apply(Self fun, Self argv){
    //! @todo
    return 0;
}

// -*-
Self skl_eval(Self self){
    //! @todo
    return NULL;
}