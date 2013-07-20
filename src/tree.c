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
	tree->childs = pack_new();

	tree->gender = 0;
	tree->name   = strdup("");
	tree->note   = strdup("");

	if (parent) pack_add_tail(parent->childs, tree);

	return tree;
}

void tree_free(Tree * tree)
{
	Tree * pa = tree->parent;

	void del(Tree * t)
	{
		pack_walk(t->childs, Tree, tt, del(tt););
		pack_free(t->childs);
		free(ANY t->name);
		free(ANY t->note);
		free(t);
	}
	del(tree);

	if (pa)
		pack_walk(pa->childs, Tree, tt, {
			if (tt == tree)
				pack_delete_me();
		});
}


/***************************************************
 *
 * util
 *
 */

void tree_set_name(Tree * tree, const char * name)
{
	if (tree->name) free(ANY tree->name);
	if (name) tree->name = strdup(name);
	else tree->name = NULL;
}


void tree_set_note(Tree * tree, const char * note)
{
	if (tree->note) free(ANY tree->note);
	if (note) tree->note = strdup(note);
	else tree->note = NULL;
}


void tree_count_direct_childs(Tree * tree, int * nboy, int * ngirl)
{
	int nchild[2] = { 0, 0 };
	pack_walk(tree->childs, Tree, t, {
		nchild[t->gender]++;
	});
	*nboy  = nchild[0];
	*ngirl = nchild[1];
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

	char str[256];

	Tree * tree_read(Tree * parent)
	{
		Tree * t = tree_new(parent);

		#define SLASH_TO_EMPTY(STR)		(strcmp((STR), "/") ? (STR) : "")

		fscanf(fp, "%s", str);
		t->gender = (strcmp(str, "女") == 0);

		fscanf(fp, "%s", str);
		t->name = strdup(SLASH_TO_EMPTY(str));

		fscanf(fp, "%s", str);
		t->note = strdup(SLASH_TO_EMPTY(str));

		int nchild;
		fscanf(fp, "%d", &nchild);

		// read all childs
		while (nchild--) tree_read(t);

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

	void tree_write(Tree * tree, int level)
	{
		#define EMPTY_TO_SLASH(STR)		((STR)[0] ? (STR) : "/")
		for (int i=0; i<level; i++) fprintf(fp, "  ");
		fprintf(fp, "%s %s %s %d\n",
				(tree->gender ? "女" : "男"),
				EMPTY_TO_SLASH(tree->name),
				EMPTY_TO_SLASH(tree->note),
				pack_length(tree->childs));
		pack_walk(tree->childs, Tree, t, tree_write(t, level+1););
	}

	tree_write(tree, 0);

	fclose(fp);
	return false;
}

