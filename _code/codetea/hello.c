#include <stdio.h>
#include <string.h> /* TODO: useless include */

static int _bar(const char *foo)
{
    return printf("%s\n", foo);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *foo = "Hello World";

    if (argc > 1)
        foo = argv[1];

    ret = _bar(foo);
    printf("execute %s\n", ret > 0 ? "success" : "fail");

    return ret > 0 ? 0 : -1;
}