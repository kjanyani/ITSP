#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <math.h>
#include <Windows.h>

#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;
int ThresholdArea = 500;//chosen arbitrary.. need to work on it
// The source of webcam to used, if IP is used, just change the type to string and store IP address
const int Camera_Source_1 = 0;// "http://10.196.5.232:8080/videofeed";
const int Camera_Source_2 = 0;// "http://10.196.1.129:8080/videofeed";
							  // variables to store pixel coordinates, D denotes digital coordinates, A and B are the two cameras
int DxA1 = 0, DyA1 = 0, DxA2 = 0, DyA2 = 0, DxB1 = 0, DyB1 = 0;
//pixel coordinates of center of image
int OxA, OyA, OxB, OyB;
//Camera Specifications
double fA, fB;
//camera film coordinates and values
double xA1, yA1, xA2, yA2, xB1, yB1;
//time variables
double tA1 = 0, tA2 = 0, tB1 = 0;
//int to string helper function
string intToString(int number) {

	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void searchForMovement(Mat thresholdImage, Mat &cameraFeed, int &x, int &y){
    //notice how we use the '&' operator for objectDetected and cameraFeed. This is because we wish
    //to take the values passed into the function and manipulate them, rather than just working with a copy.
    //eg. we draw to the cameraFeed to be displayed in the main() function.
	int theObject[2] = { 0,0 };
	Rect objectBoundingRectangle = Rect(0, 0, 0, 0);
	bool objectDetected = false;
    Mat temp;
    thresholdImage.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    //findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
    findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours
 
    //if contours vector is not empty, we have found some objects
    if(contours.size()>0)objectDetected=true;
    else objectDetected = false;
 
    if(objectDetected){
        //the largest contour is found at the end of the contours vector
        //we will simply assume that the biggest contour is the object we are looking for.
        vector< vector<Point> > largestContourVec;
        largestContourVec.push_back(contours.at(contours.size()-1));
        //make a bounding rectangle around the largest contour then find its centroid
        //this will be the object's final estimated position.
        objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		if (objectBoundingRectangle.area() > ThresholdArea) {
			int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
			int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

			//update the objects positions by changing the 'theObject' array values
			theObject[0] = xpos, theObject[1] = ypos;
		}
    }
    //make some temp x and y variables so we dont have to type out so much
    x = theObject[0];
    y = theObject[1];
     
    //draw some crosshairs around the object
    circle(cameraFeed,Point(x,y),20,Scalar(0,255,0),2);
    line(cameraFeed,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    line(cameraFeed,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    line(cameraFeed,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    line(cameraFeed,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
 
    //write the position of the object to the screen
    putText(cameraFeed,"Tracking object at (" + intToString(x)+","+intToString(y)+")",Point(x,y),1,1,Scalar(255,0,0),2);
 
     
 
}

int main(int argc, const char** argv)
{

	// Init background substractor
	Ptr<BackgroundSubtractor> bg_model1 = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
	Ptr<BackgroundSubtractor> bg_model2 = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
	Ptr<BackgroundSubtractor> bg_model3 = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
	// Create empy input img, foreground and background image and foreground mask.
	Mat img, foregroundMask1, foregroundMask2, foregroundMask3, backgroundImage1, backgroundImage2, backgroundImage3;

	// capture video from source 0, which is web camera, If you want capture video from file just replace //by  VideoCapture cap("videoFile.mov")
	VideoCapture capA(Camera_Source_1), capB(Camera_Source_1);
	double t = getTickCount();
	// main loop to grab sequence of input files
	for (;;) {
		//double t = getTickCount();

		bool ok = capB.grab();

		if (ok == false) {

			std::cout << "Video Capture Fail at 2" << std::endl;


		}
		else {
			// obtain input image from source
			capB.retrieve(img, CV_CAP_OPENNI_BGR_IMAGE);
			// Just resize input image if you want
			resize(img, img, Size(640, 480));
			// create foreground mask of proper size
			if (foregroundMask2.empty()) {
				foregroundMask2.create(img.size(), img.type());
			}

			// compute foreground mask 8 bit image
			// -1 is parameter that chose automatically your learning rate

			bg_model2->apply(img, foregroundMask2, true ? -1 : 0);

			// smooth the mask to reduce noise in image
			GaussianBlur(foregroundMask2, foregroundMask2, Size(31, 31), 3.5, 3.5);

			// threshold mask to saturate at black and white values
			threshold(foregroundMask2, foregroundMask2, 10, 255, THRESH_BINARY);
			bg_model2->getBackgroundImage(backgroundImage2);
			imshow("foreground mask2", foregroundMask2);
			int key6 = waitKey(40);

			if (!backgroundImage2.empty()) {



				imshow("mean background image", backgroundImage2);
				int key5 = waitKey(40);

			}
		}
		ok = capA.grab();

		if (ok == false) {

			std::cout << "Video Capture Fail at 1" << std::endl;


		}
		else {

			// obtain input image from source
			capA.retrieve(img, CV_CAP_OPENNI_BGR_IMAGE);
			// Just resize input image if you want
			resize(img, img, Size(640, 480));
			imshow("Feed", img);
			// create foreground mask of proper size
			if (foregroundMask1.empty()) {
				foregroundMask1.create(img.size(), img.type());
			}

			// compute foreground mask 8 bit image
			// -1 is parameter that chose automatically your learning rate

			bg_model1->apply(img, foregroundMask1, true ? -1 : 0);

			// smooth the mask to reduce noise in image
			GaussianBlur(foregroundMask1, foregroundMask1, Size(31, 31), 3.5, 3.5);

			// threshold mask to saturate at black and white values
			threshold(foregroundMask1, foregroundMask1, 10, 255, THRESH_BINARY);
			if (((getTickCount() - t) / getTickFrequency()) > 4) {
				searchForMovement(foregroundMask1, foregroundMask1, DxA1, DyA1);
			}
			// create black foreground image
			//foregroundImg = Scalar::all(0);
			// Copy source image to foreground image only in area with white mask
			//img.copyTo(foregroundImg, foregroundMask);

			//Get background image
			bg_model1->getBackgroundImage(backgroundImage1);

			// Show the results
			imshow("foreground mask1", foregroundMask1);
			//imshow("foreground image", foregroundImg);
			//t = (getTickCount() - t) / getTickFrequency();
			//cout << t << endl;

			 waitKey(40);

			if (!backgroundImage1.empty()) {



				imshow("mean background image", backgroundImage1);
				 waitKey(40);

			}

			if (DxA1 != 0)
			{
				bool ok = capB.grab();

				if (ok == false) {

					std::cout << "Video Capture Fail at 2" << std::endl;


				}
				else {
					// obtain input image from source
					capB.retrieve(img, CV_CAP_OPENNI_BGR_IMAGE);
					// Just resize input image if you want
					resize(img, img, Size(640, 480));
					// create foreground mask of proper size
					if (foregroundMask2.empty()) {
						foregroundMask2.create(img.size(), img.type());
					}

					// compute foreground mask 8 bit image
					// -1 is parameter that chose automatically your learning rate

					bg_model2->apply(img, foregroundMask2, true ? -1 : 0);

					// smooth the mask to reduce noise in image
					GaussianBlur(foregroundMask2, foregroundMask2, Size(31, 31), 3.5, 3.5);

					// threshold mask to saturate at black and white values
					threshold(foregroundMask2, foregroundMask2, 10, 255, THRESH_BINARY);
					searchForMovement(foregroundMask2, foregroundMask2, DxB1, DyB1);
					imshow("ForegroundMask2", foregroundMask2);

					 ok = capA.grab();

					if (ok == false) {

						std::cout << "Video Capture Fail at 3" << std::endl;


					}
					else {

						// obtain input image from source
						capA.retrieve(img, CV_CAP_OPENNI_BGR_IMAGE);
						// Just resize input image if you want
						resize(img, img, Size(640, 480));
						// create foreground mask of proper size
						if (foregroundMask3.empty()) {
							foregroundMask3.create(img.size(), img.type());
						}

						// compute foreground mask 8 bit image
						// -1 is parameter that chose automatically your learning rate

						bg_model1->apply(img, foregroundMask3, true ? -1 : 0);

						// smooth the mask to reduce noise in image
						GaussianBlur(foregroundMask3, foregroundMask3, Size(31, 31), 3.5, 3.5);

						// threshold mask to saturate at black and white values
						threshold(foregroundMask3, foregroundMask3, 10, 255, THRESH_BINARY);
						searchForMovement(foregroundMask3, foregroundMask3, DxA2, DyA2);
						break;
						imshow("ForegroundMask3", foregroundMask3);
					}
				}
			}

		}
		
		
		
	}
	cout << DxA1 << "  " << DyA1 << "  " << DxB1 << "  " << DyB1 << "  " << DxA2 << "  " << DyA2 << "  ";
	waitKey(0);
	return EXIT_SUCCESS;
}