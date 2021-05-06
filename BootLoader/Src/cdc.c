/*
 * cdc.c
 *
 *  Created on: 2020. 12. 11.
 *      Author: baram
 */


#include "cdc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>


static bool is_init = false;

bool cdcInit(void)
{
  bool ret = true;


  is_init = true;

  return ret;
}

bool cdcIsInit(void)
{
  return is_init;
}
