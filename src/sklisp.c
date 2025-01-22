#include "sklisp.h"
#include<string.h>
#include<errno.h>
#include<assert.h>

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

// -*-
u32 sklisp_hash(Self self){
    switch(self->kind){
    case CONS:
        return SKL_TRAIT_HASH(sklisp.consTrait)(self);
    case INTEGER:
        return SKL_TRAIT_HASH(sklisp.intTrait)(self);
    case FLOAT:
        return SKL_TRAIT_HASH(sklisp.floatTrait)(self);
    case STRING:
        return SKL_TRAIT_HASH(sklisp.strTrait)(self);
    case SYMBOL:
        return SKL_TRAIT_HASH(sklisp.symTrait)(self);
    case VEC:
        return SKL_TRAIT_HASH(sklisp.vecTrait)(self);
    case CHANNEL:
        return SKL_TRAIT_HASH(sklisp.channelTrait)(self);
    case BUILTIN:
    case SPECIAL:
        return skl_hash(SKL_VALUE_DATA(self), sizeof(void*));
    }
    return 0;
}

// -
u32 skl_hash(void* obj, usize size){
    u32 result = 0;
    for(u32 i = 0; i < size; i++){
        result += ((char*)obj)[i];
        result += (result << 10);
        result ^= (result >> 6);
    }

    result += (result << 3);
    result ^= (result >> 11);
    result += (result << 15);
    return result;
}

// -*-----------*-
// -*- Numbers -*-
// -*-----------*-
// -*-
Self skl_new_integer_from_cstr(const char* from){
    Self self = skl_new(INTEGER);
    mpz_t* num = SKL_VALUE_DATA(self);
    mpz_init(*num);
    mpz_set_str(*num, from, 10);
    return self;
}

// -*-
Self skl_new_integer(long num){
    Self self = skl_new(INTEGER);
    mpz_t* data = SKL_VALUE_DATA(self);
    mpz_init(*data);
    mpz_set_ui(*data, num);
    return self;
}

// -*-
Self skl_new_float_from_cstr(const char* from){
    Self self = skl_new(FLOAT);
    mpf_t* data = SKL_VALUE_DATA(self);
    mpf_init2(*data, 256);
    mpf_set_str(*data, from, 10);
    return self;
}

// -*-
Self skl_new_float(double num){
    Self self = skl_new(FLOAT);
    mpf_t* data = SKL_VALUE_DATA(self);
    mpf_init2(*data, 64);
    mpf_set_d(*data, num);
    return self;
}

// -*-
long skl_to_integer(Self self){
    return mpz_get_si(SKL_DEREF_INTEGER(self));
}

// -*-
double skl_to_float(Self self){
    return mpf_get_d(SKL_DEREF_FLOAT(self));
}

// -*--------*-
// -*- Cons -*-
// -*--------*-
// -*-
Self skl_list_expect_len(Self self, Self err, int len){
    int n = 0;
    Self ptr = self;
    while(ptr != sklisp.Nil){
        n++;
        ptr = SKL_CDR(ptr);
        if(!SKL_IS_LIST(ptr)){
            skl_delete(err);
            SKL_THROW(sklisp.InvalidListError, self);
        }
        if(n > len){
            SKL_THROW(sklisp.InvalidArgNumberError, err);
        }
    }
    if(n != len){
        SKL_THROW(sklisp.InvalidArgNumberError, err);
    }
    return sklisp.True;
}

// -*-
Self skl_list_expect_min_len(Self self, Self err, int len){
    int n = 0;
    Self ptr = self;
    while(ptr != sklisp.Nil){
        n++;
        ptr = SKL_CDR(ptr);
        if(!SKL_IS_LIST(ptr)){
            skl_delete(err);
            SKL_THROW(sklisp.InvalidListError, self);
        }
        if(n >= len){ return sklisp.True; }
    }

    if(n < len){ SKL_THROW(sklisp.InvalidArgNumberError, err); }
    return sklisp.True;
}

