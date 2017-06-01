//Attempt 1 written on 20/05/17
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include<conio.h>
#include<opencv2\opencv.hpp>
#include<math.h>

using namespace std;
using namespace cv;

//our sensitivity value to be used in the absdiff() function
int SENSITIVITY_VALUE = 100;
//size of blur used to smooth the intensity image output from absdiff() function
int BLUR_SIZE = 10;
// The source of webcam to used, if IP is used, just change the type to string and store IP address
const int Camera_Source_1 = 1;// "http://10.196.5.232:8080/videofeed";
const int Camera_Source_2 = 0;// "http://10.196.1.129:8080/videofeed";
							  // variables to store pixel coordinates, D denotes digital coordinates, A and B are the two cameras
int DxA1 = 0, DyA1 = 0, DxA2 = 0, DyA2 = 0, DxB1 = 0, DyB1 = 0;
//pixel coordinates of center of image
int OxA, OyA, OxB, OyB;
//Camera Specifications
double fA, fB, sA, sB;
//camera film coordinates and values
double xA1, yA1, xA2, yA2, xB1, yB1;
//time variables
double tA1 = 0, tA2 = 0, tB1 = 0;
const string trackbarWindowName = "Trackbars";
//we'll have just one object to search for
//and keep track of its position.

//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);

//Function Definitions to be used

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed





}

void createTrackbars() {
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	
	sprintf(TrackbarName, "SENSITIVITY_VALUE", SENSITIVITY_VALUE);
	sprintf(TrackbarName, "BLUR_SIZE", BLUR_SIZE);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("SENSITIVITY_VALUE", trackbarWindowName, &SENSITIVITY_VALUE, 255, on_trackbar);
	createTrackbar("BLUR_SIZE", trackbarWindowName, &BLUR_SIZE, 255, on_trackbar);


}
//int to string helper function
string intToString(int number) {

	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}
//Function that detects the white space in the thresholdImage and shows it real time on camerafeed, modified by me to also store position of object in x and y
void searchForMovement(Mat thresholdImage, Mat &cameraFeed, int &x, int &y) {
	//notice how we use the '&' operator for objectDetected and cameraFeed. This is because we wish
	//to take the values passed into the function and manipulate them, rather than just working with a copy.
	//eg. we draw to the cameraFeed to be displayed in the main() function.
	x = 0; // modified by me
	y = 0;
	cout << "search for movement called" << endl;
	int theObject[2] = { 0,0 };
	bool objectDetected = false;
	Mat temp, temp1;
	thresholdImage.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object

	//if contours vector is not empty, we have found some objects
	if (contours.size()>0)objectDetected = true;
	else objectDetected = false;

	if (objectDetected) {
		//the largest contour is found at the end of the contours vector
		//we will simply assume that the biggest contour is the object we are looking for.
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(contours.size() - 1));
		//make a bounding rectangle around the largest contour then find its centroid
		//this will be the object's final estimated position.
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
		int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

		//update the objects positions by changing the 'theObject' array values
		theObject[0] = xpos, theObject[1] = ypos;
		cout << "object found" << endl;
	}
	//make some temp x and y variables so we dont have to type out so much
	x = theObject[0]; //Modification to remove int as already defined above
	y = theObject[1];

	//draw some crosshairs around the object
	circle(cameraFeed, Point(x, y), 20, Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	line(cameraFeed, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);

	//write the position of the object to the screen
	putText(cameraFeed, "Tracking object at (" + intToString(x) + "," + intToString(y) + ")", Point(x, y), 1, 1, Scalar(255, 0, 0), 2);

}


