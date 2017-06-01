//Attempt 1 written on 20/05/17
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include<conio.h>
#include<opencv2\opencv.hpp>
#define _USE_MATH_DEFINES ;
#include<math.h>
#include"Stepper.h"
#include"SerialPort.h"
#include <sstream>
using namespace std;
using namespace cv;

//Value of G
const double g = 9800;

//Debugging mode Parameter
bool debug = FALSE;

//our sensitivity value to be used in the absdiff() function
const static int SENSITIVITY_VALUE = 10;
//size of blur used to smooth the intensity image output from absdiff() function
const static int BLUR_SIZE = 10;
// The source of webcam to used, if IP is used, just change the type to string and store IP address
const int Camera_Source_1 = 0;// "http://10.196.5.232:8080/videofeed";
const int Camera_Source_2 = 1;// "http://10.196.1.129:8080/videofeed";
// variables to store pixel coordinates, D denotes digital coordinates, A and B are the two cameras
int DxA1 = 0, DyA1 = 0, DxA2 = 0, DyA2 = 0, DxB1 = 0, DyB1 = 0;
//pixel coordinates of center of image
int OxA, OyA, OxB, OyB;
//Camera Specifications
double F1, F2;
//camera film coordinates and values
double XSA1, YSA1, XSA2, YSA2, XSB1, YSB1;
//Camera 2 Coordinates wrt 1
double X0, Y0, Z0;
//Trajectory variables
double xo, yo, zo, VX, VY, VZ;
//time variables
double tA1=0, tA2=0, tB1=0;
//coordinates of left bottom stepper from camera coordinates
double YstepperLB, ZstepperLB;
//we'll have just one object to search for
//and keep track of its position.
//String for getting the output from arduino
char output[MAX_DATA_LENGTH];

/*Portname must contain these backslashes, and remember to
replace the following com port*/
char *port_name = "\\\\.\\COM3";

//String for incoming data
char incomingData[MAX_DATA_LENGTH];

//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);

//Function Definitions to be used

void print(vector< vector<double> > A) {
	int n = A.size();
	for (int i = 0; i<n; i++) {
		for (int j = 0; j<n + 1; j++) {
			cout << A[i][j] << "\t";
			if (j == n - 1) {
				cout << "| ";
			}
		}
		cout << "\n";
	}
	cout << endl;
}

