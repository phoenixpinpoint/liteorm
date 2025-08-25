#include "liteorm.h"

#include <stdio.h>
#include <stdlib.h>

static inline void *field_pointer(const void *record,
                                  const LITEORM_Field *field) {
  return (void *)((const unsigned char *)record + field->offset);
}

const LITEORM_Field *liteorm_find_pk(const LITEORM_Model *model) {
  for (int i = 0; i < model->field_count; i++) {
    if (model->fields[i].is_pk) {
      return &model->fields[i];
    }
  }
  return NULL;
}
