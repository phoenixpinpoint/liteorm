#include "liteorm.h"
#include "buffer/buffer.h"

#include <stdio.h>
#include <stdlib.h>

static inline void *field_pointer(const void *record,
                                  const LITEORM_Field *field) {
  return (void *)((const unsigned char *)record + field->offset);
}

static void bind_one(sqlite3_stmt *statementHandle, int idx,
                     const LITEORM_Field *field, const void *ptr) {
  switch (field->type & 0xFF) {
  case LITEORM_I32:
    sqlite3_bind_int(statementHandle, idx, *(const int *)ptr);
    break;
  case LITEORM_I64:
    sqlite3_bind_int64(statementHandle, idx, *(const long long *)ptr);
    break;
  case LITEORM_BOOL:
    sqlite3_bind_int(statementHandle, idx, *(const int *)ptr != 0);
    break;
  case LITEORM_REAL:
    sqlite3_bind_double(statementHandle, idx, *(const double *)ptr);
      break;
    case LITEORM_TEXT:
    if (field->size > 0) {
      sqlite3_bind_text(statementHandle, idx, (const char *)ptr, -1,
                        SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_text(statementHandle, idx, *(char *const *)ptr, -1,
                        SQLITE_TRANSIENT);
    }
    break;
  default:
    sqlite3_bind_null(statementHandle, idx);
    break;
  }
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
    LITEORM_Field *f = (LITEORM_Field*)model->fields.data[i];

    if (f->is_pk) {
      return f;
    }
  }
  return NULL;
}

LITEORM_Field *liteorm_add_new_field(LITEORM_Model *model, char* name, unsigned type, size_t offset, bool is_pk, bool auto_inc) {
  //if(!model->fields) {
  //  return -1;
  //}

  LITEORM_Field *field = malloc(sizeof(LITEORM_Field));
  if (!field) {
    return (LITEORM_Field*)-1;
  }
  field->name = buffer_new_with_copy(name);
  field->type = type;
  field->offset = offset;
  field->is_pk = is_pk;
  field->auto_inc = auto_inc;

  vec_push(&model->fields, field);

  return field;
}


LITEORM_Err liteorm_create_table(sqlite3 *databaseHandle,
                                 const LITEORM_Model *model) {
  buffer_t *sqlQueryBuffer = buffer_new();
  buffer_append(sqlQueryBuffer, "CREATE TABLE IF NOT EXISTS `");
  buffer_append(sqlQueryBuffer, model->table->data);
  buffer_append(sqlQueryBuffer, "` (");
  for (int i = 0; i < model->field_count; i++) {
    const LITEORM_Field *field = model->fields.data[i];
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

// Need to fix offset or come up with better solution for the bind.
LITEORM_Err liteorm_insert(sqlite3 *databaseHandle, const LITEORM_Model *model,
                           void *record) {
  buffer_t *sqlQueryBuffer = buffer_new();
  buffer_appendf(sqlQueryBuffer, "INSERT INTO %s VALUES (", model->table->data);
  for (int i = 0; i < model->field_count; i++) {
    buffer_appendf(sqlQueryBuffer, "%s?", i ? "," : "");
  }
  buffer_append(sqlQueryBuffer, ");");

  sqlite3_stmt *statementHandle = NULL;
  int insertResultCode = sqlite3_prepare_v2(
      databaseHandle, sqlQueryBuffer->data, -1, &statementHandle, NULL);
  if (insertResultCode != SQLITE_OK) {
    return (LITEORM_Err){insertResultCode, sqlite3_errmsg(databaseHandle)};
  }

  for (int i = 0; i < model->field_count; i++) {
    // TODO Add error handling/warning
    bind_one(statementHandle, i + 1, model->fields.data[i],
             field_pointer(record, model->fields.data[i]));
  }

  insertResultCode = sqlite3_step(statementHandle);

  sqlite3_finalize(statementHandle);
  if (insertResultCode != SQLITE_DONE) {
    return (LITEORM_Err){insertResultCode, sqlite3_errmsg(databaseHandle)};
  }

  const LITEORM_Field *primaryKey = liteorm_find_pk(model);
  if (primaryKey && primaryKey->auto_inc) {
    *(long long *)field_pointer(record, primaryKey) =
        sqlite3_last_insert_rowid(databaseHandle);
  }

  return LITEORM_OK;
}

LITEORM_Err liteorm_drop_table(sqlite3 *databaseHandle,
                               const LITEORM_Model *model) {
  buffer_t *sqlQueryBuffer = buffer_new_with_copy("DROP TABLE IF EXISTS `");
  buffer_append(sqlQueryBuffer, model->table->data);
  buffer_append(sqlQueryBuffer, "`;");

  char *errorMessage = NULL;
  int returnCode = sqlite3_exec(databaseHandle, sqlQueryBuffer->data, NULL,
                                NULL, &errorMessage);

  buffer_free(sqlQueryBuffer);

  LITEORM_Err error = {returnCode, errorMessage};

  return returnCode == SQLITE_OK ? LITEORM_OK : error;
}