// -*-
Self skl_list_expect_max_len(Self self, Self err, int len){
    int n = 0;
    Self ptr = self;
    while(ptr != sklisp.Nil){
        n++;
        ptr = SKL_CDR(ptr);
        if(!SKL_IS_LIST(ptr)){
            skl_delete(err);
            SKL_THROW(sklisp.InvalidListError, self);
        }
        if(n > len){
            SKL_THROW(sklisp.InvalidArgNumberError, err);
        }
    }
    return sklisp.True;
}

// -*-
bool skl_is_function_form(Self self){
    if(!SKL_IS_LIST(SKL_CAR(self))){ return false; }
    return skl_is_var_list(SKL_CAR(self));
}

// -*-
bool skl_is_var_list(Self self){
    int nrest = -1;
    while(self != sklisp.Nil){
        Self car = SKL_CAR(self);
        if(nrest >= 0){
            nrest--;
            if(nrest < 0){ return false; }
            if(car == sklisp.rest){ return false; }
        }
        if(!SKL_IS_SYMBOL(car)){ return false; }
        if(car == sklisp.rest){ nrest = 1; }
        self = SKL_CDR(self);
    }
    if(nrest == 1){ return false; }
    return true;
}

// -*-
Self skl_is_proper_list(Self self){
    if(self == sklisp.Nil){ return sklisp.True; }
    if(!SKL_IS_CONS(self)){ return sklisp.Nil; }
    return skl_is_proper_list(SKL_CDR(self));
}// properlistp


// -*----------*-
// -*- String -*-
// -*----------*-
// -*-
Self skl_new_string_with_len(char* cstr, usize len){
    Self self = skl_new(STRING);
    SKL_STRING(self) = cstr;
    SKL_STRING_LEN(self) = len;
    return NULL;
}

// -*-
Self skl_new_string(char* str){
    return skl_new_string_with_len(str, strlen(str));
}

// -*-
void skl_string_repr(Self self){
    String str = (String)SKL_VALUE_DATA(self);
    if(str->repr == NULL){
        str->repr = skl_handle_str_to_lexer(str->raw);
    }
}

// -*-
Self skl_string_cat(Self lhs, Self rhs){
    String xstr = (String)SKL_VALUE_DATA(lhs);
    String ystr = (String)SKL_VALUE_DATA(rhs);
    usize len = xstr->len + ystr->len;
    char* str = skl_alloc(len+1);
    memcpy(str, xstr->raw, xstr->len);
    memcpy(str+xstr->len, ystr->raw, ystr->len);
    str[len] = '\0';
    return skl_new_string_with_len(str, len);
}

// -*----------*-
// -*- Symbol -*-
// -*----------*-
// -*-
Self skl_new_symbol(const char* name){
    Self self = (Self)dict_search(sklisp.symtab, (void*)name, strlen(name));
    if(self == NULL){
        self = skl_new_uninterned_symbol(name);
        skl_symbol_intern(self);
    }
    return self;
}

// -*-
Self skl_new_uninterned_symbol(const char* cstr){
    char* val = skl_strdup(cstr);
    Self self = skl_new(SYMBOL);
    SKL_SYMBOL(self) = val;
    *((Symbol)SKL_VALUE_DATA(self))->values = sklisp.Nil;
    if(val[0]==':'){
        SKL_SYMBOL_UPDATE(self, self);
    }
    return self;
}

// -*-
void skl_symbol_intern(Self self){
    dict_insert(
        sklisp.symtab,
        SKL_SYMBOL(self),
        strlen(SKL_SYMBOL(self)),
        self,
        sizeof(Self)
    );
    SKL_SYMBOL_PROPS(self) |= SKL_SYMBOL_INTERNED;
}

// -*-
void skl_symtab_pop(Self symtab){
    skl_delete(SKL_SYMBOL_GET(symtab));
    Symbol sym = (Symbol)SKL_VALUE_DATA(symtab);
    sym->values--;
}

// -*-
void skl_symtab_push(Self symtab, Self self){
    Symbol sym = (Symbol)SKL_VALUE_DATA(symtab);
    sym->values++;
    if(sym->values == sym->len + sym->stack){
        usize n = sym->values - sym->stack;
        sym->len *= 2;
        sym->stack = skl_realloc(sym->stack, sym->len*sizeof(Self));
        sym->values = sym->stack + n;
    }
    *sym->values = self;
    SKL_INC_RC(self);
}

