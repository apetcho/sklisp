#ifndef __SKLISP_H__
#define __SKLISP_H__

#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>
#include<stdarg.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<gmp.h>

typedef size_t usize;
typedef uint32_t u32;
typedef uint8_t u8;
typedef struct object* Self;
typedef struct cons* Cons;
typedef struct vec* Vec;
typedef struct str* String;
typedef struct channel* Channel;
typedef struct symbol* Symbol;
typedef Self (*Fun)(Self);

#define SKL_UNUSED(arg)     (void)arg

// #SKL_VALUE_DATA(obj)                                     // ::OVAL
#define SKL_VALUE_DATA(self)    ((self)->value.data)
// #SKL_VALUE_FUN                                           // ::FVAL
#define SKL_VALUE_FUN(self)     ((self)->value.fun)
// #SKL_IS_STRING()                                         // ::STRINGP
#define SKL_IS_STRING(self)     ((self)->kind==STRING)
// #SKL_IS_SYMBOL()                                         // ::SYMBOLP
#define SKL_IS_SYMBOL(self)     ((self)->kind==SYMBOL)
// #SKL_IS_CONS()                                           // ::CONSP
#define SKL_IS_CONS(self)       ((self)->kind==CONS)
// #SKL_INC_RC                                              // ::UPREF
#define SKL_INC_RC(self)        ((self)->rc++, self)
// #SKL_DEBUG()                                             // ::DB_OP
#define SKL_DEBUG(msg, self)    printf(msg); sklisp_print(self)
// #SKL_CAR(obj)                                            // ::CAR
#define SKL_CAR(self)           ((Cons)SKL_VALUE_DATA(self))->car
// #SKL_CDR(obj)                                            // ::CDR
#define SKL_CDR(self)           ((Cons)SKL_VALUE_DATA(self))->cdr
// #SKL_IS_LIST(obj)                                        // ::LISTP
#define SKL_IS_LIST(self)       ((self)->kind==CONS || (self)==sklisp.Nil)
// #SKL_IS_PAIR(obj)                                        // ::PAIRP
#define SKL_IS_PAIR(self)       ((self)->kind==CONS && !SKL_IS_LIST(SKL_CDR(self)))
// #SKL_INTEGER_PTR                                         // ::OINT
#define SKL_INTEGER_PTR(self)   ((mpz_t*)SKL_VALUE_DATA(self))
// #SKL_FLOAT_PTR                                           // ::OFLOAT
#define SKL_FLOAT_PTR(self)     ((mpf_t*)SKL_VALUE_DATA(self))
// #SKL_DEREF_INTEGER                                       // ::DINT
#define SKL_DEREF_INTEGER(self) (*((mpz_t*)SKL_VALUE_DATA(self)))
// #SKL_DEREF_FLOAT                                         // ::DFLOAT
#define SKL_DEREF_FLOAT(self)   (*((mpf_t*)SKL_VALUE_DATA(self)))
// #SKL_IS_INTEGER                                          // ::INTP
#define SKL_IS_INTEGER(self)    ((self)->kind == INTEGER)
// #SKL_IS_FLOAT                                            // ::FLOATP
#define SKL_IS_FLOAT(self)      ((self)->kind == FLOAT)
// #SKL_IS_NUMBER                                           // ::NUMP
#define SKL_IS_NUMBER(self)     (SKL_IS_INTEGER(self) || SKL_IS_FLOAT(self))
// #SKL_STRING                                              // ::OSTR
#define SKL_STRING(self)        (((String)SKL_VALUE_DATA(self))->raw)
// #SKL_STRING_LEN                                          // ::OSTRLEN
#define SKL_STRING_LEN(self)    (((String)SKL_VALUE_DATA(self))->len)
// #SKL_STRING_REPR                                         // ::OSTRP
#define SKL_STRING_REPR(self)   \
    (skl_string_genp(self), ((String)SKL_VALUE_DATA(self))->repr)

