#include <iostream>
#include <string>
#include <typeinfo>
#include <sstream>
#include <fstream>
#include <cmath>
#include "function.h"

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;


string folder = "Input/";
string videoname = "CC_Hall_1";
string input_name = folder + videoname + ".mp4";


//Window Name
string MAIN_WINDOW = "Main Output";
string INTERMEDIATE_WINDOW = "Intermediate Step";
string INTERESTED_IMG = "Interested Area Image";
string SUB_IMG = "SubStracted Image";

//Windows parameter
ofstream fout;
int key = 0;
int windowRatio = 40;
int windowWidth = 16 * windowRatio;
int windowHeight = 9 * windowRatio;
int pause = 0;
int option = 2;
int optionNum = 3;
int fullview = 0;


//Object Record
const int HISTORY_FRAME = 30;
int frameObjectArea[HISTORY_FRAME];

//Object Size retangle
int upperLimit = 70000;
int large = 60000;
int middle = 40000;
int small = 20000;
int lowerLimit = 10000;

//Interested Area
Rect InterestedArea(0,0,0,0);


//Background substraction parameter
int history = 500;
double varThreshold = 255;
bool detectShadows = true;
bool learningRate = false;


// File output parameter
int SubStractedNum = 1;


void initial(); //Function to intialize the windows postion and setting
int showContours(Mat frame, vector< vector<Point> > contours); // Show contours in main window and return Object Area
void displaySelection(Mat* arrayFrame[4]); //For display selection in intermediate step window


int main(int argc, char** argv)
{
    initial();
    test();
    int frameCount = 0;
    VideoCapture cap(input_name);
    

    // Origin image, black-white image, substracted image, background image, edge detection image
    Mat frame, bgMOG2Img, fgMOG2MaskImg, fgMOG2Img, contoursImg;
    Mat* arrayFrame[4];
    
    //The value between 0 and 1 that indicates how fast the background model is learnt.
    //Negative parameter value makes the algorithm to use some automatically chosen learning rate.
    //0 means that the background model is not updated at all,
    //1 means that the background model is completely reinitialized from the last frame.
    //Parameter in the global variable
    Ptr<BackgroundSubtractor> pMOG2;
    pMOG2 = createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
    
    

    while(true)
    {
        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;
        

        if(pause == 0)
        {
            // Video to frame part
            cap >> frame;
            if(frame.empty())
            {
                cout << "Video Ended." << endl;
                break;
            }
            resize(frame,frame,Size(windowWidth,windowHeight));


            // Interested Area part
            Mat InterestedImg(frame,InterestedArea);
            Mat inputFrame;
            if(fullview == 1)
            {
                inputFrame = frame;
            }
            else
            {
                inputFrame = InterestedImg;
            }
            


            //update the model
            pMOG2->apply(inputFrame, fgMOG2MaskImg, learningRate ? -1 : 0);
            fgMOG2Img = Scalar::all(0);
            inputFrame.copyTo(fgMOG2Img, fgMOG2MaskImg);          
            pMOG2->getBackgroundImage(bgMOG2Img);


            // Find the contours in the image
            contoursImg = fgMOG2MaskImg.clone();
            findContours(contoursImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);        
            int objectArea = showContours(inputFrame,contours);
            frameObjectArea[frameCount] = objectArea;

            // Determine the state of car
            stableDetection(frameObjectArea,HISTORY_FRAME,objectArea);


            //Put all frame to an array for show in Intermediate step windows
            arrayFrame[0] = &bgMOG2Img;
            arrayFrame[1] = &fgMOG2MaskImg;
            arrayFrame[2] = &fgMOG2Img;
            arrayFrame[3] = &contoursImg;


            //Show window part
            imshow(INTERESTED_IMG,InterestedImg);
            rectangle(frame, InterestedArea, Scalar(200,255,145), 1, 8, 0);
            imshow(MAIN_WINDOW, frame);           
        }

        displaySelection(arrayFrame);

        if((key=waitKey(1))==27) 
        {
            cout << "Key \"Esc\" was pressed." << endl;
            break;
        }


        if(frameCount != HISTORY_FRAME -1)
        {
            frameCount++;
        }   
        else
        {
            frameCount = 0;
        }

    }
    return 0;
}

void initial()
{
    fout.open("SubStracted/data.info");
    int* record = ReadIAParameter(videoname);
    InterestedArea.x = record[0];
    InterestedArea.y = record[1];
    InterestedArea.width = record[2];
    InterestedArea.height = record[3];

    for(int i=0;i<HISTORY_FRAME;i++)
    {
        frameObjectArea[i] = -1;
    }

    //Original Image Windows
    namedWindow(MAIN_WINDOW, WINDOW_AUTOSIZE );
    createTrackbar("Pause", MAIN_WINDOW, &pause, 1);
    moveWindow(MAIN_WINDOW, 0, 0);
    
    //Control Windows
    namedWindow(INTERMEDIATE_WINDOW, WINDOW_AUTOSIZE );
    createTrackbar("Option", INTERMEDIATE_WINDOW, &option, optionNum);
    moveWindow(INTERMEDIATE_WINDOW, windowWidth+60, 0);

    //Interested Area Windows
    namedWindow(INTERESTED_IMG, 1);
    moveWindow(INTERESTED_IMG, 0, 600);

    //Substracted Area Windows
    namedWindow(SUB_IMG, 1);
    moveWindow(SUB_IMG, 800, 600);
}

int showContours(Mat frame, vector< vector<Point> > contours)
{
    
    int biggestID = -1;
    int biggestArea = -1;
    double objectArea;


    //Find the biggest object in the screen, and within the specific range
    for(int i=0;i<contours.size();i++)
    {
        // To ensure the object in the specfic area
        objectArea = contourArea(contours[i], false);
        
        if(objectArea > biggestArea && objectArea < upperLimit)
        {
            int minimunX = windowWidth;
            int maximumY = 0;
            biggestArea = objectArea;
            biggestID = i;
        }
    }


    if(biggestID != -1 && biggestArea > lowerLimit)
    {
        Rect bounding_rect = boundingRect(contours[biggestID]);
        objectArea = contourArea(contours[biggestID], false);

        Mat subImg(frame,bounding_rect);
        resize(subImg,subImg,Size(16*20,9*20));
        imshow(SUB_IMG,subImg);

        if(objectArea > large && objectArea < upperLimit)
        {
            rectangle(frame, bounding_rect, Scalar(0,0,255), 1, 8, 0);
            //Red
        }
        else if(objectArea > middle && objectArea < large)
        {
            rectangle(frame, bounding_rect, Scalar(255,0,0), 1, 8, 0);
            //Blue
        }
        else if(objectArea > small && objectArea < middle)
        {
            rectangle(frame, bounding_rect, Scalar(0,255,0), 1, 8, 0);
            //Green
        }
        else if(objectArea > 1000 && objectArea < small)
        {
            rectangle(frame, bounding_rect, Scalar(255,0,255), 1, 8, 0);
            //Purple
        }
    }
    else
    {
        objectArea = -1;
    }
    return objectArea;
}

void displaySelection(Mat* arrayFrame[4])
{
    switch(option)
    {
        case 0:
            if(!arrayFrame[0]->empty())
                imshow(INTERMEDIATE_WINDOW, *arrayFrame[0]);
            break;
        case 1:
            imshow(INTERMEDIATE_WINDOW, *arrayFrame[1]);
            break;
        case 2:
            imshow(INTERMEDIATE_WINDOW, *arrayFrame[2]);
            break;
        case 3:
            imshow(INTERMEDIATE_WINDOW, *arrayFrame[3]);
            break;
    }
}
