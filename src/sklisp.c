#include "sklisp.h"

// -*--------------------------*-
// -*- Object && Fun && SForm -*-
// -*--------------------------*-
// -
static Symbol _new_symbol(void){
    Symbol sym = skl_alloc(sizeof(*sym));
    sym->props = 0;
    sym->len = 8;
    void *node = skl_alloc(sizeof(Self)*sym->len);
    sym->values = node;
    sym->stack = node;
    return sym;
}

// -
Self skl_new(TypeKind kind){
    Self self = (Self)mempool_alloc(sklisp.mempool);
    self->kind = kind;
    self->rc++;
    switch(kind){
    case INTEGER:
        SKL_VALUE_DATA(self) = skl_alloc(sizeof(mpz_t));
        break;
    case FLOAT:
        SKL_VALUE_DATA(self) = skl_alloc(sizeof(mpf_t));
        break;
    case SYMBOL:
        SKL_VALUE_DATA(self) = _new_symbol();
        break;
    case STRING:
        SKL_VALUE_DATA(self) = SKL_TRAIT_ALLOC(sklisp.strTrait);
        break;
    case CONS:
        SKL_VALUE_DATA(self) = SKL_TRAIT_ALLOC(sklisp.consTrait);
        break;
    case VEC:
        SKL_VALUE_DATA(self) = SKL_TRAIT_ALLOC(sklisp.vecTrait);
        break;
    case CHANNEL:
        SKL_VALUE_DATA(self) = skl_alloc(sizeof(struct channel));
        break;
    case BUILTIN:
    case SPECIAL:
        break;
    }
    return self;
}


// -
Self skl_new_cons(Self car, Self cdr){
    Self self = skl_new(CONS);
    SKL_CAR(self) = car;
    SKL_CDR(self) = cdr;
    return self;
}

// -
Self skl_new_fun(Fun fun){
    Self self = skl_new(BUILTIN);
    SKL_VALUE_FUN(self) = fun;
    return self;
}

// -
Self skl_new_special(Fun special){
    Self self = skl_new(SPECIAL);
    SKL_VALUE_FUN(self) = special;
    return self;
}

// -*-
static void _delete_channel(Self self){
    Channel channel = SKL_VALUE_DATA(self);
    delete_stream(channel->stream);
    close(channel->in);
    close(channel->out);
}
// -
void skl_delete(Self self){
    if(SKL_IS_SYMBOL(self)){ return; }
    self->rc--;
    if(self->rc > 0){ return; }

    mpz_t* inum = NULL;
    mpf_t* fnum = NULL;
    switch(self->kind){
    case SYMBOL:
        return;
    case FLOAT:
        fnum = SKL_FLOAT_PTR(self);
        mpf_clear(*fnum);
        skl_free(SKL_VALUE_DATA(self));
        break;
    case INTEGER:
        inum = SKL_INTEGER_PTR(self);
        mpz_clear(*inum);
        skl_free(SKL_VALUE_DATA(self));
        break;
    case STRING:
        SKL_TRAIT_DEALLOC(sklisp.strTrait);
        break;
    case CONS:
        skl_delete(SKL_CAR(self));
        skl_delete(SKL_CDR(self));
        SKL_TRAIT_DEALLOC(sklisp.consTrait);
        break;
    case VEC:
        SKL_TRAIT_DEALLOC(sklisp.vecTrait);
        break;
    case CHANNEL:
        _delete_channel(self);
        skl_free(SKL_VALUE_DATA(self));
        break;
    case BUILTIN:
    case SPECIAL:
        break;
    }
    mempool_free(sklisp.mempool, (void*)self);
}

// -*-
void skl_print(Self self){
    switch(self->kind){
    case CONS:
        SKL_TRAIT_PRINT(sklisp.consTrait);
        break;
    case INTEGER:
        SKL_TRAIT_PRINT(sklisp.intTrait);
        break;
    case FLOAT:
        SKL_TRAIT_PRINT(sklisp.floatTrait);
        break;
    case STRING:
        SKL_TRAIT_PRINT(sklisp.strTrait);
        break;
    case SYMBOL:
        SKL_TRAIT_PRINT(sklisp.symTrait);
        break;
    case VEC:
        SKL_TRAIT_PRINT(sklisp.vecTrait);
        break;
    case CHANNEL:
        SKL_TRAIT_PRINT(sklisp.channelTrait);
        break;
    case BUILTIN:
        printf("<builtin @ 0x%p", self->value.fun);
        break;
    case SPECIAL:
        printf("<special-form @ 0x%p", self->value.fun);
        break;
    default:
        fputs("Error: unknown type", stderr);
        break;
    }
}

// -*-
void skl_println(Self self){
    skl_print(self);
    fputc('\n', stdout);
}

// -
u32 skl_hash(void* obj, usize size){
    //! @todo
    return 0;
}

// -*- Numbers -*-
// -*- Cons -*-
// -*- String -*-
// -*- Symbol -*-
// -*- Vec -*-
// -*- Channel -*-


// -*----------------------------------------------------------------*-
// -*- Trait Initializers                                           -*-
// -*----------------------------------------------------------------*-
/*
static Trait _intTrait;
static Trait _floatTrait;
static Trait _stringTrait;
static Trait _symbolTrait;
static Trait _consTrait;
static Trait _funTrait;
static Trait _sformTrait;
static Trait _vecTrait;
static Trait _channelTrait;
*/

// -*----------------------------------------------------------------*-
// -*- Mempool initializers                                         -*-
// -*----------------------------------------------------------------*-
static void _object_mempool_discard(void* obj){
    Self self = (Self)obj;
    self->kind = SYMBOL;
    self->rc = 0;
    SKL_VALUE_DATA(self) = sklisp.Nil;
    SKL_VALUE_FUN(self) = NULL;
}

/*
static Mempool _mempool;
static Mempool _conspool;
static Mempool _strpool;
static Mempool _vecpool;
*/

// -*-
void sklisp_initialize(void){
    sklisp.mempool = new_mempool(sizeof(struct object), _object_mempool_discard);
    //! @todo
}

/*
void sklisp_finalize(struct SKLisp* app);
*/