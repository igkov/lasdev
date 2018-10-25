#ifndef __PWR_H__
#define __PWR_H__

/* set_pll mode options */
#define CPU_FREQ_EQU         0
#define CPU_FREQ_LTE         1
#define CPU_FREQ_GTE         2
#define CPU_FREQ_APPROX      3

/* set_pll result0 options */
#define PLL_CMD_SUCCESS      0
#define PLL_INVALID_FREQ     1
#define PLL_INVALID_MODE     2
#define PLL_FREQ_NOT_FOUND   3
#define PLL_NOT_LOCKED       4

/* set_power mode options */
#define PWR_DEFAULT          0
#define PWR_CPU_PERFORMANCE  1
#define PWR_EFFICIENCY       2
#define PWR_LOW_CURRENT      3

/* set_power result0 options */
#define PWR_CMD_SUCCESS      0
#define PWR_INVALID_FREQ     1
#define PWR_INVALID_MODE     2

typedef struct _PWRD 
{
	void (*set_pll)(unsigned int cmd[], unsigned int resp[]);
	void (*set_power)(unsigned int cmd[], unsigned int resp[]);
} PWRD;

typedef struct _ROM 
{
	const PWRD * pWRD;
} ROM;

ROM **rom = (ROM**) (0x1FFF1FF8 + 3 * (sizeof(ROM**));
unsigned int command[4], result[2];

#endif //__PWR_H__
