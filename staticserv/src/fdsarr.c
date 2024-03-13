#include "fdsarr.h"

fdsarr_t fdsarr_alloc(const size_t buf_size)
{
    fdsarr_t fdsarr = {0, 0, NULL};

    int *arr = calloc(buf_size, sizeof(int));

    if (arr)
    {
        fdsarr.buf_size = buf_size;
        fdsarr.arr = arr;
    }

    return fdsarr;
}

void fdsarr_free(fdsarr_t *const fdsarr)
{
    free(fdsarr->arr);

    fdsarr->arr = NULL;
    fdsarr->length = fdsarr->buf_size = 0;
}

bool fdsarr_is_null(const fdsarr_t *const fdsarr)
{
    return !(bool)fdsarr->arr;
}

int fdsarr_append(fdsarr_t *const fdsarr, const int elem)
{
    if (fdsarr->length == fdsarr->buf_size)
    {
        int *realloced_arr = realloc(fdsarr->arr, 2 * fdsarr->length);

        if (!realloced_arr)
            return EXIT_FAILURE;

        fdsarr->arr = realloced_arr;
        fdsarr->buf_size *= 2;
    }

    fdsarr->arr[fdsarr->length++] = elem;

    return EXIT_SUCCESS;
}

void fdsarr_remove(fdsarr_t *const fdsarr, const int elem)
{
    size_t i = 0;

    for (; i < fdsarr->length && fdsarr->arr[i] != elem; ++i);

    if (i < fdsarr->length)
    {
        for (size_t j = i; j < fdsarr->length - 1; ++j)
            fdsarr->arr[j] = fdsarr->arr[j + 1];

        --fdsarr->length;
    }
}

void fdsarr_close(const fdsarr_t *const fdsarr)
{
    for (size_t i = 0; i < fdsarr->length; ++i)
        close(fdsarr->arr[i]);
}

size_t fdsarr_length(const fdsarr_t *const fdsarr)
{
    return fdsarr->length;
}

int fdsarr_max(const fdsarr_t *const fdsarr)
{
    if (fdsarr->length == 0)
        return -1;

    int max_fd = fdsarr->arr[0];

    for (size_t i = 1; i < fdsarr->length; ++i)
        if (max_fd < fdsarr->arr[i])
            max_fd = fdsarr->arr[i];
    
    return max_fd;
}