vector<double> gauss(vector< vector<double> > A) {
	int n = A.size();

	for (int i = 0; i<n; i++) {
		// Search for maximum in this column
		double maxEl = abs(A[i][i]);
		int maxRow = i;
		for (int k = i + 1; k<n; k++) {
			if (abs(A[k][i]) > maxEl) {
				maxEl = abs(A[k][i]);
				maxRow = k;
			}
		}

		// Swap maximum row with current row (column by column)
		for (int k = i; k<n + 1; k++) {
			double tmp = A[maxRow][k];
			A[maxRow][k] = A[i][k];
			A[i][k] = tmp;
		}

		// Make all rows below this one 0 in current column
		for (int k = i + 1; k<n; k++) {
			double c = -A[k][i] / A[i][i];
			for (int j = i; j<n + 1; j++) {
				if (i == j) {
					A[k][j] = 0;
				}
				else {
					A[k][j] += c * A[i][j];
				}
			}
		}
	}

	// Solve equation Ax=b for an upper triangular matrix A
	vector<double> x(n);
	for (int i = n - 1; i >= 0; i--) {
		x[i] = A[i][n] / A[i][i];
		for (int k = i - 1; k >= 0; k--) {
			A[k][n] -= A[k][i] * x[i];
		}
	}
	return x;
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
	if (debug) {
		cout << "search for movement called" << endl;
	}
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
	double XPlane; // ZPlane represents position of dartboard wrt origin

	//some boolean variables for added functionality
	bool objectDetected = false;
	//these two can be toggled by pressing 'd' or 't'
	bool debugMode = false;
	bool trackingEnabled = true;
	//pause and resume code
	bool pause = false;
	//set up the matrices that we will need
	//Ref Frames are to store the initial image from whcih final frames are subracted
	Mat RefframeA, RefframeB, frame1, frame2, frame3;
	//their grayscale images (needed for absdiff() function)
	Mat RefgrayImageA, RefgrayImageB, grayImageA, grayImageB;
	//resulting difference image
	Mat differenceImageA, differenceImageB;
	//thresholded difference image (for use in findContours() function)
	Mat thresholdImageA, thresholdImageB;
	//video capture object.
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
		DxA1 = 0, DyA1 = 0, DxA2 = 0, DyA2 = 0, DxB1 = 0, DyB1 = 0;
		//read frame for camera A1
		captureA >> frame1;
		tA1 = getTickCount();
		//convert frame1 to gray scale for frame differencing
		cv::cvtColor(frame1, grayImageA, COLOR_BGR2GRAY);
		//perform frame differencing with the Reference frame images. This will output an "intensity image"
		//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
		cv::absdiff(grayImageA, RefgrayImageA, differenceImageA);
		//thresholding to give image in just 0 and 1
		cv::threshold(differenceImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		//blur the image to get rid of the noise. This will output an intensity image
		cv::blur(thresholdImageA, thresholdImageA, cv::Size(BLUR_SIZE, BLUR_SIZE));
		//threshold again to obtain binary image from blur output
		cv::threshold(thresholdImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
		searchForMovement(thresholdImageA, frame1, DxA1, DyA1);
		if (DxA1 != 0) {
			//read frame for camera B
			//waitKey(50);
			while (DxB1 == 0)
			{
				if (debug == true)
				{
					double t = (getTickCount() - tA1) / getTickFrequency();
					cout << "time" << t << endl;
				}
				captureB >> frame2;

				tB1 = (getTickCount() - tA1) / getTickFrequency();
				cv::cvtColor(frame2, grayImageB, COLOR_BGR2GRAY);
				//perform frame differencing with the Reference frame images. This will output an "intensity image"
				//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
				cv::absdiff(grayImageB, RefgrayImageB, differenceImageB);
				//thresholding to give image in just 0 and 1
				cv::threshold(differenceImageB, thresholdImageB, SENSITIVITY_VALUE, 255, THRESH_BINARY);
				//blur the image to get rid of the noise. This will output an intensity image
				cv::blur(thresholdImageB, thresholdImageB, cv::Size(BLUR_SIZE, BLUR_SIZE));
				//threshold again to obtain binary image from blur output
				cv::threshold(thresholdImageB, thresholdImageB, SENSITIVITY_VALUE, 255, THRESH_BINARY);
				searchForMovement(thresholdImageB, frame2, DxB1, DyB1);

			}
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
			captureA >> frame3;
			tA2 = (getTickCount() - tA1) / getTickFrequency();
			//convert frame1 to gray scale for frame differencing
			cv::cvtColor(frame3, grayImageA, COLOR_BGR2GRAY);
			//perform frame differencing with the Reference frame images. This will output an "intensity image"
			//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
			cv::absdiff(grayImageA, RefgrayImageA, differenceImageA);
			//thresholding to give image in just 0 and 1
			cv::threshold(differenceImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			//blur the image to get rid of the noise. This will output an intensity image
			cv::blur(thresholdImageA, thresholdImageA, cv::Size(BLUR_SIZE, BLUR_SIZE));
			//threshold again to obtain binary image from blur output
			cv::threshold(thresholdImageA, thresholdImageA, SENSITIVITY_VALUE, 255, THRESH_BINARY);
			searchForMovement(thresholdImageA, frame3, DxA2, DyA2);
			imshow("Frame1", frame1);
			imshow("Frame2", frame2);
			imshow("Frame3", frame3);
			cout << DxA1 << "\t" << DyA1 << "\t" << DxB1 << "\t" << DyB1 << "\t" << DxA2 << "\t" << DyA2 << "\t" << tB1 << "\t" << tA2 << "\t";
			break;

		}

	}
	tA1 = 0;

	//code for data collection ends
	//code for processing information
	//Solving Equations to get final coordinates of dart
	XSA1 = (DxA1 - OxA);
	YSA1 = (DyA1 - OyA);
	XSA2 = (DxA2 - OxA);
	YSA2 = (DyA2 - OyA);
	XSB1 = (DxB1 - OxB);
	YSB1 = (DyB1 - OyB);

	//Equation Solver
	int noofvar = 6;


	vector<double> line(noofvar + 1, 0);
	vector< vector<double> > A(noofvar, line);

	//cin >> F1 >> F2 >> XSA1 >> XSA2 >> XSB1 >> YSA1 >> YSA2 >> YSB1 >> tA1 >> tA2 >> tB1 >> X0 >> Y0 >> Z0;
	// Read input data
	A[0][0] = F1;
	A[0][1] = F1*tA1;
	A[0][2] = A[0][3] = A[1][0] = A[1][1] = A[2][2] = A[2][3] = A[3][0] = A[3][1] = A[4][0] = A[4][1] = A[5][4] = A[5][5] = 0;
	A[0][4] = -XSA1;
	A[0][5] = -tA1*XSA1;
	A[0][6] = (-0.5*g*tA1*tA1*XSA1);
	A[1][2] = F1; A[1][3] = F1*tA1; A[1][4] = -YSA1; A[1][5] = tA1*YSA1; A[1][6] = (-0.5*g*tA1*tA1*YSA1); A[2][0] = F1; A[2][1] = F1*tA2; A[2][4] = -XSA2; A[2][5] = -tA2*XSA2; A[2][6] = (-0.5*g*tA2*tA2*XSA2);
	A[3][2] = F1; A[3][3] = F1*tA2; A[3][4] = -YSA2; A[3][5] = -tA2*YSA2; A[3][6] = (-g*0.5*tA2*tA2*YSA2);
	A[4][2] = XSB1; A[4][3] = XSB1*tB1; A[4][4] = -F2; A[4][5] = -F2*tB1; A[4][6] = (Y0*XSB1 - 0.5*g*F2*tB1*tB1 - F2*Z0);
	A[5][0] = F2; A[5][1] = F2*tB1; A[5][2] = -YSB1; A[5][3] = -YSB1*tB1; A[5][6] = F2*X0 - YSB1*Y0;

	// Calculate solution
	vector<double> x(noofvar);
	x = gauss(A);
	xo = x[0]; VX = x[1]; yo = x[2]; VY = x[3]; zo = x[4]; VZ = x[5];
	// Print result if debug is on
	if(debug==TRUE)
	{
	// Print input
	print(A);
	cout << "Result:\t";
	for (int i = 0; i < noofvar; i++) {
		cout << x[i] << " ";
	}
	}
	cout << endl;
	double tF = (XPlane - xo) / VX; //tf is the time of flight to reach x=k from x=xo
	double Y = yo + VY*tF;
	double Z = zo + VZ*tF - 0.5*g*tF*tF;
	//cout << tF << Y << Z;

	//Elec Code
	int xf = Y - YstepperLB, yf = Z - ZstepperLB;
	Stepper RT, RB, LT, LB; // Right top, Right bottom, Left Top, Left Bottom steppers.


	double radpulley = 40;
	double radmount = 18.98;

	double xRT = 0, yRT = 665, xRB = 0, yRB = 0, xLT = 695, yLT = 0, xLB = 695, yLB = 680, x0 = 210, y0 = 52;
	double lengthRT = sqrt((xRT - xf)*(xRT - xf) + (yRT - yf)*(yRT - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt((xRT - x0)*(xRT - x0) + (yRT - y0)*(yRT - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthRB = sqrt((xRB - xf)*(xRB - xf) + (yRB - yf)*(yRB - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt((xRB - x0)*(xRB - x0) + (yRB - y0)*(xRB - x0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthLT = sqrt((xLT - xf)*(xLT - xf) + (yLT - yf)*(yLT - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt((xLT - x0)*(xLT - x0) + (yLT - y0)*(yLT - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthLB = sqrt((xLB - xf)*(xLB - xf) + (yLB - yf)*(yLB - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt((xLB - x0)*(xLB - x0) + (yLB - y0)*(yLB - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length

	RT.StepPin = 3;
	RT.DirPin = 2;
	RT.Nstep = (abs(lengthRT) * 100) / (M_PI*radpulley); // formula for finding no. of steps obtained from length/(2*Pi*r) * 360/1.8 
	if (lengthRT < 0) { RT.windingState = 1; }
	else
		RT.windingState = 0;

	LT.StepPin = 11;
	LT.DirPin = 12;
	LT.Nstep = (abs(lengthLT) * 100) / (M_PI*radpulley); // formula for finding no. of steps obtained from length/(2*Pi*r) * 360/1.8 
	if (lengthLT < 0) { LT.windingState = 1; }
	else
		LT.windingState = 0;

	RB.StepPin = 5;
	RB.DirPin = 4;
	RB.Nstep = (abs(lengthRB) * 100) / (M_PI*radpulley); // formula for finding no. of steps obtained from length/(2*Pi*r) * 360/1.8 
	if (lengthRB < 0) { RB.windingState = 1; }
	else
		RB.windingState = 0;

	LB.StepPin = 9;
	LB.DirPin = 8;
	LB.Nstep = (abs(lengthLB) * 100) / (M_PI*radpulley); // formula for finding no. of steps obtained from length/(2*Pi*r) * 360/1.8 
	if (lengthLB < 0) { LB.windingState = 1; }
	else
		LB.windingState = 0;

	vector<Stepper> unwind; // storing unwinding stepper data
	vector<Stepper> wind;  // storing winding stepper data

	if (RT.windingState)
	{
		wind.push_back(RT);
	}
	else unwind.push_back(RT);

	if (LT.windingState)
	{
		wind.push_back(LT);
	}
	else unwind.push_back(LT);

	if (RB.windingState)
	{
		wind.push_back(RB);
	}
	else unwind.push_back(RB);

	if (LB.windingState)
	{
		wind.push_back(LB);
	}
	else unwind.push_back(LB);

	size_t Casetype = wind.size();

	sort(wind.begin(), wind.end());
	sort(unwind.begin(), unwind.end());

	string finaldata = intToString(Casetype);
	for (size_t i = 0; i < unwind.size(); i++)
	{
		finaldata += intToString(unwind[i].StepPin);
		finaldata += intToString(unwind[i].DirPin);
		finaldata += intToString(unwind[i].Nstep);

	}
	for (size_t i = 0; i < wind.size(); i++)
	{
		finaldata += intToString(wind[i].StepPin);
		finaldata += intToString(wind[i].DirPin);
		finaldata += intToString(wind[i].Nstep);

	}
	finaldata += "\n";


	SerialPort arduino(port_name);

	if (arduino.isConnected()) cout << "Connection Established" << endl;
	else cout << "ERROR, check port name";

	char *c_string = new char[finaldata.size()];
	//copying the std::string to c string
	std::copy(finaldata.begin(), finaldata.end(), c_string);

	if (arduino.isConnected()) {
		arduino.writeSerialPort(c_string, MAX_DATA_LENGTH);
	}
	delete[] c_string;

	waitKey(0);


	getchar();

	return 0;
}