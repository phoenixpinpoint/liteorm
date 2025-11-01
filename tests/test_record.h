#ifndef TEST_RECORD_H
#define TEST_RECORD_H

#include <buffer/buffer.h>
#include <stdint.h>

typedef struct {
  int64_t id;
  buffer_t *test;
  buffer_t *new_field;
} LITEORM_Test_Record;

#endif