// #SKL_SYMBOL()                                            // :: SYMNAME
#define SKL_SYMBOL(self)        (((Symbol)SKL_VALUE_DATA(self))->name)
// #SKL_SYMBOL_GET()                                        // :: GET
#define SKL_SYMBOL_GET(self)    (*((Symbol)SKL_VALUE_DATA(self))->values)          
// #SKL_SYMBOL_UPDATE()                                     // :: SET
#define SKL_SYMBOL_UPDATE(self, obj)                    \
    (                                                   \
        skl_delete(SKL_SYMBOL_GET(self)),               \
        (void)SKL_INC_RC(obj),                          \
        *((Symbol)SKL_VALUE_DATA(self))->values = obj   \
    )

// #SKL_SYMBOL_SET()                                        // :: SSET
#define SKL_SYMBOL_SET(self, obj)                       \
    (                                                   \
        skl_delete(SKL_SYMBOL_GET(self)),               \
        *((Symbol)SKL_VALUE_DATA(self))->values = obj   \
    )

// #SKL_SYMBOL_CONSTANT                                     // :: SYM_CONSTANT
#define SKL_SYMBOL_CONSTANT     0x1
// #SKL_SYMBOL_INTERNED                                     // :: SYM_INTERNED
#define SKL_SYMBOL_INTERNED     0x2
// #SKL_SYMBOL_PROPS                                        // :: SYMPROPS
#define SKL_SYMBOL_PROPS(self)          \
    (((Symbol)SKL_VALUE_DATA(self))->props)
// #SKL_SYMBOL_IS_CONSTANT                                  // :: CONSTANTP
#define SKL_SYMBOL_IS_CONSTANT(self)    \
    (SKL_SYMBOL_PROPS(self) & SKL_SYMBOL_CONSTANT)
// #SKL_SYMBOL_IS_INTERNED                                  // :: INTERNP
#define SKL_SYMBOL_IS_INTERNED(self)    \
    (SKL_SYMBOL_PROPS(self) & SKL_SYMBOL_INTERNED)

// #SKL_IS_VECTOR                                           // ::VECTORP
#define SKL_IS_VECTOR(self)         ((self)->kind==VEC)
// #SKL_VECTOR_LEN                                          // ::VLENGTH
#define SKL_VECTOR_LEN(self)        (((Vec)SKL_VALUE_DATA(self))->len)
// #SKL_CHANNEL                                             // :: OPROC
#define SKL_CHANNEL(self)           (((Channel)SKL_VALUE_DATA(self))->pid)
// #SKL_CHANNEL_STREAM                                      // :: OREAD
#define SKL_CHANNEL_STREAM(self)    (((Channel)SKL_VALUE_DATA(self))->stream)
// #SKL_IS_CHANNEL                                          // :: DETACHP
#define SKL_IS_CHANNEL(self)        ((self)->kind==CHANNEL)

// #SKL_IS_CALLABLE                                         // ::FUNCP
#define SKL_IS_CALLABLE(self)                                                   \
    (                                                                           \
        ((self)->kind==CONS && SKL_CAR(self)->kind==SYMBOL                      \
        && ((SKL_CAR(self)==sklisp.lambda) || (SKL_CAR(self)==sklisp.macro)))   \
        || ((self)->kind==BUILTIN) || ((self)->kind==SPECIAL)                   \
    )

// #SKL_THROW                                               // :: THROW
#define SKL_THROW(to, self)         \
    {                               \
        sklisp.ThrownError = to;    \
        sklisp.AttachError = self;  \
        return sklisp.Error;        \
    }

// #SKL_CHECK                                               // :: CHECK
#define SKL_CHECK(self)             \
    if((self)==sklisp.Error){ return sklisp.Error; }

// #SKL_EXPECT_LEN                                          // :: REQ
#define SKL_EXPECT_LEN(xs, n, err)                      \
    if(skl_list_expect_len(xs, err, n)==sklisp.Error){  \
        return sklisp.Error;                            \
    }
// #SKL_EXPECT_MIN_LEN                                      // :: REQM
#define SKL_EXPECT_MIN_LEN(xs, n, err)                      \
    if(skl_list_expect_min_len(xs, err, n)==sklisp.Error){  \
        return sklisp.Error;                                \
    }

