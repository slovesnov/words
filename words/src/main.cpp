/*
 * main.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

/*for compile cgi under windows
 * define NDEBUG CGI
 * lib iconv for cgi
 * i guess not need -mwindows option under cgi
 *
 * under linux old gcc cann't compile too old compiler
 * under linux may be NOGTK=0?1
 * define NDEBUG CGI NOGTK=1
 *
 */
#ifdef CGI
	#include "common/WordsBase.h"
#else
#include "Frame.h"
#include "aslov.h"
#endif

int main(int argc, char **argv) {
#ifdef CGI
	WordsBase w;
#else
	gtk_init(&argc, &argv);
	aslovInit(argv);
	Frame frame;
	gtk_main();
#endif
}
