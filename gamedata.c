#include <stdlib.h>
#include <time.h>

int rand_bounded_int(int min, int max) {
  srand(time(NULL));
  return rand() % (max - min + 1) + min;
}
