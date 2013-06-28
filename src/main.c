
#include <stdio.h>
#include <string.h>
#include "ui.h"

int main(int argc, const char * argv[])
{
	if (argc == 2) filename = strdup(argv[1]);
	ui_run();

	return 0;
}