// #SKL_EXPECT_MAX_LEN                                      // :: REQX
#define SKL_EXPECT_MAX_LEN(xs, n, err)                      \
    if(skl_list_expect_max_len(xs, err, n)==sklisp.Error){  \
        return sklisp.Error;                                \
    }

// #SKL_EXPECT_LIST                                         // :: REQPROP
#define SKL_EXPECT_LIST(self)                               \
    if(skl_is_proper_list(self)==sklisp.Nil){               \
        SKL_THROW(sklisp.InvalidListError, self);           \
    }
// #SKL_DOC                                                 // :: DOC
#define SKL_DOC(doc)                                        \
    if(self==sklisp.docstr){                                \
        return skl_new_string(skl_strdup(doc));             \
    }

#define SKL_TRAIT_ALLOC(trait)      ((trait)->alloc)
#define SKL_TRAIT_DEALLOC(trait)    ((trait)->dealloc)
#define SKL_TRAIT_PRINT(trait)      ((trait)->print)
#define SKL_TRAIT_HASH(trait)       ((trait)->hash)


// -*----------------------------------------------------------------*-
// -*-                        SKLISP::UTILS                         -*-
// -*----------------------------------------------------------------*-
// common.h
void* skl_alloc(usize);
void* skl_realloc(void*, usize);
void skl_free(void*);
char* skl_strdup(const char* str);
char* skl_joinpath(const char*, const char*);
char* skl_handle_str_from_lexer(const char*);   // process_str
char* skl_handle_str_to_lexer(const char*);     // unprocess_str
void skl_error(const char*);

// -----------------*-
// -*- hashtable.h -*-
// -----------------*-
typedef struct entry* Entry;
struct entry{
    void* key;
    usize klen;
    void* val;
    usize vlen;

    Entry next;
};

typedef struct dict* Dict;
typedef struct dictIterator* DictIterator;

struct dict {
    Entry *bucket;
    usize size;
    int len;
    int (*hashfn)(void*, usize, usize);
};

struct dictIterator {
    void* key;
    void* val;
    usize klen;
    usize vlen;
    // bookkeeping data
    struct {
        Dict dict;
        Entry entry;
        int index;
    } internal;
};

Dict new_dict(usize, int (*hash)(void*, usize, usize));
void* dict_search(Dict dict, void* key, usize klen);
void* dict_insert(Dict dict, void* key, usize klen, void* val, usize vlen);
void dict_remove(Dict dict, void* key, usize klen);
Dict dict_rehash(Dict dict, usize newSize);
void dict_destroy(Dict dict);
void dict_init_iterator(Dict dict, DictIterator iterator);
void dict_iterator_next(DictIterator iterator);
int dict_hash(void* key, usize klen, usize len);

// -*----------------------------------------------------------------*-
// -*-                       SKLISP::SCANNER                        -*-
// -*----------------------------------------------------------------*-
// -*- reader.h -*-
typedef struct {
    Self head;
    Self tail;
    int quoteMode;
    int dotpairMode;
    int vecMode;
} SKLState;

typedef struct {
    FILE* fp;
    char* src;          // str

    // -*- meta -*-
    char* name;         // ??? filename
    int interactive;
    char* ps1;
    char* ps2;

    // -*- Stream State -*-
    u32 lineno;
    char* srcp;         // strp

    // -*- atom stream buffer
    char* buf;
    char* bufp;
    usize buflen;

    // -*- stream buffer -*-
    int* readbuf;
    int* readbufp;
    usize readbuflen;

    // -*- indicators -*-
    int eof;
    int error;
    int shebang;
    int done;

    // -*- state stack -*-
    usize stackSize;
    SKLState* base;
    SKLState* state;
} Stream; // reader

Stream* new_stream(FILE* fp, char* str, char* name, int interactive);
void delete_stream(Stream* stream);
Self stream_read_sexp(Stream* stream);
int stream_load_file(FILE* fp, char* filename, int interactive);
void repl(void);


// -*----------------------------------------------------------------*-
// -*-                     SKLISP::OBJECT & Co.                     -*-
// -*----------------------------------------------------------------*-
// -*- mem.h -*-
typedef struct mempool* Mempool;
struct mempool {
    usize itemsize;
    void **stack;
    void **base;
    usize len;
    void (*discard)(void*);
};

