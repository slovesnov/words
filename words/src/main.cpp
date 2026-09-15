/*
 * main.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#ifdef NOGTK
	#include "common/WordsBase.h"
#else
#include "Frame.h"
#include "aslov.h"
#endif

int main(int argc, char **argv) {
#ifdef NOGTK
	WordsBase w;
#else
	gtk_init(&argc, &argv);
	aslovInit(argv);
	Frame frame;
	gtk_main();
#endif
}
