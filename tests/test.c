#include <assertion-macros/assertion-macros.h>
#include <console-colors/console-colors.c>
#include <describe/describe.c>

#include <buffer/buffer.c>
#include <dotenv-c/dotenv.c>
#include <stdlib.h>
#include <vec/vec.c>

#include "../src/liteorm.c"
#include "buffer/buffer.h"
#include "test_record.h"

int main() {

  int envLoadReturnCode = env_load(".env", true);
  if (envLoadReturnCode == -1) {
    printf("WARNING: Unable to load .env file. Ignore if you are not using the "
           ".env\n");
  }

  char *databaseName = getenv("LITEORM_DB");
  if (!databaseName) {
    printf("ERROR: Please define LITEORM_DB in the .env file or in the ENV "
           "Vars\n");
    return -1;
  }
  printf("\nTest Database Handle: %s\n", databaseName);

  sqlite3 *databaseHandle;

  int databaseConnectReturnCode = sqlite3_open(databaseName, &databaseHandle);

  LITEORM_Field one;
  LITEORM_Field two;

  LITEORM_Model model;

  LITEORM_Test_Record test_insert = {0};

  describe("LiteORM") {
    it("should handle a valid field") {
      one.name = buffer_new_with_copy("id\0");
      one.type = LITEORM_I32;
      one.offset = offsetof(LITEORM_Test_Record, id);
      one.is_pk = 1;
      one.auto_inc = 1;

      assert_str_equal(one.name->data, "id\0");
      assert_equal(one.type, LITEORM_I32);
      assert_equal(one.is_pk, 1);
      assert_equal(one.auto_inc, 1);
    }

    it("should handle a second valid field") {
      two.name = buffer_new_with_copy("test\0");
      two.type = LITEORM_TEXT;
      two.offset = offsetof(LITEORM_Test_Record, test);
      two.is_pk = 0;
      two.auto_inc = 0;

      assert_str_equal(two.name->data, "test\0");
      assert_equal(two.type, LITEORM_TEXT);
      assert_equal(two.is_pk, 0);
      assert_equal(two.auto_inc, 0);
    }

    it("should handle creating a model") {
      model.table = buffer_new_with_copy("test\0");
      vec_init(&model.fields);
      vec_push(&model.fields, &one);
      vec_push(&model.fields, &two);
      model.field_count = model.fields.length;

      assert_str_equal("test\0", model.table->data);
      assert_equal(model.field_count, 2);
      LITEORM_Field *f1 = model.fields.data[0];
      LITEORM_Field *f2 = model.fields.data[1];
      assert_equal(f1->type, LITEORM_I32);
      assert_equal(f2->type, LITEORM_TEXT);
    }

    it("should add a field to a model") {
      LITEORM_Field *f3 = liteorm_add_new_field(&model, "new_field", LITEORM_TEXT, offsetof(LITEORM_Test_Record, new_field), false, false);
      assert_str_equal(f3->name->data, "new_field");
      assert_equal(f3->type, LITEORM_TEXT);
      assert_equal(f3->is_pk, 0);
      assert_equal(f3->auto_inc, 0);
    }

    it("should create a new table") {
      LITEORM_Err createTableErrorCode =
          liteorm_create_table(databaseHandle, &model);

      assert_equal(createTableErrorCode.code, 0);
      sqlite3_free(createTableErrorCode.msg);
    }

    it("shouldn't error on a table that doesn't exists") {
      LITEORM_Model invalidModel;
      invalidModel.table = buffer_new_with_copy("invalid-table-name");

      LITEORM_Err dropTableErrorCode =
          liteorm_drop_table(databaseHandle, &invalidModel);

      assert_equal(dropTableErrorCode.code, 0);

      buffer_free(invalidModel.table);

      sqlite3_free(dropTableErrorCode.msg);
    }

    it("should create a new record") {
      test_insert.test = buffer_new_with_copy("This is a test");

      assert_str_equal(test_insert.test->data, "This is a test");
    }

    // Currently working on the binding code, the offsets may causing an issue.
    // Will need to run through gdb
    it("should insert a new record") {

      LITEORM_Err insertResultCode =
          liteorm_insert(databaseHandle, &model, &test_insert);
      assert_equal(insertResultCode.code, 0);
    }

    it("shouldn't insert a record with duplicate PRIMAY KEY") {}

    it("should identify a primary key") {
      const LITEORM_Field *primaryKey = liteorm_find_pk(&model);
      assert_str_equal(primaryKey->name->data, "id");
    }

    it("should drop a table") {
      LITEORM_Err dropTableErrorCode =
          liteorm_drop_table(databaseHandle, &model);

      assert_equal(dropTableErrorCode.code, 0);

      // TODO This handling sucks, we expect the user to know to free the
      // message specifically not a huge fan. THIS NEEDS TO BE REFACTORED TO BE
      // MORE PALLETABLE.
      sqlite3_free(dropTableErrorCode.msg);
    }

    buffer_free(one.name);
    buffer_free(two.name);
    buffer_free(model.table);
    vec_deinit(&model.fields);

    sqlite3_close(databaseHandle);
  }
}
