#include <stdio.h>
#include <string.h> /* TODO: useless include */

static int _bar(const char *foo)
{
    return printf("%s\n", foo);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    const char *foo = "Hello World";

    ret = _bar(foo);

    printf("execute %s\n", ret > 0 ? "success" : "fail");

    return ret > 0 ? 0 : -1;
}