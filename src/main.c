
#include <stdio.h>
#include <string.h>
#include "ui.h"

const char * filename_to_open;

int main(int argc, const char * argv[])
{
	if (argc == 2) filename_to_open = strdup(argv[1]);
	ui_run();

	return 0;
}

