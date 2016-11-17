#include <string>
#include <iostream>
using namespace std;	

#ifndef FUNCTION_H
#define FUNCTION_H

void test();
int* ReadIAParameter(string videoname);
bool stableDetection(int frameArray[], const int length, int objectArea);

#endif