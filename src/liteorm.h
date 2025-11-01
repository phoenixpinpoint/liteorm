#ifndef LITEORM_H
#define LITEORM_H

#include <stdbool.h>
#include <stddef.h>

#include <sqlite3.h>

#include <buffer/buffer.h>
#include <vec/vec.h>

typedef enum {
  LITEORM_I32,
  LITEORM_I64,
  LITEORM_REAL,
  LITEORM_TEXT,
  LITEORM_BLOB,
  LITEORM_BOOL,
  LITEORM_NULLABE
} LITEORM_Type;

typedef struct {
  buffer_t *name; // column name
  unsigned type;
  size_t offset;
  size_t size;
  bool is_pk;
  bool auto_inc;
} LITEORM_Field;

typedef vec_t(LITEORM_Field) vec_liteorm_field_t;

typedef struct {
  buffer_t *table;
  vec_void_t fields;
  int field_count;
  size_t struct_size;
} LITEORM_Model;

typedef struct {
  int code;
  char *msg;
} LITEORM_Err;

typedef struct {
  buffer_t *sql;
  void (*bind_fn)(sqlite3_stmt *statement, int *bind_index, void *user_ctx);
  void *user_ctx;
} LITEORM_Where;

typedef int (*liteorm_row_callback)(void *user_ctx, void *record);

static void bind_one(sqlite3_stmt *statementHandle, int idx,
                     const LITEORM_Field *field, const void *ptr);

#define LITEORM_OK                                                             \
  (LITEORM_Err) { 0, NULL }

const LITEORM_Field *liteorm_find_pk(const LITEORM_Model *model);

LITEORM_Err liteorm_create_table(sqlite3 *databaseHandle,
                                 const LITEORM_Model *model);

LITEORM_Err liteorm_insert(sqlite3 *databaseHandle, const LITEORM_Model *model,
                           void *record);

LITEORM_Err liteorm_update_by_pk(sqlite3 *datbaseHandle,
                                 const LITEORM_Model *model,
                                 const void *record);

LITEORM_Err liteorm_delete_by_pk(sqlite3 *databaseHandle,
                                 const LITEORM_Model *model,
                                 const void *record);

LITEORM_Err liteorm_select_all(sqlite3 *databaseHandle,
                               const LITEORM_Model *model,
                               liteorm_row_callback callback, void *user_ctx);

LITEORM_Err liteorm_select_where(sqlite3 *databaseHandle, LITEORM_Model *model,
                                 const LITEORM_Where *wherequery,
                                 liteorm_row_callback callback, void *user_ctx);

LITEORM_Err liteorm_drop_table(sqlite3 *datbaseHandle,
                               const LITEORM_Model *model);

const LITEORM_Field *liteorm_find_pk(const LITEORM_Model *model);

#endif
