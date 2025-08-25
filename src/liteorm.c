#include "liteorm.h"
#include "buffer/buffer.h"

#include <stdio.h>
#include <stdlib.h>

static inline void *field_pointer(const void *record,
                                  const LITEORM_Field *field) {
  return (void *)((const unsigned char *)record + field->offset);
}

static const char *sqlite_type(unsigned type) {
  switch (type & 0xFF) {
  case LITEORM_I32:
  case LITEORM_I64:
  case LITEORM_BOOL:
    return "INTEGER";
  case LITEORM_REAL:
    return "REAL";
  case LITEORM_TEXT:
    return "TEXT";
  case LITEORM_BLOB:
    return "BLOB";
  default:
    return "TEXT";
  }
}

const LITEORM_Field *liteorm_find_pk(const LITEORM_Model *model) {
  for (int i = 0; i < model->field_count; i++) {
    if (model->fields.data[i].is_pk) {
      return &model->fields.data[i];
    }
  }
  return NULL;
}

LITEORM_Err liteorm_create_table(sqlite3 *databaseHandle,
                                 const LITEORM_Model *model) {
  buffer_t *sqlQueryBuffer = buffer_new();
  buffer_append(sqlQueryBuffer, "CREATE TABLE IF NOT EXISTS ");
  buffer_append(sqlQueryBuffer, model->table->data);
  buffer_append(sqlQueryBuffer, " (");
  for (int i = 0; i < model->field_count; i++) {
    const LITEORM_Field *field = &model->fields.data[i];
    buffer_append(sqlQueryBuffer, field->name->data);
    buffer_append(sqlQueryBuffer, " ");
    buffer_append(sqlQueryBuffer, sqlite_type(field->type));
    if (field->is_pk) {
      buffer_append(sqlQueryBuffer, " PRIMARY KEY");
      if (field->auto_inc) {
        buffer_append(sqlQueryBuffer, " AUTOINCREMENT");
      }
    }
    if (i < model->field_count - 1) {
      buffer_append(sqlQueryBuffer, ", ");
    }
  }
  buffer_append(sqlQueryBuffer, ");");
  char *errorMessage = NULL;
  int returnCode = sqlite3_exec(databaseHandle, sqlQueryBuffer->data, NULL,
                                NULL, &errorMessage);
  buffer_free(sqlQueryBuffer);

  LITEORM_Err error = {returnCode, errorMessage};

  if (errorMessage) {
    sqlite3_free(errorMessage);
  }
  return returnCode == SQLITE_OK ? LITEORM_OK : error;
}
