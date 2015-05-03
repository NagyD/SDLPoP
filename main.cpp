#include "common.h"

int main(int argc, char* argv[]) {
    g_argc = argc;
    g_argv = argv;
    pop_main();
    return 0;
}