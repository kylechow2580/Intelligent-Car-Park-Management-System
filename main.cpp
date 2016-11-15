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


string input_name = "Input/MOV_0002.mp4";
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

//Object Size retangle
int limit = 70000;
int large = 60000;
int middle = 40000;
int small = 20000;

//Interested Area
Rect InterestedArea(10,100,500,250);


//Background substraction parameter
int history = 500;
double varThreshold = 255;
bool detectShadows = true;
bool learningRate = false;


int SubStractedNum = 1;
Size size(windowWidth,windowHeight);


void initial(); //Function to intialize the windows postion and setting
void showContours(Mat frame, vector< vector<Point> > contours); // Show contours in main window
void displaySelection(Mat* arrayFrame[4]);


int main(int argc, char** argv)
{
    initial();
    
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
            cap >> frame;
            if(frame.empty())
            {
                cout << "Video Ended." << endl;
                break;
            }


            resize(frame,frame,size);
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
            showContours(inputFrame,contours);

            arrayFrame[0] = &bgMOG2Img;
            arrayFrame[1] = &fgMOG2MaskImg;
            arrayFrame[2] = &fgMOG2Img;
            arrayFrame[3] = &contoursImg;

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
    }
    return 0;
}

void initial()
{
    fout.open("SubStracted/data.info");

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

void showContours(Mat frame, vector< vector<Point> > contours)
{
    int i;
    int biggestID = -1;
    int biggestArea = -1;
    
    for(i=0;i<contours.size();i++)
    {
        // To ensure the object in the specfic area
        double objectArea = contourArea(contours[i], false);
        
        if(objectArea > biggestArea && objectArea < limit)
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
    }
    if(biggestArea != -1 && biggestArea > 1000)
    {
        cout << "Substracted Object Area: " << biggestArea << endl;
    }
    else
    {
        cout << "Substracted Object Area: No object substracted" << endl;
    }
    
    if(biggestID != -1 && biggestArea > 1000)
    {
        Rect bounding_rect = boundingRect(contours[biggestID]);
        double objectArea = contourArea(contours[biggestID], false);

        Mat subImg(frame,bounding_rect);
        resize(subImg,subImg,Size(16*20,9*20));
        imshow(SUB_IMG,subImg);

        if(objectArea > large && objectArea < limit)
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
    //     imshow(INTERESTED_IMG,subImg);
    // }
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