int main() {

	//some boolean variables for added functionality
	bool objectDetected = false;
	//these two can be toggled by pressing 'd' or 't'
	bool debugMode = false;
	bool trackingEnabled = true;
	//pause and resume code
	bool pause = false;
	//set up the matrices that we will need
	//Ref Frames are to store the initial image from whcih final frames are subracted
	Mat RefframeA, RefframeB, frameA, frameB;
	//their grayscale images (needed for absdiff() function)
	Mat RefgrayImageA, RefgrayImageB, grayImageA, grayImageB;
	//resulting difference image
	Mat differenceImageA, differenceImageB;
	//thresholded difference image (for use in findContours() function)
	Mat thresholdImageA, thresholdImageB;
	//video capture object.
	
	//Creates slider bar for SensitivityValue
	VideoCapture captureA, captureB;
	captureA.open(Camera_Source_1);
	captureB.open(Camera_Source_2);
	// Storing the Ref frame for camera A, press esc to store it
	while (1) {

		if (!captureA.isOpened()) {
			cout << "ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}
		//read first frame
		captureA.read(RefframeA);
		imshow("RefFrameA", RefframeA);
		//waitKey causes a delay of 10 milliseconds and returns the ascii of the character pressed.. ascii of esc is 27
		if (waitKey(30) == 27) {
			//convert RefframeA to gray scale for frame differencing
			cv::cvtColor(RefframeA, RefgrayImageA, COLOR_BGR2GRAY);
			break;
		}
	}
	// Storing the Ref frame for camera B, press esc to store it
	while (1) {

		if (!captureB.isOpened()) {
			cout << "ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}
		//read first frame
		captureB.read(RefframeB);
		imshow("RefFrameB", RefframeB);
		//waitKey causes a delay of 10 milliseconds and returns the ascii of the character pressed.. ascii of esc is 27
		if (waitKey(30) == 27) {
			cv::cvtColor(RefframeB, RefgrayImageB, COLOR_BGR2GRAY);
			break;
		}
	}
	//capturing remaining frames and processing them
	while (1) {
		createTrackbars();
		DxA1 = 0, DyA1 = 0, DxA2 = 0, DyA2 = 0, DxB1 = 0, DyB1 = 0;
		//read frame for camera A
		captureA >> frameA;
		tA1 = getTickCount();
		//convert frame1 to gray scale for frame differencing
		cv::cvtColor(frameA, grayImageA, COLOR_BGR2GRAY);
		//perform frame differencing with the Reference frame images. This will output an "intensity image"
		//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
		cv::absdiff(grayImageA, RefgrayImageA, differenceImageA);
		//thresholding to give image in just 0 and 1
		cv::threshold(differenceImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		//blur the image to get rid of the noise. This will output an intensity image
		//cv::blur(thresholdImageA, thresholdImageA, cv::Size(BLUR_SIZE, BLUR_SIZE));
		//threshold again to obtain binary image from blur output
		//cv::threshold(thresholdImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		imshow("thresholdImageA", thresholdImageA);
		waitKey(10);
		/*searchForMovement(thresholdImageA, frameA, DxA1, DyA1);
		if (DxA1 != 0) {
			//read frame for camera B
			//waitKey(50);
			while (DxB1 == 0)
			{
				double t = (getTickCount() - tA1) / getTickFrequency();
				cout << "time" << t << endl;
				captureB >> frameB;

				tB1 = (getTickCount() - tA1) / getTickFrequency();
				cv::cvtColor(frameB, grayImageB, COLOR_BGR2GRAY);
				//perform frame differencing with the Reference frame images. This will output an "intensity image"
				//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
				cv::absdiff(grayImageB, RefgrayImageB, differenceImageB);
				//thresholding to give image in just 0 and 1
				cv::threshold(differenceImageB, thresholdImageB, SENSITIVITY_VALUE, 255, THRESH_BINARY);
				//blur the image to get rid of the noise. This will output an intensity image
				cv::blur(thresholdImageB, thresholdImageB, cv::Size(BLUR_SIZE, BLUR_SIZE));
				//threshold again to obtain binary image from blur output
				cv::threshold(thresholdImageB, thresholdImageB, SENSITIVITY_VALUE, 255, THRESH_BINARY);
				searchForMovement(thresholdImageB, frameB, DxB1, DyB1);

			}*/
			/*for (int i = 0; i < 10; i++)
			{
			captureB.read(frameB);
			imshow(intToString(i), frameB);
			waitKey(30);
			}*/

			//read frame for camera A
			/*int i = 0;
			Mat frameC = frameA;
			while (sum(frameA != frameC) == Scalar(0, 0, 0, 0)&&i<100) {
			captureA.read(frameA);
			cout << "same";
			i++;
			}
			CvCapture* capture=cvCaptureFromCAM(0);
			IplImage *frame = cvQueryFrame(capture); //to read properties of frame.
			IplImage *frame2 = NULL;
			while (1) {
			if (frame2)
			frame = cvCloneImage(frame2); // copy image to allow grabbing next frame
			frame2 = cvQueryFrame(capture); //read next frame
			if (!frame2) break; //if frame cannot be read, EOF so break from loop
			imshow("frame2", frame2);
			}
			//Mat diff = frameA != frameC;
			//bool isEqual = (sum(frameA != frameC) == Scalar(0, 0, 0, 0));
			*/
			//captureA.open(Camera_Source_1);
			//waitKey(50);
			/*captureA >> frameA;
			tA2 = (getTickCount() - tA1) / getTickFrequency();
			//convert frame1 to gray scale for frame differencing
			cv::cvtColor(frameA, grayImageA, COLOR_BGR2GRAY);
			//perform frame differencing with the Reference frame images. This will output an "intensity image"
			//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
			cv::absdiff(grayImageA, RefgrayImageA, differenceImageA);
			//thresholding to give image in just 0 and 1
			cv::threshold(differenceImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			//blur the image to get rid of the noise. This will output an intensity image
			cv::blur(thresholdImageA, thresholdImageA, cv::Size(BLUR_SIZE, BLUR_SIZE));
			//threshold again to obtain binary image from blur output
			cv::threshold(thresholdImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			searchForMovement(thresholdImageA, frameA, DxA2, DyA2);
			imshow("FrameA", frameA);
			imshow("FrameB", frameB);
			cout << DxA1 << "\t" << DyA1 << "\t" << DxB1 << "\t" << DyB1 << "\t" << DxA2 << "\t" << DyA2 << "\t" << tB1 << "\t" << tA2 << "\t";
			break;*/

		}

	//}
	waitKey(0);
	//code for data collection ends
	//code for processing information
	getch();
	return 0;
}