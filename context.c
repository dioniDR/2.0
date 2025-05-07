#include <stdio.h>
#include <stdlib.h>

void append_to_context(const char* cmd, const char* output) {
    FILE *f = fopen("context.txt", "a");
    fprintf(f, "user\t%s\n", cmd);
    fprintf(f, "assistant\t%s\n", output);
    fclose(f);
}

void load_context() {
    FILE *f = fopen("context.txt", "a");
    if (f) fclose(f);
}
