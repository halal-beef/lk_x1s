/*
 * Copyright (c) 2013 Google, Inc.
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lib/version.h>

#include <lk/debug.h>
#include <stdio.h>
#include <lk/init.h>

/* generated for us */
#include <buildid.h>

/* ARCH, PLATFORM, TARGET, PROJECT should be defined by the build system */

/* BUILDID is optional, and may be defined anywhere */
#ifndef BUILDID
#define BUILDID ""
#endif

const lk_version_t version = {
    .struct_version = VERSION_STRUCT_VERSION,
    .arch = ARCH,
    .platform = PLATFORM,
    .target = TARGET,
    .project = PROJECT,
    .buildid = BUILDID
};

void print_version(void)
{
    printf("version:\n");
    printf("\tarch:     %s\n", version.arch);
    printf("\tplatform: %s\n", version.platform);
    printf("\ttarget:   %s\n", version.target);
    printf("\tproject:  %s\n", version.project);
    printf("\tbuildid:  %s\n", version.buildid);
}

#if WITH_LIB_CONSOLE

#include <lk/debug.h>
#include <lib/console.h>

static int cmd_version(int argc, const cmd_args *argv)
{
    print_version();
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("version", "print version", &cmd_version)
STATIC_COMMAND_END(version);

#endif // WITH_LIB_CONSOLE

#if LK_DEBUGLEVEL > 0
// print the version string if any level of debug is set
LK_INIT_HOOK(version, (void *)&print_version, LK_INIT_LEVEL_HEAP - 1);
#endif

