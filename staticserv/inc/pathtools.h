#ifndef PATHTOOLS_H
#define PATHTOOLS_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define FILE_TYPE_HTML "html"
#define FILE_TYPE_CSS "css"
#define FILE_TYPE_JS "js"
#define FILE_TYPE_PNG "png"
#define FILE_TYPE_JPG "jpg"
#define FILE_TYPE_JPEG "jpeg"
#define FILE_TYPE_SWF "swf"
#define FILE_TYPE_GIF "gif"
#define FILE_TYPE_TXT "txt"

typedef enum
{
    NONE,
    TXT,
    HTML,
    CSS,
    JS,
    PNG,
    JPG,
    JPEG,
    SWF,
    GIF
} filetype_t;

filetype_t file_type(const char *);

size_t file_size(const int);

bool path_is_inside(const char *);

#endif
