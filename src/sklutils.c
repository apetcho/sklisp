#include "sklisp.h"

// -*-
void* skl_alloc(usize size){
    //! @todo
    return NULL;
}

void* skl_realloc(void* arg, usize size){
    //! @todo
    return NULL;
}

void skl_free(void* arg){
    //! @todo
}

char* skl_strdup(const char* str){
    //! @todo
    return NULL;
}

char* skl_joinpath(const char* lhs, const char* rhs){
    //! @todo
    return NULL;
}

char* skl_handle_str_from_lexer(const char* cstr){
    //! @todo
    return NULL;
} // process_str

char* skl_handle_str_to_lexer(const char* cstr){
    //! @todo
    return NULL;
}// unprocess_str

void skl_error(const char* cstr){
    //! @todo
}

// -*-
Dict new_dict(usize size, int (*hash)(void*, usize, usize)){
    //! @todo
    return NULL;
}

void* dict_search(Dict dict, void* key, usize klen){
    //! @todo
    return NULL;
}

void* dict_insert(Dict dict, void* key, usize klen, void* val, usize vlen){
    //! @todo
    return NULL;
}

void dict_remove(Dict dict, void* key, usize klen){
    //! @todo
}

Dict dict_rehash(Dict dict, usize newSize){
    //! @todo
    return NULL;
}

void dict_destroy(Dict dict){
    //! @todo
}

void dict_init_iterator(Dict dict, DictIterator iterator){
    //! @todo
}

void dict_iterator_next(DictIterator iterator){
    //! @todo
}

u32 dict_hash(void* key, usize klen, usize len){
    //! @todo
    return 0;
}