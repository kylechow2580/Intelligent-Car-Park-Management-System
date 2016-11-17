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


void test()
{
	cout << "testing function" << endl;
}

int* ReadIAParameter(string videoname)
{
	ifstream fin;
	string filename = "Input/" + videoname + ".txt";
	const char* p = filename.c_str();
	fin.open(p);
	string comment = "";
	int* argc = new int[4];
	getline(fin,comment);
	for(int i=0;i<4;i++)
	{
		fin >> argc[i];
	}
	return argc;
}

bool stableDetection(int frameArray[], const int length, int objectArea)
{
	int ignore = 0;
	int sumArea = 0;
    int averageArea = 0;

    int sortedArray[length];
    for(int i=0;i<length;i++)
    {
    	sortedArray[i] = 0 + frameArray[i];
    }

    
    sort(sortedArray,sortedArray + length);

    for(int j=0;j<length;j++)
    {
    	cout << "Frame[";
    	if(j<10)
    		cout << "0";
    	cout << j << "]: " << frameArray[j] << "\tSorted array[";
    	if(j<10)
    		cout << "0";
    	cout << j << "]: " << sortedArray[j] << endl;
        if(sortedArray[j] != -1)
        {
            if(j<length-1 && sortedArray[j+1] - sortedArray[j] > 4000 )
            {
                ignore++;
                // cout << "<-----Abnormal object area in the screen";
            }
            else
            {
                sumArea += sortedArray[j];
            }
        }
        else
        {
            // cout << "<-----No Object in the screen";
            ignore++;
        }
        // cout << endl;
    }

    

    if(ignore == length)
    {
        cout << "Average area is 0." << endl;
    }
    else
    {
        averageArea = sumArea / (length - ignore);
        cout << "Average Area: " << averageArea << endl;
    }

    int differentArea = objectArea - averageArea;
    if(sortedArray[0] == -1 && sortedArray[length-1] == -1)
    {
        cout << "No Object detected." << endl;
    }
    else if(abs(differentArea) < 1500) // average area +- 1500 = object area, stable > 10 == stop car
    {
        cout << "Object is nearly stop." << endl;
    }
    else
    {
        cout << "Object is moving." << endl;
    }

}