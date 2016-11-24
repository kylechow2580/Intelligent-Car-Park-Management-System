#include <string>
#include <iostream>
using namespace std;	

#ifndef FUNCTION_H
#define FUNCTION_H

void test();
int* ReadParameter(string videoname);
bool stableDetection(int frameArray[], const int length, int objectArea, int abnormalFrameDifferent, int stableConstant);

#endif