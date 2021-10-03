/*
 * main.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#include "Frame.h"

int main (int argc, char** argv){
	gtk_init (&argc, &argv);
	Frame frame(argv[0]);
	gtk_main ();

	return 0;
}
