#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h")) == 0)
        printf("Usage: cm2lc <sourcefile>\n");
}