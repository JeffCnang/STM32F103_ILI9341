#include "ILI93xx.h"
#include "stdlib.h"
#include "stdint.h"
#include <string.h>
#include "font.h" 

#include "main.h"
				 
//LCD的畫筆顏色和背景色	   
uint16_t POINT_COLOR=0x0000;	//畫筆顏色
uint16_t BACK_COLOR=0xFFFF;  //背景色 

//管理LCD重要參數
//默認為豎屏
_lcd_dev lcddev;
	 
//寫寄存器函數
//regval:寄存器值
void LCD_WR_REG(uint16_t regval)
{ 
	LCD->LCD_REG=regval;//寫入要寫的寄存器序號
}
//寫LCD數據
//data:要寫入的值
void LCD_WR_DATA(uint16_t data)
{										    	   
	LCD->LCD_RAM=data;
}
//讀LCD數據
//返回值:讀到的值
uint16_t LCD_RD_DATA(void)
{										    	   
	return LCD->LCD_RAM;		 
}					   
//寫寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要寫入的數據
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD->LCD_REG = LCD_Reg;		//寫入要寫的寄存器序號
	LCD->LCD_RAM = LCD_RegValue;//寫入數據
}
//讀寄存器
//LCD_Reg:寄存器地址
//返回值:讀到的數據
uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);		//寫入要讀的寄存器序號
	return LCD_RD_DATA();		//返回讀到的值
}
//開始寫GRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=lcddev.wramcmd;
}
//LCD寫GRAM
//RGB_Code:顏色值
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD->LCD_RAM = RGB_Code;//寫十六位GRAM
}
//從ILI93xx讀出的數據為GBR格式，而我們寫入的時候為RGB格式。
//通過該函數轉換
//c:GBR格式的顏色值
//返回值：RGB格式的顏色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;
	rgb=(b<<11)+(g<<5)+(r<<0);
	return(rgb);
}
//當mdk -O1時間優化時需要設置
//延時i
void opt_delay(uint8_t i)
{
	while(i--);
}
//讀取個某點的顏色值	 
//x,y:坐標
//返回值:此點的顏色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超過了範圍,直接返回		   
	LCD_SetCursor(x,y);	    
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X2E);//9341/6804/3510 發送讀GRAM指令
	//else LCD_WR_REG(R34);      		 				//其他IC發送讀GRAM指令
	if(LCD->LCD_RAM)r=0;							//dummy Read	   
	opt_delay(2);	  
 	r=LCD->LCD_RAM;  		  						//實際坐標顏色
 	if(lcddev.id==0X9341||lcddev.id==0X5310)		//9341/NT35310要分2次讀出
 	{
		opt_delay(2);	  
		b=LCD->LCD_RAM; 
		g=r&0XFF;//對於9341/5310,第一次讀取的是RG的值,R在前,G在後,各佔8位
		g<<=8;
	}
	if(lcddev.id==0X9341||lcddev.id==0X5310)return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));	//ILI9341/NT35310需要公式轉換一下
	else return LCD_BGR2RGB(r);						//其他IC
}			 
//LCD開啟顯示
void LCD_DisplayOn(void)
{					   
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X29);	//開啟顯示
}	 
//LCD關閉顯示
void LCD_DisplayOff(void)
{	   
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X28);	//關閉顯示
}   
//設置光標位置
//Xpos:橫坐標
//Ypos:縱坐標
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 
 	if(lcddev.id==0X9341||lcddev.id==0X5310)
	{		    
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(Xpos>>8); 
		LCD_WR_DATA(Xpos&0XFF);	 
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(Ypos>>8); 
		LCD_WR_DATA(Ypos&0XFF);
	}
} 		 
//設置LCD的自動掃瞄方向
//注意:其他函數可能會受到此函數設置的影響(尤其是9341/6804這兩個奇葩),
//所以,一般設置為L2R_U2D即可,如果設置為其他掃瞄方式,可能導致顯示不正常.
//dir:0~7,代表8個方向(具體定義見lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310等IC已經實際測試	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint8_t dirreg=0;
	uint16_t temp;
	if(lcddev.id==0x9341||lcddev.id==0X6804||lcddev.id==0X5310)//9341/6804/5310,很特殊
	{
		switch(dir)
		{
			case L2R_U2D://從左到右,從上到下
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://從左到右,從下到上
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://從右到左,從上到下
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://從右到左,從下到上
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://從上到下,從左到右
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://從上到下,從右到左
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://從下到上,從左到右
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://從下到上,從右到左
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;	 
		}
		dirreg=0X36;
		if(lcddev.id!=0X5310)regval|=0X08;		//攸關 RGB 顏色模式
		LCD_WriteReg(dirreg,regval);
 		if((regval&0X20)||lcddev.dir==1)
		{
			if(lcddev.width<lcddev.height)//交換X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}else  
		{
			if(lcddev.width>lcddev.height)//交換X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}  
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(0);LCD_WR_DATA(0);
		LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(0);LCD_WR_DATA(0);
		LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);  
  }
}
//畫點
//x,y:坐標
//POINT_COLOR:此點的顏色
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		//設置光標位置 
	LCD_WriteRAM_Prepare();	//開始寫入GRAM
	LCD->LCD_RAM=POINT_COLOR;
}

