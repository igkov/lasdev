#ifndef __UNI_T_PROTO_H__
#define __UNI_T_PROTO_H__

enum {
    ERR_BATTERY_LOW = 101,
    ERR_CALCULATION = 104,
    ERR_HIGH_TEMP = 152,
    ERR_LOW_TEMP = 153,
    ERR_OUT_OF_RANGE = 154,
    ERR_WEAK_SIGNAL = 155,
    ERR_STRONG_SIGNAL = 156,
    ERR_BACKGROUND_ILLUM = 157,
    ERR_DEVICE_SHAKING = 160
};

int get_dist_start(void);
int get_dist_check(void);
int get_dist_end(int *dist);
int start_receive(void);

int laser_on(void);
void power_on(void);
void power_down(void);

void uni_t_init(void);

#endif // __UNI_T_PROTO_H__
