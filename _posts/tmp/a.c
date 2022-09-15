#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void av_free(void *p)
{
    free(p);
}

void av_freep1(void *arg)
{
    void **val = (void **) arg;

    printf("val=0x%08X, arg=0x%08X\n", *val, arg);

    av_free(*val);
    *val = NULL;

    printf("val=0x%08X, arg=0x%08X\n", *val, arg);    
}

void av_freep(void *arg)
{
    void *val;

    memcpy(&val, arg, sizeof(val));
    
    printf("val=0x%08X, arg=0x%08X\n", val, arg);
    
    memcpy(arg, &(void *){ NULL }, sizeof(val));
    
    printf("val=0x%08X, arg=0x%08X\n", val, arg);
    
    av_free(val);
}

int main()
{
    char *buffer = (char *) malloc(0x80);
    
    if (!buffer) {
        printf("malloc(%d) failed\n", 0x80);
        return -1;
    }
    
    printf("buffer=0x%08X\n", buffer);
    printf("&buffer=0x%08X\n", &buffer);

    av_freep1(&buffer);

    printf("buffer=0x%08X\n", buffer);
    return 0;
}
