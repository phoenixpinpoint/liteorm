#include <assertion-macros/assertion-macros.h>
#include <console-colors/console-colors.c>
#include <describe/describe.c>

#include <buffer/buffer.c>
#include <vec/vec.c>

#include "../src/liteorm.h"

int main() {

  LITEORM_Field one;
  LITEORM_Field two;

  LITEORM_Model model;

  describe("LiteORM") {
    it("should handle a valid field") {
      one.name = buffer_new_with_copy("id\0");
      one.type = LITEORM_I32;
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
      vec_push(&model.fields, one);
      vec_push(&model.fields, two);
      model.field_count = model.fields.length;

      assert_str_equal("test\0", model.table->data);
      assert_equal(model.field_count, 2);
      assert_equal(model.fields.data[0].type, LITEORM_I32);
      assert_equal(model.fields.data[1].type, LITEORM_TEXT);
    }

    buffer_free(one.name);
    buffer_free(two.name);
    buffer_free(model.table);
    vec_deinit(&model.fields);
  }
}
