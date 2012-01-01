
#ifndef __MULTIVERSE_HEADER__
#define __MULTIVERSE_HEADER__

#include <stdio.h>
#include <stdlib.h>

#include "mvAttr.h"
#include "mvArray.h"
#include "mvIntset.h"
#include "mvCodebook.h"
#include "error.h"
#include "mvCommand.h"
#include "mvAttrSpec.h"
#include "mvQuery.h"

/*******************************/
/* Common utility functions    */
/*                             */
/* Implementation: mv_common.c */
/*******************************/

/***************************/
/* Common string functions */
/***************************/

#define STREQ(x, y) (strcmp((x), (y)) == 0)
#define MAX(x, y) ( ((x) < (y)) ? (y) : (x) )

int       mv_strhash(const char* str);

/*****************************/
/* Common data structures    */
/*                           */
/* Implementation: mv_data.c */
/*****************************/


#include "mvEntity.h"

/*******************/
/* Name-value pair */
/*******************/

void    mvAttr_update(mvAttr* dst, mvAttr* src);

/*********************/
/* Cache of entities */
/*********************/

#include "mvSession.h"

void      mv_local_end();
void      mv_local_read(mvCommand& target) throw (mvError*);
void      mv_local_execute(mvCommand& cmd);
void      mv_local_start();

#endif
