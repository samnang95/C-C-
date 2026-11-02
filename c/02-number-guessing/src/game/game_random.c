#include "game/game.h"

#include <stdlib.h>

int game_random_in_range(int min, int max) {
    if (min > max) {
        int tmp = min;
        min = max;
        max = tmp;
    }
    if (min == max) {
        return min;
    }

    unsigned long long span =
        (unsigned long long)((long long)max - (long long)min) + 1ULL;
    unsigned long long rand_space = (unsigned long long)RAND_MAX + 1ULL;

    if (span >= rand_space) {
        return (int)((long long)min + (long long)((unsigned long long)rand() % span));
    }

    /* Rejection sampling eliminates modulo bias when span does not divide rand_space */
    unsigned long long limit = rand_space - (rand_space % span);
    unsigned long long r = 0;
    do {
        r = (unsigned long long)rand();
    } while (r >= limit);

    return (int)((long long)min + (long long)(r % span));
}

