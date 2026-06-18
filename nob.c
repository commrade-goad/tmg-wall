#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "nob.h"

#define PREFIX "src/"

enum BuildType {
    DEBUG,
    RELEASE
};

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};
    enum BuildType bt = RELEASE;
    bool as_lib = false;

    // Parse flags first
    if (argc >= 2 && strcmp(argv[1], "--so") == 0) {
        as_lib = true;
    }

    // Initialize compiler base command
    nob_cc(&cmd);
    nob_cc_flags(&cmd);

    // Add target-specific compilation flags
    if (as_lib) {
        cmd_append(&cmd, "-fPIC", "-shared", "-DAS_LIB=1");
    }

    // Add math library optimization flags
    cmd_append(&cmd, "-lm");
    switch (bt) {
        case DEBUG:
            cmd_append(&cmd, "-ggdb");
            break;
        case RELEASE:
            cmd_append(&cmd, "-O2");
            break;
        default:
            break;
    }

    // Append inputs
    nob_cc_inputs(&cmd, PREFIX"main.c", PREFIX"helper.c");

    // Assign final binary destination safely
    if (as_lib) {
        nob_cc_output(&cmd, "libtmgwall.so");
    } else {
        nob_cc_output(&cmd, "tmg-wall");
    }

    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
