#ifndef _PARSER_H
#define _PARSER_H

typedef struct {
  int whatever; // FIXME
} sl1_t;

typedef struct {
  int version;
} ctb_t;

int    parse_sl1_file  (sl1_t *sl1, const char *in);
int    parse_ctb_file  (ctb_t *ctb, const char *in);

#endif
