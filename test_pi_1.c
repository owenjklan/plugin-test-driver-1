/* First simple test plugin.
   This provides a very simple extension function and the name.
   
   Written by Owen Klan  -  10th December, 2015 */

#include <stdio.h>

#include "dltutorial.h"

char plugin_name[PLUGIN_NAME_LEN] = "Test1";

void func() {
    printf("Hello from Test1. This is the first test.\n");
}
