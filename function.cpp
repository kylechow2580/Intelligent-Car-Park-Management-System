#include <iostream>
#include <string>
#include <typeinfo>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "function.h"

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int AFD;
int SC;

void test()
{
	cout << "testing function" << endl;
}

int* ReadParameter(string videoname)
{
	ifstream fin;
	int numOfParameter = 12;
	string filename = "Input/" + videoname + ".txt";
	const char* p = filename.c_str();


	fin.open(p);
	string comment = "";
	int* argc = new int[numOfParameter];
	getline(fin,comment);
	for(int i=0;i<numOfParameter;i++)
	{
		fin >> argc[i];
	}
	return argc;
}


int findIgnore(int* frameArray, int* sortedArray, int length, int& sumArea)
{
	int ignore = 0;
	cout << endl << "=======Infomation=======" << endl;
    for(int j=0;j<length;j++)
    {
    	cout << "Frame[";
    	if(j<10)
    		cout << "0";
    	cout << j << "]: " << frameArray[j];
		cout << "   Sorted Array[";
		if(j<10)
			cout << "0";
		cout << j << "]: " << sortedArray[j];
        if(sortedArray[j] != -1)
        {
            if(j<length-1 && sortedArray[j+1] - sortedArray[j] > abs(AFD))
            {
            	cout << "   {" << sortedArray[j+1] - sortedArray[j] << "}";
                ignore++;
            }
            else
            {
                sumArea += sortedArray[j];
            }
        }
        else
        {
            ignore++;
        }
        cout << endl;
    }
    return ignore;
}

bool stableDetection(int frameArray[], const int length, int objectArea, int abnormalFrameDifferent, int stableConstant)
{
	int ignore = 0;
	int sumArea = 0;
    int averageArea = 0;

    AFD = abnormalFrameDifferent;
    SC = stableConstant;

    int sortedArray[length];
    for(int i=0;i<length;i++)
    {
    	sortedArray[i] = 0 + frameArray[i];
    }    
    // sort(sortedArray,sortedArray + length);


    ignore = findIgnore(frameArray,sortedArray,length,sumArea);

    

    if(ignore == length) // if all frame ignored
    {
        cout << "Average area is 0." << endl;
    }
    else
    {
        averageArea = sumArea / (length - ignore);
        cout << "Average Area: " << averageArea << endl;
    }



    int differentArea = objectArea - averageArea;
    cout << "Object Area: " << objectArea << endl;
    if(sortedArray[0] == -1 && sortedArray[length-1] == -1)
    {
        cout << "No Object detected." << endl;
    }
    else if(abs(differentArea) < stableConstant) // average area +- 1500 = object area, stable > 10 == stop car
    {
        cout << "Object is nearly stop." << endl;
    }
    else
    {
        cout << "Object is moving." << endl;
    }

}