// -*-------*-
// -*- Vec -*-
// -*-------*-
// -*-
Self skl_new_vec(usize len, Self init){
    Self self = skl_new(VEC);
    Vec vec = SKL_VALUE_DATA(self);
    vec->len = len;
    if(len==0){ len = 1; }
    vec->data = skl_alloc(sizeof(Self*)*len);
    for(usize i=0; i < vec->len; i++){
        vec->data[i] = SKL_INC_RC(init);
    }
    return self;
}

// -*-
Self skl_list_to_vec(Self self){
    int len = 0;
    Self ptr = self;
    while(ptr != sklisp.Nil){
        len++;
        ptr = SKL_CDR(ptr);
    }
    Self vec = skl_new_vec(len, sklisp.Nil);
    ptr = self;
    usize i = 0;
    while(ptr != sklisp.Nil){
        skl_vec_set(vec, i, SKL_INC_RC(SKL_CAR(ptr)));
        i++;
        ptr = SKL_CDR(ptr);
    }
    return vec;
}

// -*-
void skl_vec_set(Self self, usize idx, Self val){
    Vec vec = SKL_VALUE_DATA(self);
    Self item = vec->data[idx];
    vec->data[idx] = val;
    skl_delete(item);
}

// -*-
Self skl_vec_safe_set(Self self, Self idx, Self val){
    long i = skl_to_integer(idx);
    Vec vec = SKL_VALUE_DATA(self);
    if(i < 0 || i >= (int)vec->len){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(idx));
    }
    skl_vec_set(self, i, SKL_INC_RC(val));
    return SKL_INC_RC(val);
}

// -*-
Self skl_vec_get(const Self self, usize idx){
    Vec vec = SKL_VALUE_DATA(self);
    return vec->data[idx];
}


// -*-
Self skl_vec_safe_get(const Self self, Self idx){
    long i = skl_to_integer(idx);
    Vec vec = SKL_VALUE_DATA(self);
    if(i < 0 || i >= (int)vec->len){
        SKL_THROW(sklisp.IndexError, SKL_INC_RC(idx));
    }
    return SKL_INC_RC(skl_vec_get(self, i));
}

// -*-
Self skl_vec_concat(const Self lhs, const Self rhs){
    usize xlen = SKL_VECTOR_LEN(lhs);
    usize ylen = SKL_VECTOR_LEN(rhs);
    Self self = skl_new_vec(xlen+ylen, sklisp.Nil);
    usize i = 0;
    for(i=0; i < xlen; i++){
        skl_vec_set(self, i, SKL_INC_RC(skl_vec_get(lhs, i)));
    }
    for(i=0; i < ylen; i++){
        skl_vec_set(self, i+xlen, SKL_INC_RC(skl_vec_get(rhs, i)));
    }
    return self;
}

// -*-
Self skl_vec_slice(const Self self, int start, int end){
    Vec vec = SKL_VALUE_DATA(self);
    if(end==-1){ end = vec->len - 1; }
    Self obj = skl_new_vec(1+ end - start, sklisp.Nil);
    for(int i= start; i <= end; i++){
        skl_vec_set(obj, i - start, SKL_INC_RC(skl_vec_get(self, i)));
    }
    return obj;
}

// -*-----------*-
// -*- Channel -*-
// -*-----------*-
// -*-
Self skl_new_channel(Self self){
    if(SKL_IS_SYMBOL(self)){
        self = SKL_SYMBOL_GET(self);
    }
    Self obj = skl_new(CHANNEL);
    Channel channel = SKL_VALUE_DATA(obj);
    int xpipe[2];
    int ypipe[2];
    if(pipe(xpipe) != 0){
        SKL_THROW(
            sklisp.InvalidChannelError,
            skl_new_string(skl_strdup(strerror(errno)))
        );
    }
    if(pipe(ypipe) != 0){
        SKL_THROW(
            sklisp.InvalidChannelError,
            skl_new_string(skl_strdup(strerror(errno)))
        )
    }
    channel->pid = fork();
    if(channel->pid == 0){
        // child process
        channel->in = ypipe[0];
        channel->out = xpipe[1];
        close(ypipe[1]);
        close(xpipe[0]);

        fclose(stdin);
        int fout = fileno(stdout);
        close(fout);
        dup2(channel->out, fout);
        channel->stream = new_stream(fdopen(channel->in, "r"), NULL, "parent", 0);
        sklisp.parentChannel = obj;

        // execute given function
        Self exe = skl_new_cons(self, sklisp.Nil);
        skl_eval(exe);
        exit(0);
        SKL_THROW(sklisp.ExitFailure, obj);
    }

    // parent process
    channel->in = xpipe[0];
    channel->out = ypipe[1];
    close(xpipe[1]);
    close(ypipe[0]);
    channel->stream = new_stream(fdopen(channel->in, "r"), NULL, "channel", 0);
    return NULL;
}

