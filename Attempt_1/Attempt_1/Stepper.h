#pragma once


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