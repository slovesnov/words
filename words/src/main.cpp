/*
 * main.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#include "Frame.h"
#include "aslov.h"

int main (int argc, char** argv){
	gtk_init (&argc, &argv);
	aslovInit(argv);
	Frame frame;
	gtk_main ();
}