// -*-
Self skl_channel(Self self){
    //! @todo
    return NULL;
}

// -*-
Self skl_channel_receive(Self self){
    //! @todo
    return NULL;
}

// -*-
Self skl_channel_send(Self self){
    //! @todo
    return NULL;
}


// -*----------------------------------------------------------------*-
// -*- Trait Initializers                                           -*-
// -*----------------------------------------------------------------*-
static u32 _hash_integer(void* obj){
    Self self = (Self)obj;
    char* str = mpz_get_str(NULL, 16, SKL_DEREF_INTEGER(self));
    u32 result = skl_hash(str, strlen(str));
    skl_free(str);
    return result;
}

// -*-
static u32 _hash_float(void* obj){
    Self self = (Self)obj;
    char* str = mpf_get_str(NULL, NULL, 16, 0, SKL_DEREF_FLOAT(self));
    u32 result = skl_hash(str, strlen(str));
    skl_free(str);
    return result;
}

// -*-
static void _print_integer(void* obj){
    Self self = (Self)obj;
    gmp_printf("%Zd", SKL_INTEGER_PTR(self));
}

// -*-
static void _print_float(void* obj){
    Self self = (Self)obj;
    gmp_printf("%.Ff", SKL_FLOAT_PTR(self));
}

// -*-
static void* _new_cons(void){
    return mempool_alloc(sklisp.conspool);
}

// -*-
static void _delete_cons(void* self){
    mempool_free(sklisp.conspool, self);
}

static u32 _hash_cons(void* obj){
    Self self = (Self)obj;
    return sklisp_hash(SKL_CAR(self)) ^ sklisp_hash(SKL_CDR(self));
}

static void _print_cons(void* obj){
    Self self = (Self)obj;
    fputc('(', stdout);
    Self ptr = self;
    while(ptr->kind == CONS){
        skl_print(SKL_CAR(ptr));
        ptr = SKL_CDR(ptr);
        if(ptr->kind == CONS){ fputc(' ', stdout); }
    }
    if(ptr != sklisp.Nil){
        printf(" . ");
        skl_print(ptr);
    }
    fputc(')', stdout);
}

// -*-
static void* _new_str(void){
    return mempool_alloc(sklisp.strpool);
}

static void _delete_str(void* obj){
    String self = (String)obj;
    if(self->repr != NULL){
        return skl_free(self->repr);
    }
    mempool_free(sklisp.strpool, (void*)self);
}

static void _print_str(void* obj){
    Self self = (Self)obj;
    printf("%s", SKL_STRING(self));
}

static u32 _hash_str(void* obj){
    Self self = (Self)obj;
    return skl_hash(SKL_STRING(self), SKL_STRING_LEN(self));
}

// -*-
static void _print_symbol(void* arg){
    Self self = (Self)arg;
    printf("%s", ((Symbol)SKL_VALUE_DATA(self))->name);
}

static u32 _hash_symbol(void* arg){
    Self self = (Self)arg;
    return skl_hash(SKL_SYMBOL(self), strlen(SKL_SYMBOL(self)));
}

// -*-
static void* _new_vec(void){
    return mempool_alloc(sklisp.vecpool);
}

// -*-
static void _dealloc_vec(void* obj){
    Vec vec = (Vec)obj;
    for(usize i=0; i < vec->len; i++){
        skl_delete(vec->data[i]);
    }
    skl_free(vec->data);
    mempool_free(sklisp.vecpool, (void*)vec);
}