Mempool new_mempool(usize itemsize, void (*discard)(void*));
void delete_mempool(Mempool mempool);
void* mempool_alloc(Mempool mempool);
void mempool_free(Mempool mempool, void* item);


// -*------------*-
// -*- object.h -*-
// -*------------*-
typedef enum TypeKind {
    INTEGER,
    FLOAT,
    STRING,
    SYMBOL,
    CONS,
    VEC,
    BUILTIN,    // CFUNC
    SPECIAL,
    CHANNEL,    // DETACH
} TypeKind;

// -*-
typedef struct {
    void* (*alloc)(void);       // *_create
    void (*dealloc)(void*);     // *_destroy
    void (*print)(void*);
    u32 (*hash)(void*);
} Trait;

// -*-
typedef union{
    void* data;
    Self (*fun)(Self);
} Value;

// -*-
struct object{
    TypeKind kind;
    u32 rc;
    Value value;
};


// -
Self skl_new(TypeKind kind);
void skl_delete(Self self);
void skl_print(Self self);
void skl_println(Self self);
Self skl_new_cons(Self car, Self cdr);
Self skl_new_fun(Fun fun);
Self skl_new_special(Fun special);
// -
u32 skl_hash(void* obj, usize size);
u32 sklisp_hash(Self self);


/*
void object_init(void);                             // XXX: <init>
//Object create_object(TypeKind);                        // <create> obj_create()
void destroy_object(Object self);                       // <destroy>
u32 obj_hash(Object self);                              // <hash>
u32 hash(void* buf, usize buflen);
static {
    stataic Mempool *mempool => objMempool !!!!
    static object_discard(void* self);
}
*/

// -*----------*-
// -*- cons.h -*-
// -*----------*-
struct cons{
    Self car;
    Self cdr;
};

Self skl_list_expect_len(Self self, Self err, int len);
Self skl_list_expect_min_len(Self self, Self err, int len);
Self skl_list_expect_max_len(Self self, Self err, int len);
bool skl_is_function_form(Self self);
bool skl_is_var_list(Self self);
Self skl_is_proper_list(Self self);         // properlistp

/*
void cons_init(void);                       // <init>
Cons create_cons(void);                     // <create>
void destroy_cons();                        // <destroy>
u32 cons_hash(Object self)                  // <hash>
static {
    Mempool* mempool;
    void cons_discard(void)
}
*/

// -*------------*-
// -*- number.h -*-
// -*------------*-
Self skl_new_integer_from_cstr(const char* from);
Self skl_new_integer(long num);
Self skl_new_float_from_cstr(const char* from);
Self skl_new_float(double num);
long skl_to_integer(Self self);
double skl_to_float(Self self);

/*
u32 integer_hash(Object self);              // <hash>
u32 float_hash(Object self);                // <hash>
*/

// -*- str.h -*-
struct str {
    char* raw;
    char* repr;     // print
    usize len;
};
Self skl_new_string_with_len(char* cstr, usize len);
Self skl_new_string(char* str);
void skl_string_repr(Self self);
Self skl_string_cat(Self lhs, Self rhs);

/*
void str_init(void)                             // <init>
String* create_string(void);                    // <create>
void destroy_string(void);                      // <destroy>
u32 string_hash(Object self);                   // <hash>
static {
    Mempool* mempool;
    void string_discard(void* obj);
}
*/

// -*- symtab.h -*-
struct symbol {
    char* name;
    u8 props;
    Self* values;
    Self* stack;
    u32 len;
};

Self skl_new_symbol(const char* name);
Self skl_new_uninterned_symbol(const char*);
void skl_symbol_intern(Self self);
void skl_symtab_pop(Self symtab);
void skl_symtab_push(Self symtab, Self self);

/*
void symtab_init(void);                             // :: <init>
Symbol* create_symbol(void);                        // :: <create>
u32 symbol_hash(Object self);                       // <hash>
*/

// -*- vector.h -*-
struct vec {
    Self *data;
    usize len;
};


