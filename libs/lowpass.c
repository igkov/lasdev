#include "lowpass.h"

float lowpass(float x, float a, float xp)
{
	xp = a * x + (1 - a) * xp;
	return xp;
}

float lowpass_smart(float x, pfilter_t filter)
{
#if 1
	float delta = ABS(x - filter->xp);
	// a = [0.0f, 1.0f];
	if (delta > filter->max_d)
		filter->xp = x;
	else
		filter->xp = filter->a * x + (1 - filter->a) * filter->xp;
	return filter->xp;
#else
	float a = filter->a;
	float delta = ABS(x - filter->xp);
	// a = [0.0f, 1.0f];
	if (delta > PI/2)
	{
		a = 1.0f;
	}
	else
	{
		// delta = [0.0f, 1.0f]
		delta /= (PI/2);
	}
	filter->xp = a * x + (1 - a) * filter->xp;
	return filter->xp;
#endif
}