//快速畫點
//x,y:坐標
//color:顏色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	if(lcddev.id==0X9341||lcddev.id==0X5310)
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(x>>8);
		LCD_WR_DATA(x&0XFF);	 
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(y>>8);
		LCD_WR_DATA(y&0XFF);
	}
	LCD->LCD_REG=lcddev.wramcmd; 
	LCD->LCD_RAM=color;
}	 


//設置LCD顯示方向
//dir:0,豎屏；1,橫屏
void LCD_Display_Dir(uint8_t dir)
{
	if(dir==0)			//豎屏
	{
		lcddev.dir=0;	//豎屏
		lcddev.width=240;
		lcddev.height=320;
		if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)
		{
			lcddev.wramcmd=0X2C;
	 		lcddev.setxcmd=0X2A;
			lcddev.setycmd=0X2B;  	 
			if(lcddev.id==0X6804||lcddev.id==0X5310)
			{
				lcddev.width=320;
				lcddev.height=480;
			}
		}
	}else 				//橫屏
	{	  				
		lcddev.dir=1;	//橫屏
		lcddev.width=320;
		lcddev.height=240;
		if(lcddev.id==0X9341||lcddev.id==0X5310)
		{
			lcddev.wramcmd=0X2C;
	 		lcddev.setxcmd=0X2A;
			lcddev.setycmd=0X2B;  	 
		}
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默認掃瞄方向
}	 
//設置窗口,並自動設置畫點坐標到窗口左上角(sx,sy).
//sx,sy:窗口起始坐標(左上角)
//width,height:窗口寬度和高度,必須大於0!!
//窗體大小:width*height.
//68042,橫屏時不支持窗口設置!! 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
	width=sx+width-1;
	height=sy+height-1;
	if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X6804)//6804橫屏不支持
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(sx>>8); 
		LCD_WR_DATA(sx&0XFF);	 
		LCD_WR_DATA(width>>8); 
		LCD_WR_DATA(width&0XFF);  
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(sy>>8); 
		LCD_WR_DATA(sy&0XFF); 
		LCD_WR_DATA(height>>8); 
		LCD_WR_DATA(height&0XFF); 
	}
} 
//初始化lcd
//該初始化函數可以初始化各種ILI93XX液晶,但是其他函數是基於ILI9320的!!!
//在其他型號的驅動芯片上沒有測試! 
void LCD_Init(void)
{ 						
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
 	HAL_Delay(50);
	
 	LCD_WriteReg(0x0000,0x0001);
	HAL_Delay(50);
	
  lcddev.id = LCD_ReadReg(0x0000);   
  if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//讀到ID不正確,新增lcddev.id==0X9300判斷，因為9341在未被復位的情況下會被讀成9300
	{	
 		//嘗試9341 ID的讀取		
		LCD_WR_REG(0XD3);				   
		LCD_RD_DATA(); 				//dummy read 	
 		LCD_RD_DATA();   	    	//讀到0X00
		
  	lcddev.id=LCD_RD_DATA();   	//讀取93								   
 		lcddev.id<<=8;
		lcddev.id|=LCD_RD_DATA();  	//讀取41 	   			   
 		if(lcddev.id!=0X9341)		//非9341,嘗試是不是6804
		{	
 			LCD_WR_REG(0XBF);				   
			LCD_RD_DATA(); 			//dummy read 	 
	 		LCD_RD_DATA();   	    //讀回0X01			   
	 		LCD_RD_DATA(); 			//讀回0XD0 			  	
	  		lcddev.id=LCD_RD_DATA();//這裡讀回0X68 
			lcddev.id<<=8;
	  		lcddev.id|=LCD_RD_DATA();//這裡讀回0X04	   	  
 		} 
		if(lcddev.id!=0X9341&&lcddev.id!=0X6804)//不是9341也不是6804，嘗試看看是不是NT35310
		{
			LCD_WR_REG(0XD4);				   
			LCD_RD_DATA(); 			//dummy read  
			LCD_RD_DATA();   		//讀回0X01	 
	 		lcddev.id=LCD_RD_DATA();//讀回0X53	
			lcddev.id<<=8;	 
	  		lcddev.id|=LCD_RD_DATA();//這裡讀回0X10	 
		}
	}
 	//printf(" LCD ID:%x\r\n",lcddev.id); //打印LCD ID  
	if(lcddev.id==0X9341)	//9341初始化
	{	 
		LCD_WR_REG(0xCF);  
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0xC1); 
		LCD_WR_DATA(0X30); 
		LCD_WR_REG(0xED);  
		LCD_WR_DATA(0x64); 
		LCD_WR_DATA(0x03); 
		LCD_WR_DATA(0X12); 
		LCD_WR_DATA(0X81); 
		LCD_WR_REG(0xE8);  
		LCD_WR_DATA(0x85); 
		LCD_WR_DATA(0x10); 
		LCD_WR_DATA(0x7A); 
		LCD_WR_REG(0xCB);  
		LCD_WR_DATA(0x39); 
		LCD_WR_DATA(0x2C); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x34); 
		LCD_WR_DATA(0x02); 
		LCD_WR_REG(0xF7);  
		LCD_WR_DATA(0x20); 
		LCD_WR_REG(0xEA);  
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_REG(0xC0);    //Power control 
		LCD_WR_DATA(0x1B);   //VRH[5:0] 
		LCD_WR_REG(0xC1);    //Power control 
		LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0] 
		LCD_WR_REG(0xC5);    //VCM control 
		LCD_WR_DATA(0x30); 	 //3F
		LCD_WR_DATA(0x30); 	 //3C
		LCD_WR_REG(0xC7);    //VCM control2 
		LCD_WR_DATA(0XB7); 
		LCD_WR_REG(0x36);    // Memory Access Control 
		LCD_WR_DATA(0x48);
		
		LCD_WR_REG(0x3A);   
		LCD_WR_DATA(0x55);
		
		LCD_WR_REG(0xB1);   
		LCD_WR_DATA(0x00);   
		LCD_WR_DATA(0x1A);
		
		LCD_WR_REG(0xB6);    // Display Function Control 
		LCD_WR_DATA(0x0A);
		
		
		LCD_WR_DATA(0xA2); 
		LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
		LCD_WR_DATA(0x00); 
		LCD_WR_REG(0x26);    //Gamma curve selected 
		LCD_WR_DATA(0x01); 
		LCD_WR_REG(0xE0);    //Set Gamma 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x2A); 
		LCD_WR_DATA(0x28); 
		LCD_WR_DATA(0x08); 
		LCD_WR_DATA(0x0E); 
		LCD_WR_DATA(0x08); 
		LCD_WR_DATA(0x54); 
		LCD_WR_DATA(0XA9); 
		LCD_WR_DATA(0x43); 
		LCD_WR_DATA(0x0A); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 		 
		LCD_WR_REG(0XE1);    //Set Gamma 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x15); 
		LCD_WR_DATA(0x17); 
		LCD_WR_DATA(0x07); 
		LCD_WR_DATA(0x11); 
		LCD_WR_DATA(0x06); 
		LCD_WR_DATA(0x2B); 
		LCD_WR_DATA(0x56); 
		LCD_WR_DATA(0x3C); 
		LCD_WR_DATA(0x05); 
		LCD_WR_DATA(0x10); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x3F); 
		LCD_WR_DATA(0x3F); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_REG(0x2B); 
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x01);
		LCD_WR_DATA(0x3f);
		LCD_WR_REG(0x2A); 
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0xef);	 
		LCD_WR_REG(0x11); //Exit Sleep
		HAL_Delay(120);
		LCD_WR_REG(0x29); //display on	
	}
	
	LCD_Display_Dir(0);		 	//默認為豎屏
	//LCD_LED=1;					//點亮背光
	LCD_Clear(BLUE);
}  
//清屏函數
//color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
	uint32_t index=0;      
	uint32_t totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 			//得到總點數
	if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804橫屏的時候特殊處理  
	{						    
 		lcddev.dir=0;	 
 		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 			
		LCD_SetCursor(0x00,0x0000);		//設置光標位置  
 		lcddev.dir=1;	 
  		lcddev.setxcmd=0X2B;
		lcddev.setycmd=0X2A;  	 
 	}else LCD_SetCursor(0x00,0x0000);	//設置光標位置 
	LCD_WriteRAM_Prepare();     		//開始寫入GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	   
	}
}  
//在指定區域內填充單個顏色
//(sx,sy),(ex,ey):填充矩形對角坐標,區域大小為:(ex-sx+1)*(ey-sy+1)   
//color:要填充的顏色
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
	uint16_t i,j;
	uint16_t xlen=0;
	uint16_t temp;
	if((lcddev.id==0X6804)&&(lcddev.dir==1))	//6804橫屏的時候特殊處理  
	{
		temp=sx;
		sx=sy;
		sy=lcddev.width-ex-1;	  
		ex=ey;
		ey=lcddev.width-temp-1;
 		lcddev.dir=0;	 
 		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;
		LCD_Fill(sx,sy,ex,ey,color);
 		lcddev.dir=1;
  		lcddev.setxcmd=0X2B;
		lcddev.setycmd=0X2A;
 	}else
	{
		xlen=ex-sx+1;
		for(i=sy;i<=ey;i++)
		{
		 	LCD_SetCursor(sx,i);      				//設置光標位置 
			LCD_WriteRAM_Prepare();     			//開始寫入GRAM	  
			for(j=0;j<xlen;j++)LCD_WR_DATA(color);	//設置光標位置 	    
		}
	}	 
}  
//在指定區域內填充指定顏色塊			 
//(sx,sy),(ex,ey):填充矩形對角坐標,區域大小為:(ex-sx+1)*(ey-sy+1)   
//color:要填充的顏色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	uint16_t height,width;
	uint16_t i,j;
	width=ex-sx+1; 			//得到填充的寬度
	height=ey-sy+1;			//高度
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//設置光標位置 
		LCD_WriteRAM_Prepare();     //開始寫入GRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*height+j];//寫入數據 
	}	  
}  
//畫線
//x1,y1:起點坐標
//x2,y2:終點坐標  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //計算坐標增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //設置單步方向 
	else if(delta_x==0)incx=0;//垂直線 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平線 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //選取基本增量坐標軸 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//畫線輸出 
	{  
		LCD_DrawPoint(uRow,uCol);//畫點 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance;
			uCol+=incy; 
		} 
	}  
}    
//畫矩形	  
//(x1,y1),(x2,y2):矩形的對角坐標
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置畫一個指定大小的圓
//(x,y):中心點
//r    :半徑

