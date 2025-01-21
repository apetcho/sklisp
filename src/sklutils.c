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
    fputs(cstr, stderr);
}

// -*-
Dict new_dict(usize size, int (*hash)(void*, usize, usize)){
    Dict dict = (Dict)skl_alloc(sizeof(*dict));
    dict->bucket = (Entry*)skl_alloc(sizeof(Entry)*size);
    dict->len = 0;
    dict->size = size;
    for(usize i=0; i < size; i++){
        dict->bucket[i] = NULL;
    }
    if(hash==NULL){ dict->hashfn = dict_hash;}
    else{ dict->hashfn = hash; }
    return dict;
}

// -*-
void* dict_search(Dict dict, void* key, usize klen){
    int idx = dict_hash(key, klen, dict->size);
    if(dict->bucket[idx]==NULL){ return NULL; }

    Entry entry = dict->bucket[idx];
    while(entry != NULL){
        if(entry->klen==klen){
            if(memcmp(key, entry->key, klen)==0){
                return entry->val;
            }
        }
        entry = entry->next;
    }
    return NULL;
}

// -*-
void* dict_insert(Dict dict, void* key, usize klen, void* val, usize vlen){
    int idx = dict_hash(key, klen, dict->size);
    Entry entry = dict->bucket[idx];
    Entry last = NULL;

    // search for an existing key
    while(entry != NULL){
        if(entry->klen==klen){
            if(memcmp(key, entry->key, klen)==0){
                entry->val = val;
                entry->vlen = vlen;
                return entry;
            }
        }
        last = entry;
        entry = entry->next;
    }

    Entry node = (Entry)skl_alloc(sizeof(*node));
    node->key = key;
    node->val = val;
    node->klen = klen;
    node->vlen = vlen;
    node->next = NULL;

    if(last != NULL){
        last->next = node;
    }else{
        dict->bucket[idx] = node;
    }
    dict->len++;
    return node->val;
}

void dict_remove(Dict dict, void* key, usize klen){
    Entry last;
    Entry entry;
    int idx = dict_hash(key, klen, dict->size);
    entry = dict->bucket[idx];
    last = NULL;

    while(entry != NULL){
        if(entry->klen==klen){
            if(memcmp(key, entry->key, klen)==0){
                if(last != NULL){ last->next = entry->next; }
                else{ dict->bucket[idx] = entry->next; }
                skl_free(entry);
                break;
            }
        }
        last = entry;
        entry = entry->next;
    }
}

Dict dict_rehash(Dict dict, usize newSize){
    Dict table = new_dict(newSize, dict->hashfn);
    if(table==NULL){ return NULL; }
    struct dictIterator iterator;
    dict_init_iterator(dict, &iterator);
    assert(table != NULL);
    for(; iterator.key != NULL; dict_iterator_next(&iterator)){
        dict_insert(table, iterator.key, iterator.klen, iterator.val, iterator.vlen);
    }
    dict_destroy(dict);
    return table;
}

void dict_destroy(Dict dict){
    Entry entry;
    Entry last;
    for(int i=0; i < (int)dict->size; i++){
        entry = dict->bucket[i];
        while(entry != NULL){
            // destroy node
            last = entry;
            entry = entry->next;
            skl_free(last);
        }
    }
    skl_free(dict->bucket);
    skl_free(dict);
}

void dict_init_iterator(Dict dict, DictIterator iterator){
    //! @todo
}

void dict_iterator_next(DictIterator iterator){
    //! @todo
}

int dict_hash(void* key, usize klen, usize len){
    //! @todo
    return 0;
}