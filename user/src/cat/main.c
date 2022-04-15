#include <stdio.h>

char buf[512];
void
cat(int fd)
{
    int n;

    while ((n = read(fd, buf, sizeof(buf))) > 0)
        write(1, buf, n);

    printf("\n");
    
    if (n < 0) {
        printf(1, "cat: read error\n");
        exit(0);
    }
}

int
main(int argc, char* argv[])
{
    /* TODO: Your code here. */
    int fd, i;
    if (argc <= 1) {
        cat(0);
        exit(0);
    }

    for (i = 1; i < argc; i++) {
        if ((fd = open(argv[i], 0)) < 0) {
            fprintf(stderr, "cat: cannot open %s\n", argv[i]);
            exit(0);
        }
        cat(fd);
        close(fd);
    }
    exit(0);
}