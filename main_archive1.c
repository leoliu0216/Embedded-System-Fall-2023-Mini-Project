#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTB0
#define EN eS_PORTB1
#define RW eS_PORTB2

#define F_CPU 16000000UL
#include <avr/io.h>
#include "lcd.h"
#include <util/delay.h>
#include <string.h>


int main_archive(void)
{
	// DDRD = 0xFF;
	// DDRB = 0xFF;
	// char i;
	// unsigned char Character1[8] = { 0x00, 0x0A, 0x15, 0x11, 0x0A, 0x04, 0x00, 0x00 };  /* Custom char set for alphanumeric LCD Module */
	// unsigned char Character2[8] = { 0x04, 0x1F, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x1F };
	// unsigned char Character3[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00 };
	// unsigned char Character4[8] = { 0x01, 0x03, 0x07, 0x1F, 0x1F, 0x07, 0x03, 0x01 };
	// unsigned char Character5[8] = { 0x01, 0x03, 0x05, 0x09, 0x09, 0x0B, 0x1B, 0x18 };
	// unsigned char Character6[8] = { 0x0A, 0x0A, 0x1F, 0x11, 0x11, 0x0E, 0x04, 0x04 };
	// unsigned char Character7[8] = { 0x00, 0x00, 0x0A, 0x00, 0x04, 0x11, 0x0E, 0x00 };
	// unsigned char Character8[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };

	// Lcd4_Init();
	
	// LCD_Custom_Char(0, Character1);  /* Build Character1 at position 0 */
	// LCD_Custom_Char(1, Character2);  /* Build Character2 at position 1 */
	// LCD_Custom_Char(2, Character3);  /* Build Character3 at position 2 */
	// LCD_Custom_Char(3, Character4);  /* Build Character4 at position 3 */
	// LCD_Custom_Char(4, Character5);  /* Build Character5 at position 4 */
	// LCD_Custom_Char(5, Character6);  /* Build Character6 at position 5 */
	// LCD_Custom_Char(6, Character7);  /* Build Character6 at position 6 */
	// LCD_Custom_Char(7, Character8);  /* Build Character6 at position 7 */

	// Lcd4_Set_Cursor(1, 1);
	// Lcd4_Write_String("Custom char LCD");
	// Lcd4_Set_Cursor(2, 1);
	
	// while(1)
	// {
	// 	for(i=0;i<8;i++)		/* function will send data 1 to 8 to lcd */
	// 	{
	// 		Lcd4_Write_Char(i);		/* char at 'i'th position will display on lcd */
	// 		Lcd4_Write_Char(' ');		/* space between each custom char. */
	// 	}
	// }
	
	DDRD = 0xFF;
	DDRB = 0xFF;
	char* string = "Welcome to B31DD Embedded Systems!";
	int i;
	Lcd4_Init();
	Lcd4_Clear();
	while(1)
	{
		Lcd4_Set_Cursor(1,1);
		Lcd4_Write_String(string);
		for(i=0;i<strlen(string)-1;i++)
		{
			_delay_ms(500);
			Lcd4_Shift_Left();
		}
		for(i=0;i<strlen(string)-1;i++)
		{
			_delay_ms(500);
			Lcd4_Shift_Right();
		}
		Lcd4_Clear();
		// Lcd8_Write_Char('e');
		// Lcd8_Write_Char('S');
		_delay_ms(100);
	}
}
