#include "gamedata.h"
#include <assert.h>
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>

Test(basic, first) { assert(1); }

Test(Random, repeats) {
  int min = 100;
  int max = 200;
  for (int i = 0; i < 100000; ++i) {
    int r_int = rand_bounded_int(min, max);
    cr_assert_lt(r_int, max, "max violation. expected %d < %d", r_int, max);
    cr_assert_gt(r_int, min, "min violation. expected %d > %d", r_int, min);
  }
}
