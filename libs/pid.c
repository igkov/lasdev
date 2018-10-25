/**
	PID regulator code

	Based on Example from wescott website.
 */

#include "pid.h"

void pid_init(pid_struct_t *pid, float iGain, float pGain, float dGain, float iInt)
{
	//
	pid->iGain = iGain;
	pid->pGain = pGain;
	pid->dGain = dGain;

	// 
	pid->dState = 0.0f; // last val
	pid->iState = 0.0f;

	// 
	pid->iMax = iInt;
	pid->iMin = -iInt;
}

void pid_setlast(pid_struct_t *pid, float dState)
{
	pid->dState = dState;
}

float pid_update(pid_struct_t *pid, float targetPosition, float currentPosition)
{
	float pTerm;
	float dTerm;
	float iTerm;
	
	// Calculate delta (error):
	float err = targetPosition - currentPosition;

	// Calculate the proportional term
	pTerm = pid->pGain * err;

	// Calculate the integral state with appropriate limiting
	pid->iState += err;

	// Guard interval check:
	if (pid->iState > pid->iMax) pid->iState = pid->iMax;
	else if (pid->iState < pid->iMin) pid->iState = pid->iMin;

	// Calculate the integral term:
	iTerm = pid->iGain * pid->iState; 

	// Calculate the differential term:
	dTerm = pid->dGain * (pid->dState - currentPosition);

	// Set Last Position:
	pid->dState = currentPosition;

	return pTerm + dTerm + iTerm;
}
