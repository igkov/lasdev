#ifndef __LCD_H__
#define __LCD_H__

#define LCD_SET_BIT(n) {lcd_frame[(n)/8]|=(1<<((n)%8));}
#define LCD_CLR_BIT(n) {lcd_frame[(n)/8]&=~(1<<((n)%8));}

#define LCD_LINE1_SEG1_A  (154)
#define LCD_LINE1_SEG1_B  (165)
#define LCD_LINE1_SEG1_C  (166)
#define LCD_LINE1_SEG1_D  (168)
#define LCD_LINE1_SEG1_E  (170)
#define LCD_LINE1_SEG1_F  (169)
#define LCD_LINE1_SEG1_G  (167)

#define LCD_LINE1_SEG2_A  (174)
#define LCD_LINE1_SEG2_B  (176)
#define LCD_LINE1_SEG2_C  (177)
#define LCD_LINE1_SEG2_D  (179)
#define LCD_LINE1_SEG2_E  (164)
#define LCD_LINE1_SEG2_F  (163)
#define LCD_LINE1_SEG2_G  (178)

#define LCD_LINE1_SEG3_A  (183)
#define LCD_LINE1_SEG3_B  (185)
#define LCD_LINE1_SEG3_C  (186)
#define LCD_LINE1_SEG3_D  (188)
#define LCD_LINE1_SEG3_E  (173)
#define LCD_LINE1_SEG3_F  (172)
#define LCD_LINE1_SEG3_G  (187)

#define LCD_LINE1_SEG4_A  (192)
#define LCD_LINE1_SEG4_B  (194)
#define LCD_LINE1_SEG4_C  (195)
#define LCD_LINE1_SEG4_D  (197)
#define LCD_LINE1_SEG4_E  (182)
#define LCD_LINE1_SEG4_F  (181)
#define LCD_LINE1_SEG4_G  (196)

#define LCD_LINE1_SEG5_A  (201)
#define LCD_LINE1_SEG5_B  (203)
#define LCD_LINE1_SEG5_C  (204)
#define LCD_LINE1_SEG5_D  (206)
#define LCD_LINE1_SEG5_E  (191)
#define LCD_LINE1_SEG5_F  (190)
#define LCD_LINE1_SEG5_G  (205)

#define LCD_LINE1_SEGX_A  (199)
#define LCD_LINE1_SEGX_B  (210)
#define LCD_LINE1_SEGX_C  (211)
#define LCD_LINE1_SEGX_D  (213)
#define LCD_LINE1_SEGX_E  (215)
#define LCD_LINE1_SEGX_F  (214)
#define LCD_LINE1_SEGX_G  (212)

#define LCD_LINE2_SEG1_A  (227)
#define LCD_LINE2_SEG1_B  (238)
#define LCD_LINE2_SEG1_C  (237)
#define LCD_LINE2_SEG1_D  (239)
#define LCD_LINE2_SEG1_E  (241)
#define LCD_LINE2_SEG1_F  (242)
#define LCD_LINE2_SEG1_G  (240)

#define LCD_LINE2_SEG2_A  (247)
#define LCD_LINE2_SEG2_B  (249)
#define LCD_LINE2_SEG2_C  (248)
#define LCD_LINE2_SEG2_D  (250)
#define LCD_LINE2_SEG2_E  (235)
#define LCD_LINE2_SEG2_F  (236)
#define LCD_LINE2_SEG2_G  (251)

#define LCD_LINE2_SEG3_A  (256)
#define LCD_LINE2_SEG3_B  (258)
#define LCD_LINE2_SEG3_C  (257)
#define LCD_LINE2_SEG3_D  (259)
#define LCD_LINE2_SEG3_E  (244)
#define LCD_LINE2_SEG3_F  (245)
#define LCD_LINE2_SEG3_G  (260)

#define LCD_LINE2_SEG4_A  (265)
#define LCD_LINE2_SEG4_B  (267)
#define LCD_LINE2_SEG4_C  (266)
#define LCD_LINE2_SEG4_D  (268)
#define LCD_LINE2_SEG4_E  (253)
#define LCD_LINE2_SEG4_F  (254)
#define LCD_LINE2_SEG4_G  (269)

#define LCD_LINE2_SEG5_A  (274)
#define LCD_LINE2_SEG5_B  (276)
#define LCD_LINE2_SEG5_C  (275)
#define LCD_LINE2_SEG5_D  (277)
#define LCD_LINE2_SEG5_E  (262)
#define LCD_LINE2_SEG5_F  (263)
#define LCD_LINE2_SEG5_G  (278)

#define LCD_LINE2_SEGX_A  (272)
#define LCD_LINE2_SEGX_B  (283)
#define LCD_LINE2_SEGX_C  (282)
#define LCD_LINE2_SEGX_D  (284)
#define LCD_LINE2_SEGX_E  (286)
#define LCD_LINE2_SEGX_F  (287)
#define LCD_LINE2_SEGX_G  (285)

#define LCD_LINE3_SEG1_A  (88)
#define LCD_LINE3_SEG1_B  (97)
#define LCD_LINE3_SEG1_C  (98)
#define LCD_LINE3_SEG1_D  (83)
#define LCD_LINE3_SEG1_E  (85)
#define LCD_LINE3_SEG1_F  (84)
#define LCD_LINE3_SEG1_G  (82)

#define LCD_LINE3_SEG2_A  (106)
#define LCD_LINE3_SEG2_B  (91)
#define LCD_LINE3_SEG2_C  (92)
#define LCD_LINE3_SEG2_D  (94)
#define LCD_LINE3_SEG2_E  (96)
#define LCD_LINE3_SEG2_F  (95)
#define LCD_LINE3_SEG2_G  (93)

