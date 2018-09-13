/*

Calee Fulling
Started with: n/a
Purpose: CS471 HW 2b (xv6 program)
Program description: takes either just a decimal number or a decimal
number and a base from the user and converts the number to the different
base. The new base is defaulted to 2 unless another base is specified by
the user.

The base must be greater than 2 and less than 36.
*/

#include "types.h"
#include "stat.h"
#include "user.h"

void convertBase(int num, int base);

int main(int argc, char * argv[]) {

  if (argc < 2) {                                         // stderr, no number given
    printf(2, "Usage: number || Usage: number base\n");
    exit();
  }

  int base;
  int num = atoi(argv[1]);

  if (argc == 2)                                          // if base was not declared, defaults to base 2
    base = 2;

  if (argc == 3)                                          // base was declared
    base = atoi(argv[2]);                                 // base is now user's base

  if (base == 1) {
    for (int i = 0; i < num; i++) {
      printf(1, "|");
    }
    printf(1, "\n");
    exit();
  }
  if (base > 36) {                                        // stderr, invalid base
    printf(2, "Usage: base cannot be more than 36\n");
    exit();
  }

  if (num == 0) {                                         // user's number is 0, no need to convert
    printf(1, "%d\n", num);
    exit();
  }

  convertBase(num, base);
  printf(1,"\n");
  exit();
}

void convertBase(int num, int base) {                     // computing & printing new value
  int rem = num % base;                                   // mods & places remainder into rem
  if (num == 0)
    return;
  convertBase(num / base, base);
  if (rem < 10)                                            // printing rem as numbers
    printf(1, "%d", rem);
  else                                                    // printing rem as characters
    printf(1, "%c", 'A' + (rem - 10));
}