void Draw_Circle_Fill(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判斷下個點位置的標誌
	while(a<=b)
	{
		LCD_DrawLine(x0,y0,x0+a,y0-b);             //5
		LCD_DrawLine(x0,y0,x0+b,y0-a);             //0
		LCD_DrawLine(x0,y0,x0+b,y0+a);             //4
		LCD_DrawLine(x0,y0,x0+a,y0+b);             //6
		LCD_DrawLine(x0,y0,x0-a,y0+b);             //1
		LCD_DrawLine(x0,y0,x0-b,y0+a);
		LCD_DrawLine(x0,y0,x0-a,y0-b);             //2
		LCD_DrawLine(x0,y0,x0-b,y0-a);             //7
		a++;
		//使用Bresenham算法畫圓
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}

void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判斷下個點位置的標誌
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+b,y0-a);             //0
		LCD_DrawPoint(x0+b,y0+a);             //4
		LCD_DrawPoint(x0+a,y0+b);             //6
		LCD_DrawPoint(x0-a,y0+b);             //1
		LCD_DrawPoint(x0-b,y0+a);
		LCD_DrawPoint(x0-a,y0-b);             //2
		LCD_DrawPoint(x0-b,y0-a);             //7
		a++;
		//使用Bresenham算法畫圓
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}

