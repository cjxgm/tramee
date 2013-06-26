// vim: noet ts=4 sw=4 sts=0
// Tramee Tree
#ifndef __TRAMEE$TREE__
#define __TRAMEE$TREE__

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
	Pack * boys;

	// information
	const char * name;
	const char * wife;
	int ngirl;
};


/***************************************************
 *
 * manipulation
 *
 */

Tree * tree_new(Tree * parent);


/***************************************************
 *
 * io
 *
 */

Tree * tree_load(const char * file);
bool   tree_save(Tree * tree, const char * file);

#endif