Self skl_new_vec(usize len, Self init);
Self skl_list_to_vec(Self self);
void skl_vec_set(Self self, usize idx, Self val);
Self skl_vec_get(const Self self, usize idx);
Self skl_vec_safe_set(Self self, Self idx, Self val);
Self skl_vec_safe_get(const Self self, Self idx);
Self skl_vec_concat(const Self lhs, const Self rhs);
Self skl_vec_slice(const Self self, int start, int end);

/*
void vec_init(void);                                // :: <init>
Vec create_vec(void);                               // :: <create>
void destroy_vec(void* vec);                        // :: <destroy>
void vec_print(const Self self);                    // :: <str> -> print()
u32 vec_hash(Object self);                              // :: <hash>
*/

// -*------------*-
// -*- detach.h -*-
// -*------------*-
struct channel {
    int in;
    int out;
    pid_t pid;
    Stream* stream;
};

Self skl_new_channel(Self self);
Self skl_channel(Self self);
Self skl_channel_receive(Self self);
Self skl_channel_send(Self self);

/* :: Channel
Channel* create_channel(void);                          // :: <create>
void destroy_channel(Self self);                        // :: <destroy>
u32 channel_hash(Object self);                          // :: <hash>
void channel_print(void);                               // :: <str>
*/

// -*----------*-
// -*- eval.h -*-
// -*----------*-
Self skl_eval(Self self);
Self skl_top_eval(Self self);
Self skl_eval_body(Self self);
Self skl_bind_args(Self vars, Self argv);           // i.e assign_args
void skl_unbind_args(Self vars);                    // i.e unassign_args
Self skl_apply(Self fun, Self argv);
Self skl_num_equal(Self);
/*
void sklisp_init(void* app); <sklisp_init()>                     // i.e wisp_init
:{
    // init mempools;
    // init symbtabs
    // init other sklisp fields
    // void skl_init_math(SKLisp* app);
    // void skl_init_builtins(SKLisp* app);
    // void skl_init(SKLisp* app);
    object_init()   :=> objMempool
    symtab_init()   :=> symtab
    cons_init()     :=> consMempool
    str_init()      :=> strMempool
    lisp_init()     :{ skl_init_math, skl_init_builtins}
    vec_init()      :=> vecMempool <skl_init_mempools()>
    eval_init()     :=> skl_init()
}
*/

// -*- lisp.h -*-
/*
void sklisp_init(void);                                 // i.e lisp_init()
--> init_builtins
--> init_math;
*/

// ::lisp_math

struct SKLisp{
    // -*----------*-
    // -*- Traits -*-
    // -*----------*-
    Trait* intTrait;
    Trait* floatTrait;
    Trait* strTrait;
    Trait* symTrait;
    Trait* consTrait;
    Trait* channelTrait;
    Trait* vecTrait;
    // -*------------*-
    // -*- mempools -*-
    // -*------------*-
    Mempool mempool;       // object
    Mempool conspool;
    Mempool strpool;
    Mempool vecpool;
    // -*----------------*-
    // -*- symbol table -*-
    // -*----------------*-
    Dict symtab;
    // -*-
    char* ps1;
    char* ps2;
    char* path;             // - wisproot

    Self Nil;
    Self True;
    Self parentChannel;

    u32 stackDepth;
    u32 maxStackDepth;
    int interactiveMode;

    // -*------------*-
    // -*-  symbols -*-
    // -*------------*-
    Self lambda;
    Self macro;
    Self rest;
    Self optional;
    Self quote;
    Self docstr;

    // -*--------------*-
    // -*- Exceptions -*-
    // -*--------------*-
    Self Error;
    Self ThrownError;
    Self AttachError;
    Self VoidFunError;
    Self InvalidArgNumberError;
    Self TypeError;
    Self ValueError;
    Self InvalidListError;
    Self InvalidListEndError;
    Self InterruptError;
    Self IndexError;
    Self InvalidChannelError;
    Self ExitFailure;
    Self ChannelSendError;
};

// -*-
extern struct SKLisp sklisp;

void skl_init_math(void);
void skl_init_builtins(void);
// void skl_init_interpreter(struct SKLisp* app);          // :: eval_init()
void skl_init(void);                      // i.e eval_init
void sklisp_initialize(void);
void sklisp_finalize(void);

#endif