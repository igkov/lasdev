#ifndef __PID_H__
#define __PID_H__

typedef struct
{
	float dState;     // Last position input
	float iState;     // Integrator state
	float iMax, iMin; // Guard interval

	// Maximum and minimum allowable integrator stat
	float iGain; // integral gain
	float pGain; // proportional gain
	float dGain; // derivative gain
} pid_struct_t;

void pid_init(pid_struct_t *pid, float iGain, float pGain, float dGain, float iInt);
float pid_update(pid_struct_t *pid, float err, float position);
void pid_setlast(pid_struct_t *pid, float dState);

#endif // __PID_H__
