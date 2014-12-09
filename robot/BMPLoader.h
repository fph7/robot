#ifndef BMPLOADER
#define BMPLOADER
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <Windows.h>

int LoadBMP(const char* location, GLuint &texture);

#endif