// -*-
static void _print_vec(void* obj){
    Self self = (Self)obj;
    Vec vec = SKL_VALUE_DATA(self);
    if(vec->len==0){
        printf("[]");
        return;
    }
    printf("[");
    for(usize i=0; i < vec->len-1; i++){
        skl_print(vec->data[i]);
        printf(" ");
    }
    skl_print(vec->data[vec->len-1]);
    printf("]");
}

// -*-
static u32 _hash_vec(void* obj){
    u32 result = 0;
    Self self = (Self)obj;
    Vec vec = SKL_VALUE_DATA(self);
    for(usize i=0; i < vec->len; i++){
        result ^= sklisp_hash(vec->data[i]);
    }
    return result;
}

// -*-
static u32 _hash_channel(void* obj){
    pid_t pid = SKL_CHANNEL((Self)obj);
    return skl_hash(&pid, sizeof(pid_t));
}

// -*-
static void _print_channel(void* obj){
    pid_t pid = SKL_CHANNEL((Self)obj);
    printf("<ChannelID %d>", pid);
}


static Trait _intTrait = {
    .alloc = NULL,
    .dealloc = NULL,
    .print = _print_integer,
    .hash = _hash_integer,
};

static Trait _floatTrait = {
    .alloc = NULL,
    .dealloc = NULL,
    .print = _print_float,
    .hash = _hash_float,
};

static Trait _consTrait = {
    .alloc = _new_cons,
    .dealloc = _delete_cons,
    .print = _print_cons,
    .hash = _hash_cons,
};

static Trait _stringTrait = {
    .alloc = _new_str,
    .dealloc = _delete_str,
    .print = _print_str,
    .hash = _hash_str,
};

static Trait _symbolTrait = {
    .alloc = NULL,
    .dealloc = NULL,
    .print = _print_symbol,
    .hash = _hash_symbol,
};

static Trait _vecTrait = {
    .alloc = _new_vec,
    .dealloc = _dealloc_vec,
    .print = _print_vec,
    .hash = _hash_vec,
};

static Trait _channelTrait = {
    .alloc = NULL,
    .dealloc = NULL,
    .print = _print_channel,
    .hash = _hash_channel,
};

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

static void _cons_mempool_discard(void* obj){
    Cons self = (Cons)obj;
    self->car = sklisp.Nil;
    self->cdr = sklisp.Nil;
}

// -*-
static void _str_mempool_discard(void* obj){
    String str = (String)obj;
    str->raw = NULL;
    str->repr = NULL;
    str->len = 0;
}

// -*-
static void _vec_mempool_discard(void* obj){
    Vec vec = (Vec)obj;
    vec->data = NULL;
    vec->len = 0;
}

static void _init_symtab(void){
    sklisp.symtab = new_dict(2048, NULL);
    sklisp.Nil = skl_new_symbol("nil");
    *((Symbol)SKL_VALUE_DATA(sklisp.Nil))->values = sklisp.Nil;
    SKL_SYMBOL_PROPS(sklisp.Nil) |= SKL_SYMBOL_CONSTANT;
    sklisp.True = skl_new_symbol("t");
    SKL_SYMBOL_PROPS(sklisp.True) |= SKL_SYMBOL_CONSTANT;
    SKL_SYMBOL_UPDATE(sklisp.True, sklisp.True);
}

// -*-
void sklisp_initialize(void){
    // -*-
    sklisp.consTrait = &_consTrait;
    sklisp.intTrait = &_intTrait;
    sklisp.floatTrait = &_floatTrait;
    sklisp.strTrait = &_stringTrait;
    sklisp.symTrait = &_symbolTrait;
    sklisp.vecTrait = &_vecTrait;
    sklisp.channelTrait = &_channelTrait;
    // -*-
    sklisp.mempool = new_mempool(sizeof(struct object), _object_mempool_discard);
    sklisp.conspool = new_mempool(sizeof(struct cons), _cons_mempool_discard);
    sklisp.strpool = new_mempool(sizeof(struct str), _str_mempool_discard);
    sklisp.vecpool = new_mempool(sizeof(struct vec), _vec_mempool_discard);

    // -*-
    _init_symtab();
    //! @todo
}

/*
void sklisp_finalize(struct SKLisp* app);
*/