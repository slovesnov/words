/*
 * main.cpp
 *
 *  Created on: 14.09.2015
 *      Author: alexey slovesnov
 */

#include "common/WordsBase.h"
#ifndef NOGTK
#include "Frame.h"
#endif

int main(int argc, char **argv) {
#ifdef NOGTK
  WordsBase words;
#else
  gtk_init(&argc, &argv);
  aslovInit(argv);
  WordsBase words;
  Frame frame;
  gtk_main();
#endif
}
