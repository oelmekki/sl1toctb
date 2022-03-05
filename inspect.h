#ifndef _INSPECT_H
#define _INSPECT_H

int inspect               (const char *in);
int show_preview_image    (const char *filename, int image_type, const char *dir);
int export_layers         (const char *filename, const char *dir);

#endif
