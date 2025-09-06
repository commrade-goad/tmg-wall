#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "nob.h"
#include <stdbool.h>

enum BuildType {
    DEBUG,
    RELEASE
};

static const enum BuildType bt = RELEASE;

bool build_deps(Cmd *cmd) {
    static const char *out = "stb_image.o";
    static const char *in = "stb_image.h";
    if (nob_needs_rebuild1(out, in)) {
        nob_cc(cmd);
        nob_cc_flags(cmd);
        nob_cc_output(cmd, out);
        cmd_append(cmd, "-lm");
        cmd_append(cmd, "-x", "c");
        cmd_append(cmd, "-c");
        switch (bt) {
            case DEBUG:
                cmd_append(cmd, "-ggdb");
                break;
            case RELEASE:
                cmd_append(cmd, "-O2");
                break;
            default:
                break;
        }
        cmd_append(cmd, "-DSTB_IMAGE_IMPLEMENTATION");
        nob_cc_inputs(cmd, in);
        if (!nob_cmd_run_sync_and_reset(cmd)) return false;
    }
    return true;
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Cmd cmd = {0};

    if (!build_deps(&cmd)) return 1;

    cmd_append(&cmd, "g++");
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
    nob_cc_inputs(&cmd, "main.cpp", "helper.cpp", "argparser.cpp", "stb_image.o");

    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    return 0;
}
