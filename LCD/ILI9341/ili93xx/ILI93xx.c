#include "ILI93xx.h"
#include "stdlib.h"
#include "stdint.h"
#include <string.h>
#include "font.h" 

#include "main.h"
				 
//LCD���e���C��M�I����	   
uint16_t POINT_COLOR=0x0000;	//�e���C��
uint16_t BACK_COLOR=0xFFFF;  //�I���� 

//�޲zLCD���n�Ѽ�
//�q�{���ݫ�
_lcd_dev lcddev;
	 
//�g�H�s�����
//regval:�H�s����
void LCD_WR_REG(uint16_t regval)
{ 
	LCD->LCD_REG=regval;//�g�J�n�g���H�s���Ǹ�
}
//�gLCD�ƾ�
//data:�n�g�J����
void LCD_WR_DATA(uint16_t data)
{										    	   
	LCD->LCD_RAM=data;
}
//ŪLCD�ƾ�
//��^��:Ū�쪺��
uint16_t LCD_RD_DATA(void)
{										    	   
	return LCD->LCD_RAM;		 
}					   
//�g�H�s��
//LCD_Reg:�H�s���a�}
//LCD_RegValue:�n�g�J���ƾ�
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD->LCD_REG = LCD_Reg;		//�g�J�n�g���H�s���Ǹ�
	LCD->LCD_RAM = LCD_RegValue;//�g�J�ƾ�
}
//Ū�H�s��
//LCD_Reg:�H�s���a�}
//��^��:Ū�쪺�ƾ�
uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);		//�g�J�nŪ���H�s���Ǹ�
	return LCD_RD_DATA();		//��^Ū�쪺��
}
//�}�l�gGRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=lcddev.wramcmd;
}
//LCD�gGRAM
//RGB_Code:�C���
void LCD_WriteRAM(uint16_t RGB_Code)
{
	LCD->LCD_RAM = RGB_Code;//�g�Q����GRAM
}
//�qILI93xxŪ�X���ƾڬ�GBR�榡�A�ӧڭ̼g�J���ɭԬ�RGB�榡�C
//�q�L�Ө���ഫ
//c:GBR�榡���C���
//��^�ȡGRGB�榡���C���
uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;
	rgb=(b<<11)+(g<<5)+(r<<0);
	return(rgb);
}
//��mdk -O1�ɶ��u�Ʈɻݭn�]�m
//����i
void opt_delay(uint8_t i)
{
	while(i--);
}
//Ū���ӬY�I���C���	 
//x,y:����
//��^��:���I���C��
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//�W�L�F�d��,������^		   
	LCD_SetCursor(x,y);	    
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X2E);//9341/6804/3510 �o�eŪGRAM���O
	//else LCD_WR_REG(R34);      		 				//��LIC�o�eŪGRAM���O
	if(LCD->LCD_RAM)r=0;							//dummy Read	   
	opt_delay(2);	  
 	r=LCD->LCD_RAM;  		  						//��ڧ����C��
 	if(lcddev.id==0X9341||lcddev.id==0X5310)		//9341/NT35310�n��2��Ū�X
 	{
		opt_delay(2);	  
		b=LCD->LCD_RAM; 
		g=r&0XFF;//���9341/5310,�Ĥ@��Ū�����ORG����,R�b�e,G�b��,�U��8��
		g<<=8;
	}
	if(lcddev.id==0X9341||lcddev.id==0X5310)return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));	//ILI9341/NT35310�ݭn�����ഫ�@�U
	else return LCD_BGR2RGB(r);						//��LIC
}			 
//LCD�}�����
void LCD_DisplayOn(void)
{					   
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X29);	//�}�����
}	 
//LCD�������
void LCD_DisplayOff(void)
{	   
	if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X28);	//�������
}   
//�]�m���Ц�m
//Xpos:���
//Ypos:�a����
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
//�]�mLCD���۰ʱ��ˤ�V
//�`�N:��L��ƥi��|���즹��Ƴ]�m���v�T(�ר�O9341/6804�o��ө_��),
//�ҥH,�@��]�m��L2R_U2D�Y�i,�p�G�]�m����L���ˤ覡,�i��ɭP��ܤ����`.
//dir:0~7,�N��8�Ӥ�V(����w�q��lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310��IC�w�g��ڴ���	   	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	uint8_t dirreg=0;
	uint16_t temp;
	if(lcddev.id==0x9341||lcddev.id==0X6804||lcddev.id==0X5310)//9341/6804/5310,�ܯS��
	{
		switch(dir)
		{
			case L2R_U2D://�q����k,�q�W��U
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://�q����k,�q�U��W
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://�q�k�쥪,�q�W��U
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://�q�k�쥪,�q�U��W
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://�q�W��U,�q����k
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://�q�W��U,�q�k�쥪
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://�q�U��W,�q����k
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://�q�U��W,�q�k�쥪
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;	 
		}
		dirreg=0X36;
		if(lcddev.id!=0X5310)regval|=0X08;		//���� RGB �C��Ҧ�
		LCD_WriteReg(dirreg,regval);
 		if((regval&0X20)||lcddev.dir==1)
		{
			if(lcddev.width<lcddev.height)//�洫X,Y
			{
				temp=lcddev.width;
				lcddev.width=lcddev.height;
				lcddev.height=temp;
 			}
		}else  
		{
			if(lcddev.width>lcddev.height)//�洫X,Y
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
//�e�I
//x,y:����
//POINT_COLOR:���I���C��
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		//�]�m���Ц�m 
	LCD_WriteRAM_Prepare();	//�}�l�g�JGRAM
	LCD->LCD_RAM=POINT_COLOR;
}

//�ֳt�e�I
//x,y:����
//color:�C��
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


//�]�mLCD��ܤ�V
//dir:0,�ݫ̡F1,���
void LCD_Display_Dir(uint8_t dir)
{
	if(dir==0)			//�ݫ�
	{
		lcddev.dir=0;	//�ݫ�
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
	}else 				//���
	{	  				
		lcddev.dir=1;	//���
		lcddev.width=320;
		lcddev.height=240;
		if(lcddev.id==0X9341||lcddev.id==0X5310)
		{
			lcddev.wramcmd=0X2C;
	 		lcddev.setxcmd=0X2A;
			lcddev.setycmd=0X2B;  	 
		}
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//�q�{���ˤ�V
}	 
//�]�m���f,�æ۰ʳ]�m�e�I���Ш쵡�f���W��(sx,sy).
//sx,sy:���f�_�l����(���W��)
//width,height:���f�e�שM����,�����j��0!!
//����j�p:width*height.
//68042,��̮ɤ�������f�]�m!! 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
	width=sx+width-1;
	height=sy+height-1;
	if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X6804)//6804��̤����
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
//��l��lcd
//�Ӫ�l�ƨ�ƥi�H��l�ƦU��ILI93XX�G��,���O��L��ƬO���ILI9320��!!!
//�b��L�������X�ʪ���W�S������! 
void LCD_Init(void)
{ 						
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
 	HAL_Delay(50);
	
 	LCD_WriteReg(0x0000,0x0001);
	HAL_Delay(50);
	
  lcddev.id = LCD_ReadReg(0x0000);   
  if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//Ū��ID�����T,�s�Wlcddev.id==0X9300�P�_�A�]��9341�b���Q�_�쪺���p�U�|�QŪ��9300
	{	
 		//����9341 ID��Ū��		
		LCD_WR_REG(0XD3);				   
		LCD_RD_DATA(); 				//dummy read 	
 		LCD_RD_DATA();   	    	//Ū��0X00
		
  	lcddev.id=LCD_RD_DATA();   	//Ū��93								   
 		lcddev.id<<=8;
		lcddev.id|=LCD_RD_DATA();  	//Ū��41 	   			   
 		if(lcddev.id!=0X9341)		//�D9341,���լO���O6804
		{	
 			LCD_WR_REG(0XBF);				   
			LCD_RD_DATA(); 			//dummy read 	 
	 		LCD_RD_DATA();   	    //Ū�^0X01			   
	 		LCD_RD_DATA(); 			//Ū�^0XD0 			  	
	  		lcddev.id=LCD_RD_DATA();//�o��Ū�^0X68 
			lcddev.id<<=8;
	  		lcddev.id|=LCD_RD_DATA();//�o��Ū�^0X04	   	  
 		} 
		if(lcddev.id!=0X9341&&lcddev.id!=0X6804)//���O9341�]���O6804�A���լݬݬO���ONT35310
		{
			LCD_WR_REG(0XD4);				   
			LCD_RD_DATA(); 			//dummy read  
			LCD_RD_DATA();   		//Ū�^0X01	 
	 		lcddev.id=LCD_RD_DATA();//Ū�^0X53	
			lcddev.id<<=8;	 
	  		lcddev.id|=LCD_RD_DATA();//�o��Ū�^0X10	 
		}
	}
 	//printf(" LCD ID:%x\r\n",lcddev.id); //���LLCD ID  
	if(lcddev.id==0X9341)	//9341��l��
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
	
	LCD_Display_Dir(0);		 	//�q�{���ݫ�
	//LCD_LED=1;					//�I�G�I��
	LCD_Clear(BLUE);
}  
//�M�̨��
//color:�n�M�̪���R��
void LCD_Clear(uint16_t color)
{
	uint32_t index=0;      
	uint32_t totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 			//�o���`�I��
	if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804��̪��ɭԯS��B�z  
	{						    
 		lcddev.dir=0;	 
 		lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  	 			
		LCD_SetCursor(0x00,0x0000);		//�]�m���Ц�m  
 		lcddev.dir=1;	 
  		lcddev.setxcmd=0X2B;
		lcddev.setycmd=0X2A;  	 
 	}else LCD_SetCursor(0x00,0x0000);	//�]�m���Ц�m 
	LCD_WriteRAM_Prepare();     		//�}�l�g�JGRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	   
	}
}  
//�b���w�ϰ줺��R����C��
//(sx,sy),(ex,ey):��R�x�ι﨤����,�ϰ�j�p��:(ex-sx+1)*(ey-sy+1)   
//color:�n��R���C��
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
	uint16_t i,j;
	uint16_t xlen=0;
	uint16_t temp;
	if((lcddev.id==0X6804)&&(lcddev.dir==1))	//6804��̪��ɭԯS��B�z  
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
		 	LCD_SetCursor(sx,i);      				//�]�m���Ц�m 
			LCD_WriteRAM_Prepare();     			//�}�l�g�JGRAM	  
			for(j=0;j<xlen;j++)LCD_WR_DATA(color);	//�]�m���Ц�m 	    
		}
	}	 
}  
//�b���w�ϰ줺��R���w�C���			 
//(sx,sy),(ex,ey):��R�x�ι﨤����,�ϰ�j�p��:(ex-sx+1)*(ey-sy+1)   
//color:�n��R���C��
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	uint16_t height,width;
	uint16_t i,j;
	width=ex-sx+1; 			//�o���R���e��
	height=ey-sy+1;			//����
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//�]�m���Ц�m 
		LCD_WriteRAM_Prepare();     //�}�l�g�JGRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*height+j];//�g�J�ƾ� 
	}	  
}  
//�e�u
//x1,y1:�_�I����
//x2,y2:���I����  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //�p�⧤�мW�q 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //�]�m��B��V 
	else if(delta_x==0)incx=0;//�����u 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//�����u 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //����򥻼W�q���жb 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//�e�u��X 
	{  
		LCD_DrawPoint(uRow,uCol);//�e�I 
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
//�e�x��	  
//(x1,y1),(x2,y2):�x�Ϊ��﨤����
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//�b���w��m�e�@�ӫ��w�j�p����
//(x,y):�����I
//r    :�b�|

void Draw_Circle_Fill(uint16_t x0,uint16_t y0,uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�P�_�U���I��m���лx
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
		//�ϥ�Bresenham��k�e��
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
	di=3-(r<<1);             //�P�_�U���I��m���лx
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
		//�ϥ�Bresenham��k�e��
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}

//�b���w��m��ܤ@�Ӧr��
//x,y:�_�l����
//num:�n��ܪ��r��:" "--->"~"
//size:�r��j�p 12/16
//mode:�|�[�覡(1)�٬O�D�|�[�覡(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
  uint8_t temp,t1,t;
	uint16_t y0=y;
	uint16_t colortemp=POINT_COLOR;      			     
	//�]�m���f		   
	num=num-' ';//�o�찾���᪺��
	if(!mode) //�D�|�[�覡
	{
	    for(t=0;t<size;t++)
	    {   
				if(size==12)	temp=asc2_1206[num][t];  //�ե�1206�r��
				else temp=asc2_1608[num][t];		 //�ե�1608�r��
					for(t1=0;t1<8;t1++)
					{			    
						if(temp&0x80)	POINT_COLOR=colortemp;
						else 					POINT_COLOR=BACK_COLOR;
						LCD_DrawPoint(x,y);
						temp<<=1;
						y++;
						if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//�W�ϰ�F
						if((y-y0)==size)
						{
							y=y0;
							x++;
							if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//�W�ϰ�F
							break;
						}
					}  	 
	    }    
	}else//�|�[�覡
	{
	    for(t=0;t<size;t++)
	    {   
				if(size==12)temp=asc2_1206[num][t];  //�ե�1206�r��
				else temp=asc2_1608[num][t];		 //�ե�1608�r�� 	                          
				for(t1=0;t1<8;t1++)
				{			    
					if(temp&0x80)LCD_DrawPoint(x,y); 
					temp<<=1;
					y++;
					if(x>=lcddev.height){POINT_COLOR=colortemp;return;}//�W�ϰ�F
					if((y-y0)==size)
					{
						y=y0;
						x++;
						if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//�W�ϰ�F
						break;
					}
				}  	 
	    }     
	}
	POINT_COLOR=colortemp;
}   
//m^n���
//��^��:m^n����.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//��ܼƦr,���쬰0,�h�����
//x,y :�_�I����	 
//len :�Ʀr�����
//size:�r��j�p
//color:�C�� 
//num:�ƭ�(0~4294967295);
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
//��ܼƦr,���쬰0,�٬O���
//x,y:�_�I����
//num:�ƭ�(0~999999999);	 
//len:����(�Y�n��ܪ����)
//size:�r��j�p
//mode:
//[7]:0,����R;1,��R0.
//[6:1]:�O�d
//[0]:0,�D�|�[���;1,�|�[���.
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
//��ܦr�Ŧ�
//x,y:�_�I����
//width,height:�ϰ�j�p  
//size:�r��j�p
//*p:�r�Ŧ�_�l�a�}		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p)
{         
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//�P�_�O���O�D�k�r��!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//�h�X
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}





