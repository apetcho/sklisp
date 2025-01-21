#include "sklisp.h"
#include<errno.h>
#include<string.h>
#include<assert.h>

// -*-
void* skl_alloc(usize size){
    void* ptr = calloc(1, size);
    if(ptr==NULL){
        char msg[512] = {0};
        snprintf(msg, sizeof(msg)-1, "Error: out of memory: %s\n", strerror(errno));
        fputs(msg, stderr);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* skl_realloc(void* ptr, usize size){
    void* mem = realloc(ptr, size);
    if(mem==NULL){
        char msg[512] = {0};
        snprintf(msg, sizeof(msg)-1, "Error: out of memory: %s\n", strerror(errno));
        fputs(msg, stderr);
        exit(EXIT_FAILURE);
    }
    return mem;
}

void skl_free(void* ptr){
    if(ptr){
        free(ptr);
    }
    ptr = NULL;
}

// -*-
char* skl_strdup(const char* str){
    char* result = skl_alloc(strlen(str)+1);
    strcpy(result, str);
    return result;
}

char* skl_joinpath(const char* lhs, const char* rhs){
    char* result = skl_alloc(strlen(lhs)+strlen(rhs)+2);
    strcpy(result, lhs);
    result[strlen(lhs)] = '/';
    strcpy(result+strlen(lhs)+1, rhs);
    return result;
}

char* skl_handle_str_from_lexer(const char* cstr){
    char* result = skl_strdup(cstr+1); // trim leading quote
    char* eq = NULL;
    char* ptr = result;
    while((eq = strchr(ptr, '\\')) != NULL){
        *eq = *(eq + 1);
        // replace \ with next character
        memmove(eq+1, eq+2, strlen(eq)+1);
        ptr = eq + 1;
    }
    // remove the trailing quote
    result[strlen(result)-1] = '\0';
    return result;
} // process_str

// -*-
char* skl_handle_str_to_lexer(const char* cstr){
    char* ptr = (char*)cstr;
    usize len = 0;
    while(*ptr != '\0'){
        if(*ptr=='\\' || *ptr == '"'){ len++; }
        ptr++;
    }

    // Trow extra for quotes and one for each character that need escaping
    char* result = skl_alloc(strlen(cstr)+len+2+1);
    strcpy(result+1, cstr);

    // Place backquotes
    ptr = result + 1;
    while(*ptr != '\0'){
        if(*ptr == '\\' || *ptr == '"'){
            memmove(ptr+1, ptr, strlen(ptr)+1);
            *ptr = '\\';
            ptr++;
        }
        ptr++;
    }

    // surrounding quotes
    result[0] = '"';
    result[strlen(result)+1] = '\0';
    result[strlen(result)] = '"';
    return result;
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