#define LCD_LINE3_SEG3_A  (115)
#define LCD_LINE3_SEG3_B  (100)
#define LCD_LINE3_SEG3_C  (101)
#define LCD_LINE3_SEG3_D  (103)
#define LCD_LINE3_SEG3_E  (105)
#define LCD_LINE3_SEG3_F  (104)
#define LCD_LINE3_SEG3_G  (102)

#define LCD_LINE3_SEG4_A  (124)
#define LCD_LINE3_SEG4_B  (109)
#define LCD_LINE3_SEG4_C  (110)
#define LCD_LINE3_SEG4_D  (112)
#define LCD_LINE3_SEG4_E  (114)
#define LCD_LINE3_SEG4_F  (113)
#define LCD_LINE3_SEG4_G  (111)

#define LCD_LINE3_SEG5_A  (129)
#define LCD_LINE3_SEG5_B  (133)
#define LCD_LINE3_SEG5_C  (134)
#define LCD_LINE3_SEG5_D  (119)
#define LCD_LINE3_SEG5_E  (121)
#define LCD_LINE3_SEG5_F  (120)
#define LCD_LINE3_SEG5_G  (118)

#define LCD_LINE3_SEGX_A  (127)
#define LCD_LINE3_SEGX_B  (138)
#define LCD_LINE3_SEGX_C  (139)
#define LCD_LINE3_SEGX_D  (141)
#define LCD_LINE3_SEGX_E  (143)
#define LCD_LINE3_SEGX_F  (142)
#define LCD_LINE3_SEGX_G  (140)

#define LCD_LINE1_N       (230)
#define LCD_LINE1_N1      (232)

#define LCD_LINE1_MINUS   (156)
#define LCD_LINE1_MINUSP  (157)
#define LCD_LINE2_MINUS   (229)
#define LCD_LINE2_MINUSP  (228)
#define LCD_LINE3_MINUS   (86)
#define LCD_LINE3_MINUSP  (87)

#define LCD_LINE1_DP1     (175)
#define LCD_LINE1_DP2     (184)
#define LCD_LINE1_DP3     (193)
#define LCD_LINE2_DP1     (246)
#define LCD_LINE2_DP2     (255)
#define LCD_LINE2_DP3     (264)
#define LCD_LINE3_DP1     (107)
#define LCD_LINE3_DP2     (116)
#define LCD_LINE3_DP3     (125)

#define LCD_BATT_COVER    (73)
#define LCD_BATT_SEG1     (76)
#define LCD_BATT_SEG2     (75)
#define LCD_BATT_SEG3     (74)

#define LCD_SETUP_MAIN    (20)
#define LCD_SETUP_A       (24)
#define LCD_SETUP_B1      (21)
#define LCD_SETUP_B2      (22)
#define LCD_SETUP_MAX     (25)
#define LCD_SETUP_MIN     (19)
#define LCD_SETUP_LIN     (23)

#define LCD_SETUP_BODY    (34)
#define LCD_SETUP_LAS     (35)
#define LCD_SETUP_1D      (42)
#define LCD_SETUP_STAR    (43)

#define LCD_SYMBOL_0     (0)
#define LCD_SYMBOL_1     (1)
#define LCD_SYMBOL_2     (2)
#define LCD_SYMBOL_3     (3)
#define LCD_SYMBOL_4     (4)
#define LCD_SYMBOL_5     (5)
#define LCD_SYMBOL_6     (6)
#define LCD_SYMBOL_7     (7)
#define LCD_SYMBOL_8     (8)
#define LCD_SYMBOL_9     (9)
#define LCD_SYMBOL_A     (10)
#define LCD_SYMBOL_B     (11)
#define LCD_SYMBOL_C     (12)
#define LCD_SYMBOL_D     (13)
#define LCD_SYMBOL_E     (14)
#define LCD_SYMBOL_F     (15)
#define LCD_SYMBOL_r     (16)
#define LCD_SYMBOL_L     (17)
#define LCD_SYMBOL_t     (18)
#define LCD_SYMBOL_P     (19)
#define LCD_SYMBOL_SPACE (20)
#define LCD_SYMBOL_MINUS (21)
//#define LCD_SYMBOL_0     (0)

void put_tilt(int value);
void put_azimuth(int value);
void put_dist(int value);
void put_clr(int line);
void put_cnt(int value);

void lcd_arr(const int16_t *arr, int flag);
uint16_t get_lcd_word(int n);

void set_frame(uint8_t *array, int size);
#if defined( WIN32 )
void get_frame(uint8_t *array, int size);
void lcd_bit_access(int pos, int flag);
#else
void send_frame(void);
void send_null_frame(void);
#endif

extern const int16_t lcd_arr_batt[];
extern const int16_t lcd_las_level[];
extern const int16_t lcd_arr_las[];
extern const int16_t lcd_unit[];
//extern const int16_t lcd_hello[];
//extern const int16_t lcd_err[];
extern const int16_t lcd_cal[];
extern const int16_t lcd_of_32[];
extern const int16_t lcd_end[];
extern const int16_t lcd_setup[];
extern const int16_t lcd_line_1_nop[];
extern const int16_t lcd_line_2_nop[];
extern const int16_t lcd_line_3_nop[];
extern const int16_t lcd_line_1_err[];
extern const int16_t lcd_line_2_err[];
extern const int16_t lcd_line_3_err[];

#endif // __LCD_H__
