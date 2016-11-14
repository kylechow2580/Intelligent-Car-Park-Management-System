#include <iostream>
#include <string>
#include <typeinfo>
#include <sstream>
#include <fstream>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace std;
using namespace cv;

//0002 , 10, 260

string input_name = "Input/MOV_0002.mp4";
string MAIN_WINDOW = "Main Output";
string INTERMEDIATE_WINDOW = "Intermediate Step";
string SUBSTRACTED_IMG = "Sub Image";

//Windows parameter
ofstream fout;
int key = 0;
int windowRatio = 40;
int windowWidth = 16 * windowRatio;
int windowHeight = 9 * windowRatio;
int pause = 0;
int option = 2;
int optionNum = 3;

//Object Size retangle
int large = 10000;
int middle = 5000;
int small = 2000;

//Interested Area
Rect InterestedArea(40,100,500,250);


//Background substraction parameter
int history = 500;
double varThreshold = 255;
bool detectShadows = true;
bool learningRate = false;


int SubStractedNum = 1;
Size size(windowWidth,windowHeight);




void initial(); //Function to intialize the windows postion and setting
void showContours(Mat frame, vector< vector<Point> > contours); // Show contours in main window

int main(int argc, char** argv)
{
    initial();
    
    VideoCapture cap(input_name);
    
    fout.open("SubStracted/data.info");
    
    Mat frame, fgMOG2MaskImg, fgMOG2Img, bgMOG2Img, contoursImg;
    Ptr<BackgroundSubtractor> pMOG2;

    pMOG2 = createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
    
    //The value between 0 and 1 that indicates how fast the background model is learnt.
    //Negative parameter value makes the algorithm to use some automatically chosen learning rate.
    //0 means that the background model is not updated at all,
    //1 means that the background model is completely reinitialized from the last frame.
    
    

    while(true)
    {
        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;
        if(pause == 0)
        {
            cap >> frame;
            if(frame.empty())
            {
                cout << "Video Ended." << endl;
                break;
            }
            resize(frame,frame,size);
            
            
            //update the model
            pMOG2->apply(frame, fgMOG2MaskImg, learningRate ? -1 : 0);
            
            fgMOG2Img = Scalar::all(0);
            frame.copyTo(fgMOG2Img, fgMOG2MaskImg);
            
            pMOG2->getBackgroundImage(bgMOG2Img);

            // Find the contours in the image
            contoursImg = fgMOG2MaskImg.clone();
            findContours(contoursImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);        
            showContours(frame,contours);
            

            // line(frame, Point(lineX,0), Point(lineX,windowHeight), Scalar(255,155,128),2);
            // line(frame, Point(0,lineY), Point(windowWidth,lineY), Scalar(255,155,128),2);
            rectangle(frame, InterestedArea, Scalar(200,255,145), 3, 8, 0);
            imshow(MAIN_WINDOW, frame);

            Mat subImg(frame,InterestedArea);
            imshow(SUBSTRACTED_IMG,subImg);
        }
        switch(option)
            {
                case 0:
                    if(!bgMOG2Img.empty())
                        imshow(INTERMEDIATE_WINDOW, bgMOG2Img);
                    break;
                case 1:
                    imshow(INTERMEDIATE_WINDOW, fgMOG2MaskImg);
                    break;
                case 2:
                    imshow(INTERMEDIATE_WINDOW, fgMOG2Img);
                    break;
                case 3:
                    imshow(INTERMEDIATE_WINDOW, contoursImg);
                    break;
            }
        if((key=waitKey(1))==27) 
        {
            cout << "Key \"Esc\" was pressed." << endl;
            break;
        }
    }
    return 0;
}

void initial()
{
    namedWindow(MAIN_WINDOW, WINDOW_AUTOSIZE );
    createTrackbar("Pause", MAIN_WINDOW, &pause, 1);
    // createTrackbar("LineX", MAIN_WINDOW, &lineX, windowWidth);
    // createTrackbar("LineY", MAIN_WINDOW, &lineY, windowHeight);
    moveWindow(MAIN_WINDOW, 0, 0);
    
    
    namedWindow(INTERMEDIATE_WINDOW, WINDOW_AUTOSIZE );
    createTrackbar("Option", INTERMEDIATE_WINDOW, &option, optionNum);
    moveWindow(INTERMEDIATE_WINDOW, windowWidth+60, 0);

    namedWindow(SUBSTRACTED_IMG, 1);
    moveWindow(SUBSTRACTED_IMG, 0, 600);
}

void showContours(Mat frame, vector< vector<Point> > contours)
{
    int i;
    int biggestID = -1;
    int biggestArea = 1000;
    
    for(i=0;i<contours.size();i++)
    {
        // To ensure the object in the specfic area
        double objectArea = contourArea(contours[i], false);
        
        if(objectArea > biggestArea)
        {
            int j;
            int minimunX = windowWidth;
            int maximumY = 0;
            for (j=0;j<contours[i].size();j++)
            {
                if(contours[i][j].x < minimunX)
                {
                    minimunX = contours[i][j].x;
                }
                if(contours[i][j].y > maximumY)
                {
                    maximumY = contours[i][j].y;
                }
            }
            biggestArea = objectArea;
            biggestID = i;
        }
        else
        {
            continue;
        }
    }

    cout << "biggestID: " << biggestID << " area: " << biggestArea << endl;
    if(biggestID != -1)
    {
        Rect bounding_rect = boundingRect(contours[biggestID]);
        double objectArea = contourArea(contours[biggestID], false);
        cout << objectArea << endl;
        if(objectArea > large)
        {
            rectangle(frame, bounding_rect, Scalar(0,0,255), 1, 8, 0);
            //Red
        }
        else if(objectArea > small && objectArea < middle)
        {
            rectangle(frame, bounding_rect, Scalar(255,0,0), 1, 8, 0);
            //Blue
        }
        else if(objectArea > 1000 && objectArea < small)
        {
            rectangle(frame, bounding_rect, Scalar(0,255,0), 1, 8, 0);
            //Green
        }

        // Mat subImg(frame,bounding_rect);
        // resize(subImg,subImg,Size(16*20,9*20));
        // imshow(SUBSTRACTED_IMG,subImg);
    }
        
    // drawContours(subImg, contours, i, Scalar(0,255,0));


    // if(minimunX <= lineX + 30)
    // {
    //     Mat subImg(frame,bounding_rect);
    //     resize(subImg,subImg,Size(16*20,9*20));
    //     stringstream filename;
    //     filename << "SubStracted/sub";
    //     filename << SubStractedNum;
    //     filename << ".jpg";
    //     // string fileName = filename.str();
    //     // fout << fileName << " 1 0 0 " << 16*20 << " " << 9*20 << endl;
    //     // SubStractedNum++;
    //     imwrite(filename.str(), subImg);
    //     imshow(SUBSTRACTED_IMG,subImg);
    // }
}
