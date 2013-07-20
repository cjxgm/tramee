// vim: noet ts=4 sw=4 sts=0
// Tramee Tree
#pragma once

#include <unistd.h>
#include "pack.h"


/***************************************************
 *
 * misc
 *
 */

// typically, a traditional Chinese family tree's node is a boy.
// only how many girls are mentioned, without further details.
typedef struct Tree Tree;
struct Tree
{
	// tree structure
	Tree * parent;
	Pack * childs;

	// information
	int gender;		// 0 for male, 1 for female
	const char * name;
	const char * note;
};


/***************************************************
 *
 * manipulation
 *
 */

Tree * tree_new(Tree * parent);
void   tree_free(Tree * tree);


/***************************************************
 *
 * util
 *
 */

// the folowing 2 functions will do strdup on "name"
void tree_set_name(Tree * tree, const char * name);
void tree_set_note(Tree * tree, const char * note);

void tree_count_direct_childs(Tree * tree, int * nboy, int * ngirl);


/***************************************************
 *
 * io
 *
 */

Tree * tree_load(const char * file);
bool   tree_save(Tree * tree, const char * file);

