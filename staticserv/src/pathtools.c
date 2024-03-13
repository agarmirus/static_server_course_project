#include "pathtools.h"

filetype_t file_type(const char *path)
{
    char *type_str = strrchr(path, '.');

    if (!type_str)
        return NONE;

    ++type_str;

    if (strcmp(type_str, FILE_TYPE_TXT) == 0)
        return TXT;

    if (strcmp(type_str, FILE_TYPE_HTML) == 0)
        return HTML;

    if (strcmp(type_str, FILE_TYPE_CSS) == 0)
        return CSS;

    if (strcmp(type_str, FILE_TYPE_JS) == 0)
        return JS;

    if (strcmp(type_str, FILE_TYPE_PNG) == 0)
        return PNG;

    if (strcmp(type_str, FILE_TYPE_JPG) == 0)
        return JPG;

    if (strcmp(type_str, FILE_TYPE_JPEG) == 0)
        return JPEG;

    if (strcmp(type_str, FILE_TYPE_SWF) == 0)
        return SWF;

    if (strcmp(type_str, FILE_TYPE_GIF) == 0)
        return GIF;

    return NONE;
}

size_t file_size(const int fd)
{
    lseek(fd, 0, SEEK_SET);

    size_t size = (size_t)lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);

    return size;
}

bool path_is_inside(const char *path)
{
    char *path_copy = malloc((strlen(path) + 1) * sizeof(char));

    if (!path_copy)
    {
        return false;
    }

    memcpy(path_copy, path, (strlen(path) + 1) * sizeof(char));

    int dir_level = 0;

    char *dir_str = strtok(path_copy, "/");

    while (dir_str)
    {
        if (strcmp(dir_str, ".") != 0)
        {
            if (strcmp(dir_str, "~") == 0)
                return false;

            dir_level += strcmp(dir_str, "..") == 0 ? -1 : 1;
        }

        dir_str = strtok(NULL, "/");
    }

    return dir_level >= 0;
}
