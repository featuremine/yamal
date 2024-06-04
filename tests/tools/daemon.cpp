/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file daemon.cpp
 * @date 10 Jan 2022
 * @brief File contains tests for YTP daemon tool
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

using namespace std;

struct state {
    const char *name;
    const char *link;
    bool empty;
} states[] = {
    /* 0 */ {NULL, NULL, false},
    /* 1 */ {"daemon.test.ytp", NULL, false},
    /* 2 */ {"daemon.test.ytp", "daemon.link.ytp", false},
    /* 3 */ {"daemon.test.ytp", "daemon.link.ytp", true},
    /* 4 */ {"daemon.test.ytp", "daemon.link2.ytp", true},
};

unsigned tran_mtrx[4][5] = {
    { 0,  1,  8, 10,  0},
    { 7,  0,  2, 12,  0},
    { 9,  6,  0,  3,  0},
    {13, 11,  5,  0,  4},
};

bool streq(const char *a, const char *b) {
    return a == b || (a && b && strcmp(a, b) == 0);
}

struct state *transitions(size_t *size) {
    unsigned sz = 0;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 5; ++c) {
            sz = max(tran_mtrx[r][c], sz);
        }
    }
    struct state *trans = (struct state *)calloc(sz, sizeof(struct state));
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 5; ++c) {
            if (!tran_mtrx[r][c])
                continue;
            trans[tran_mtrx[r][c] - 1] = states[c];
        }
    }
    *size = sz;
    return trans;
}

TEST(daemon, state_transition)
{
    size_t sz = 0;
    struct state *cur = &states[0];
    struct state *trans = transitions(&sz);
    for (size_t i = 0; i < sz; ++i) {
        struct state *next = &trans[i];
        printf("==============================================\n");
        printf("cur name %s link %s %s\n",
               cur->name ? cur->name : "NULL",
               cur->link ? cur->link : "NULL",
               cur->empty ? "empty" : "");
        printf("next name %s link %s %s\n",
               next->name ? next->name : "NULL",
               next->link ? next->link : "NULL",
               next->empty ? "empty" : "");

        if (!streq(cur->link, next->link)) {
            if (cur->link && !cur->empty) {
                printf("unlink %s\n", cur->link);
            }
            if (next->link && !next->empty) {
                printf("create yamal file %s\n", next->link);
            }
        }
        if (!streq(cur->name, next->name)) {
            if (cur->name) {
                printf("unlink %s\n", cur->name);
            }
            if (next->name) {
                if (next->link) {
                    printf("create link %s -> %s\n", next->name, next->link);
                } else {
                    printf("create yamal file %s\n", next->name);
                }
            }
        } else {
            if (!cur->link && next->link) {
                printf("unlink %s\n", cur->name);
                printf("create link %s -> %s\n", next->name, next->link);
            }
        }
        cur = next;
    }
    free(trans);
}

GTEST_API_ int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
