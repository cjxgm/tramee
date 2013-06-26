
#include <stdio.h>
#include "ui.h"

int main(int argc, const char * argv[])
{
	root = tree_new(NULL);
	root->name  = "祖";
	root->ngirl = 3;

	{
		Tree * t = tree_new(root);
		t->name  = "子1";
		t->wife  = "妻1";
		t->ngirl = 1;

		{
			Tree * t1 = tree_new(t);
			t1->name = "子11";
			t1->wife = "妻11";
		}
	}

	{
		Tree * t = tree_new(root);
		t->name = "子2";
		t->wife = "妻2";
	}

	// test if loading and saving works
	tree_save(root, "test.t");

	$_(t, tree_load("test.t"));
	tree_save(t, "test2.t");

	ui_run();

	return 0;
}

