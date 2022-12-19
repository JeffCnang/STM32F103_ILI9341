#include "XPT2046.h"
#include "stm32f1xx_hal.h"

uint16_t TP_Read(void)
{
    uint8_t i = 16;
    uint16_t value = 0;

    while(i > 0x00) {
        value <<= 1;

				HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
				HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);
			
        if(HAL_GPIO_ReadPin(TP_MISO_PORT, TP_MISO_PIN) != 0) {
            value++;
        }

        i--;
    };

    return value;
}

void TP_Write(uint8_t value)
{
    uint8_t i = 0x08;

		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);	
	
    while( i > 0 ) {
        if((value & 0x80) != 0x00) {
						HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_SET);
        }
        else {
						HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_RESET);
        }

        value <<= 1;
				HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);
				HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_RESET);        
        i--;
    };
}

uint8_t TP_Read_Coordinates(uint16_t Coordinates[2])
{
    uint32_t avg_x, avg_y = 0;		
		uint16_t rawx, rawy = 0;	
		uint32_t calculating_x, calculating_y = 0;
	
    uint32_t samples = NO_OF_POSITION_SAMPLES;
    uint32_t counted_samples = 0;

		HAL_GPIO_WritePin(TP_CLK_PORT, TP_CLK_PIN, GPIO_PIN_SET);		
		HAL_GPIO_WritePin(TP_MOSI_PORT, TP_MOSI_PIN, GPIO_PIN_SET);		
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);		
		
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_RESET);
	
    while((samples > 0)&&(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0)) {			
        TP_Write(CMD_RDY);

				rawy = TP_Read();	
				avg_y += rawy;
				calculating_y += rawy;

				
        TP_Write(CMD_RDX);
        rawx = TP_Read();
				avg_x += rawx;
				calculating_x += rawx;
        samples--;
				counted_samples++;
    };
		
		HAL_GPIO_WritePin(TP_CS_PORT, TP_CS_PIN, GPIO_PIN_SET);

		
		if((counted_samples == NO_OF_POSITION_SAMPLES)&&(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0)) {
			calculating_x /= counted_samples;
			calculating_y /= counted_samples;
			
			rawx = calculating_x;
			rawy = calculating_y;		
			
			rawx *= -1;
			rawy *= -1;
			
			//CONVERTING 16bit Value to Screen coordinates
			// 65535/273 = 240!
			// 65535/204 = 320!
			Coordinates[0] = ((240 - (rawx/X_TRANSLATION)) - X_OFFSET)*X_MAGNITUDE;
			Coordinates[1] = ((rawy/Y_TRANSLATION)- Y_OFFSET)*Y_MAGNITUDE;
			
			return TOUCHPAD_DATA_OK;			
		}
		else {
			Coordinates[0] = 0;
			Coordinates[1] = 0;
			return TOUCHPAD_DATA_NOISY;
		}
}

uint8_t TP_Touchpad_Pressed(void)
{
	if(HAL_GPIO_ReadPin(TP_IRQ_PORT, TP_IRQ_PIN) == 0) {
		return TOUCHPAD_PRESSED;
	}
	
	return TOUCHPAD_NOT_PRESSED;
}


