// ParseData.c
// used for CURL pocessing
// requires externally defined MemStruct
#ifndef	_PARSE_DATA
#define	_PARSE_DATA 1
size_t ParseData(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemStruct *mem = (struct MemStruct *)userp;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {  // out of memory!
      puts("not enough memory to realloc");
      exit(EXIT_FAILURE);
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}
#endif	// _PARSE_DATA
