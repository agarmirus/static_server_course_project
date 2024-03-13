#ifndef FDSARR_H
#define FDSARR_H

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_FDSARR_BUF_SIZE 100

typedef struct
{
    size_t buf_size;
    size_t length;

    int *arr;
} fdsarr_t;

fdsarr_t fdsarr_alloc(const size_t);

void fdsarr_free(fdsarr_t *const);

bool fdsarr_is_null(const fdsarr_t *const);

int fdsarr_append(fdsarr_t *const, const int);

void fdsarr_remove(fdsarr_t *const, const int);

void fdsarr_close(const fdsarr_t *const);

size_t fdsarr_length(const fdsarr_t *const);

int fdsarr_max(const fdsarr_t *const);

#endif
