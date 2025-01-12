#ifndef __LCD_H
#define __LCD_H		
#include "stdint.h" 
#include "stdlib.h"

  
//LCD重要參數集
typedef struct  
{										    
	uint16_t width;			//LCD 寬度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//橫屏還是豎屏控制：0，豎屏；1，橫屏。	
	uint8_t	wramcmd;		//開始寫gram指令
	uint8_t  setxcmd;		//設置x坐標指令
	uint8_t  setycmd;		//設置y坐標指令	 
}_lcd_dev;

//LCD參數
extern _lcd_dev lcddev;	//管理LCD重要參數
//LCD的畫筆顏色和背景色	   
extern uint16_t  POINT_COLOR;//默認紅色
extern uint16_t  BACK_COLOR; //背景顏色.默認為白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD端口定義---------------- 
//LCD地址結構體
typedef struct
{
	uint16_t LCD_REG;
	uint16_t LCD_RAM;
} LCD_TypeDef;
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A10作為數據命令區分線 
//注意設置時STM32內部會右移一位對其! 111110=0X3E			    
#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//掃瞄方向定義
#define L2R_U2D  0 //從左到右,從上到下
#define L2R_D2U  1 //從左到右,從下到上
#define R2L_U2D  2 //從右到左,從上到下
#define R2L_D2U  3 //從右到左,從下到上

#define U2D_L2R  4 //從上到下,從左到右
#define U2D_R2L  5 //從上到下,從右到左
#define D2U_L2R  6 //從下到上,從左到右
#define D2U_R2L  7 //從下到上,從右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默認的掃瞄方向

#define WHITE		0xFFFF
#define BLACK		0x0000
#define BLUE		0x001F
#define BRED		0XF81F
#define GRED		0XFFE0
#define GBLUE		0X07FF
#define RED			0xF800
#define	DARKRED	0xA800
#define MAGENTA	0xF81F
#define GREEN		0x07E0
#define CYAN		0x7FFF
#define YELLOW	0xFFE0
#define BROWN		0XBC40
#define BRRED		0XFC07
#define GRAY		0X8430

#define DARKBLUE      	 0X01CF	//深藍色
#define LIGHTBLUE      	 0X7D7C	//淺藍色  
#define GRAYBLUE       	 0X5458 //灰藍色
//以上三色為PANEL的顏色 
 
#define LIGHTGREEN     	 0X841F //淺綠色
//#define LIGHTGRAY        0XEF5B //淺灰色(PANNEL)
#define LGRAY 			 0XC618 //淺灰色(PANNEL),窗體背景色

#define LGRAYBLUE        0XA651 //淺灰藍色(中間層顏色)
#define LBBLUE           0X2B12 //淺棕藍色(選擇條目的反色)
	    															  
void LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//開顯示
void LCD_DisplayOff(void);													//關顯示
void LCD_Clear(uint16_t Color);	 												//清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);										//設置光標
void LCD_DrawPoint(uint16_t x,uint16_t y);											//畫點
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color);								//快速畫點
uint16_t  LCD_ReadPoint(uint16_t x,uint16_t y); 											//讀點 
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);										//畫圓
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);							//畫線
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   				//畫矩形
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);		   				//填充單色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);				//填充指定顏色
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);						//顯示一個字符
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);  						//顯示一個數字
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//顯示 數字
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p);		//顯示一個字符串,12/16字體

void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint8_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);		  
void LCD_Scan_Dir(uint8_t dir);							//設置屏掃瞄方向
void LCD_Display_Dir(uint8_t dir);						//設置屏幕顯示方向
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);//設置窗口					   																			 
int RGB888_to_RGB565(uint8_t r, uint8_t g, uint8_t b);


void UI1(void);
void UI2(void);
void UI3(void);

//9320/9325 LCD寄存器  
#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5							  		 
#endif  
	 
	 



