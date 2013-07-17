// vim: noet ts=4 sw=4 sts=0
#include "tree.h"
#include <stdio.h>
#include <string.h>


/***************************************************
 *
 * manipulation
 *
 */

Tree * tree_new(Tree * parent)
{
	create(Tree, tree);

	tree->parent = parent;
	tree->boys   = pack_new();

	tree->name  = strdup("无名氏");
	tree->wife  = strdup("无名氏");
	tree->ngirl = 0;

	if (parent) pack_add_tail(parent->boys, tree);

	return tree;
}

void tree_free(Tree * tree)
{
	Tree * pa = tree->parent;

	void del(Tree * t)
	{
		pack_walk(t->boys, Tree, tt, del(tt););
		pack_free(t->boys);
		free((void *)t->name);
		free((void *)t->wife);
		free(t);
	}
	del(tree);

	if (pa)
		pack_walk(pa->boys, Tree, tt, {
			if (tt == tree)
				pack_delete_me();
		});
}


/***************************************************
 *
 * properties
 *
 */

void tree_set_name(Tree * tree, const char * name)
{
	if (tree->name) free((void *)tree->name);
	if (name) tree->name = strdup(name);
	else tree->name = NULL;
}


void tree_set_wife(Tree * tree, const char * wife)
{
	if (tree->wife) free((void *)tree->wife);
	if (wife) tree->wife = strdup(wife);
	else tree->wife = NULL;
}


/***************************************************
 *
 * io
 *
 */

Tree * tree_load(const char * file)
{
	FILE * fp = fopen(file, "r");
	if (!fp) return NULL;

	char name[32];

	Tree * tree_read(Tree * parent)
	{
		fscanf(fp, "%s", name);
		if (name[0] == ';') return NULL;

		Tree * t = tree_new(parent);
		t->name = strdup(name);

		fscanf(fp, "%s", name);
		t->wife = strdup(name);

		fscanf(fp, "%d", &t->ngirl);

		// read all boys
		while (tree_read(t));

		return t;
	}

	Tree * tree = tree_read(NULL);

	fclose(fp);
	return tree;
}


bool tree_save(Tree * tree, const char * file)
{
	FILE * fp = fopen(file, "w");
	if (!fp) return true;

	void tree_write(Tree * tree)
	{
		fprintf(fp, "%s %s %d\n", tree->name, tree->wife, tree->ngirl);
		pack_walk(tree->boys, Tree, t, tree_write(t););
		fprintf(fp, ";\n");
	}

	tree_write(tree);

	fclose(fp);
	return false;
}