//在指定位置顯示一個字符
//x,y:起始坐標
//num:要顯示的字符:" "--->"~"
//size:字體大小 12/16
//mode:疊加方式(1)還是非疊加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
  uint8_t temp,t1,t;
	uint16_t y0=y;
	uint16_t colortemp=POINT_COLOR;      			     
	//設置窗口		   
	num=num-' ';//得到偏移後的值
	if(!mode) //非疊加方式
	{
	    for(t=0;t<size;t++)
	    {   
				if(size==12)	temp=asc2_1206[num][t];  //調用1206字體
				else temp=asc2_1608[num][t];		 //調用1608字體
					for(t1=0;t1<8;t1++)
					{			    
						if(temp&0x80)	POINT_COLOR=colortemp;
						else 					POINT_COLOR=BACK_COLOR;
						LCD_DrawPoint(x,y);
						temp<<=1;
						y++;
						if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超區域了
						if((y-y0)==size)
						{
							y=y0;
							x++;
							if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超區域了
							break;
						}
					}  	 
	    }    
	}else//疊加方式
	{
	    for(t=0;t<size;t++)
	    {   
				if(size==12)temp=asc2_1206[num][t];  //調用1206字體
				else temp=asc2_1608[num][t];		 //調用1608字體 	                          
				for(t1=0;t1<8;t1++)
				{			    
					if(temp&0x80)LCD_DrawPoint(x,y); 
					temp<<=1;
					y++;
					if(x>=lcddev.height){POINT_COLOR=colortemp;return;}//超區域了
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超區域了
						break;
					}
				}  	 
	    }     
	}
	POINT_COLOR=colortemp;
}   
//m^n函數
//返回值:m^n次方.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//顯示數字,高位為0,則不顯示
//x,y :起點坐標	 
//len :數字的位數
//size:字體大小
//color:顏色 
//num:數值(0~4294967295);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//顯示數字,高位為0,還是顯示
//x,y:起點坐標
//num:數值(0~999999999);	 
//len:長度(即要顯示的位數)
//size:字體大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非疊加顯示;1,疊加顯示.
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01);
	}
} 
//顯示字符串
//x,y:起點坐標
//width,height:區域大小  
//size:字體大小
//*p:字符串起始地址		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p)
{         
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判斷是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}





