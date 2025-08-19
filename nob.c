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

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cc_output(&cmd, "tmg-wall");
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
    nob_cc_inputs(&cmd, PREFIX"main.c", PREFIX"helper.c");

    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}
