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
Self skl_eval(Self self){
    return NULL;
}

// -*-
Self skl_top_eval(Self self){
    //! @todo
    return 0;
}

// -*-
Self skl_eval_body(Self self){
    //! @todo
    return 0;
}

// -*-
Self skl_bind_args(Self vars, Self argv){
    //! @todo
    return 0;
}// i.e assign_args

// -*-
Self skl_unbind_args(Self vars){
    //! @todo
    return 0;
}// i.e unassign_args

// -*-
Self skl_apply(Self fun, Self argv){
    //! @todo
    return 0;
}