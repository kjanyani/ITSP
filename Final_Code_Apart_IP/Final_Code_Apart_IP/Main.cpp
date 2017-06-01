#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SerialPort.h"
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include<conio.h>
#include<opencv2\opencv.hpp>
#include <vector>
#include <algorithm>
#define _USE_MATH_DEFINES ;
#include <math.h>
#include <sstream>

using namespace std;
using namespace cv;

string NumToString(int number)
{
	std::ostringstream ss;
	ss << number;
	return ss.str();
}

class Stepper
{
public:
	int StepPin, DirPin, Nstep; // step pin no. , Dirpin no. , no of steps to move respectively
	bool windingState; // takes value 1 for winding and 0 for unwinding
	Stepper()
	{	}
	
	~Stepper()
	{}

	void operator= (const Stepper& S)
	{
		this->StepPin = S.StepPin;
		this->DirPin = S.DirPin;
		this->Nstep = S.Nstep;
		this->windingState = S.windingState;

		return;
	}
	bool operator< (const Stepper& S)
	{
		return (this->Nstep > S.Nstep);
	}

};


//String for getting the output from arduino
char output[MAX_DATA_LENGTH];

/*Portname must contain these backslashes, and remember to
replace the following com port*/
char *port_name = "\\\\.\\COM3";

//String for incoming data
char incomingData[MAX_DATA_LENGTH];

int main()

{
	double xf = 10, yf = 200;
	Stepper RT, RB, LT, LB; // Right top, Right bottom, Left Top, Left Bottom steppers.


	double radpulley = 40;
	double radmount = 18.98;

	double xRT=0, yRT=665, xRB=0, yRB=0, xLB=695, yLB=0, xLT=695, yLT=680, x0=210, y0=52 ;
	double lengthRT = sqrt((xRT - xf)*(xRT - xf) + (yRT - yf)*(yRT - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt( (xRT - x0)*(xRT - x0) + (yRT - y0)*(yRT - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthRB = sqrt((xRB - xf)*(xRB - xf) + (yRB - yf)*(yRB - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt( (xRB - x0)*(xRB - x0) + (yRB - y0)*(xRB - x0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthLT = sqrt((xLT - xf)*(xLT - xf) + (yLT - yf)*(yLT - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt( (xLT - x0)*(xLT - x0) + (yLT - y0)*(yLT - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length
	double lengthLB = sqrt((xLB - xf)*(xLB - xf) + (yLB - yf)*(yLB - yf) - (radpulley - radmount)*(radpulley - radmount)) - sqrt( (xLB - x0)*(xLB - x0) + (yLB - y0)*(yLB - y0) - (radpulley - radmount)*(radpulley - radmount)); // final length - initial length

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

	string finaldata = NumToString(Casetype);
	for (size_t i = 0; i < unwind.size(); i++)
	{
		finaldata += NumToString(unwind[i].StepPin);
		finaldata += NumToString(unwind[i].DirPin);
		finaldata += NumToString(unwind[i].Nstep);

	}
	for (size_t i = 0; i < wind.size(); i++)
	{
		finaldata += NumToString(wind[i].StepPin);
		finaldata += NumToString(wind[i].DirPin);
		finaldata += NumToString(wind[i].Nstep);

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

	getch();
}
