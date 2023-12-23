#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#define LCD_Data_Dir DDRD
#define LCD_Command_Dir DDRB
#define LCD_Data_Port PORTD
#define LCD_Command_Port PORTB
#define RS PORTB1
#define RW PORTB2
#define EN PORTB3


void LCD_Command(unsigned char cmnd);
void LCD_Char (unsigned char char_data);
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();
void LCD_Shift_Right();
void LCD_Shift_Left();
void LCD_Set_Cursor(char a, char b);
void LCD_Custom_Char (unsigned char loc, unsigned char *msg);
int Random_Num_Generator(int nMin, int nMax, int seed);
void initSerialPort  (void);
void sendData (unsigned char character);
void sendString (char *str);
int enemy_Init();
int collision_detection(int agent_row, int enemy_row, int enemy_column);
int level_up();
void digits_of_reward(int reward, char* digits);

volatile int Jump_Flag = 0;
volatile int Random_Seed = 0;

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(2);
}

void LCD_Char (unsigned char char_data)  /* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(2);			/* Data write delay */
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;		/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;		/* Make LCD data port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);		/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);		/* Display ON Cursor OFF */
	LCD_Command (0x06);		/* Auto Increment cursor */
	LCD_Command (0x01);		/* clear display */
	_delay_ms(2);			/* Clear display command delay> 1.63 ms */
	LCD_Command (0x80);		/* Cursor at home position */
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)  /* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* clear display */
	LCD_Command (0x80);		/* cursor at home position */
}

void LCD_Shift_Right()
{
	LCD_Command(0x1C);
}

void LCD_Shift_Left()
{
	LCD_Command(0x18);
}

void LCD_Set_Cursor(char a, char b)
{
	if(a == 1)
	LCD_Command(0x80 + b);
	else if(a == 2)
	LCD_Command(0xC0 + b);
}

void LCD_Custom_Char (unsigned char loc, unsigned char *msg)
{
	unsigned char i;
	if(loc<8)
	{
		LCD_Command (0x40 + (loc*8));	/* Command 0x40 and onwards forces the device to point CGRAM address */
		for(i=0;i<8;i++)	/* Write 8 byte for generation of 1 character */
		LCD_Char(msg[i]);
	}
}

int Random_Num_Generator(int nMin, int nMax, int seed)
{
	srand(seed);
	int nRandonNumber = rand()%((nMax+1)-nMin) + nMin;
	return nRandonNumber;
}

ISR (PCINT0_vect) //ISR for pin change external interrupt 0
{
	Jump_Flag = 1;
}

void initSerialPort  (void)
{
	UCSR0B = 0x08;	//Set TxENO bit to 1
	UCSR0C = 0x06;	//Set UCZ01 bi UCZ00 bits to 1
	UBRR0L = 0x67;	//Set baud rate to 9600
}

void sendData (unsigned char character)
{
	while (!(UCSR0A & (1<<UDRE0)));	//Check UDR0 bit, Is it empty?
	{
		UDR0 = character;			//If UDR0 bit is empty transmit character
	}
}

void sendString (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		sendData (str[i]);
	}
}

int enemy_Init()
{
	Random_Seed += 3;
	Random_Seed = Random_Seed / 2; 
	int enemy_row = Random_Num_Generator(1, 2, Random_Seed);
	int enemy_column = 15;
	LCD_Set_Cursor(enemy_row, enemy_column);
	LCD_Char(1);
	return enemy_row;
}

int collision_detection(int agent_row, int object_row, int object_column)
{
	if(object_column == 1 && agent_row == object_row)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

int level_up()
{
	return 0;
}

void digits_of_reward(int reward, char* digits) // split score into digits(chars)
{
	int score = reward;
	int temp = 10 * reward; // for convenience of the loop
	int i = 0; // the digits of the integer
	while(score != 0)
	{
		score = score / 10;
		i++;
	}
	for(int m = i; m > 0; m--)
	{
		temp = temp / 10;
		digits[m-1] = temp % 10 + 48; // plus 48 to switch to ascii
	}
}


int main()
{
	cli();
	PCICR |=  0b00000001; // Enables Ports B Pin Change Interrupts
	PCMSK0 |= 0b00010000; // PCINT0
	EICRA = 0x02; //make INTO falling edge triggered
	sei();
	char agent_row = 1;
	int enemy_row = Random_Num_Generator(1, 2, Random_Seed);
	int enemy_column = 15;
	int playing = 0;
	int reward = 0;
	
	
	//char i;
	//
	unsigned char Character1[8] = {  0x0E, 0x0C, 0x06, 0x1F, 0x0E, 0x1B, 0x1B, 0x11 };  // custom character 1, agent
	unsigned char Character2[8] = {  0x0E, 0x1F, 0x15, 0x15, 0x1F, 0x0E, 0x0A, 0x0E  };  // custom character 2, enemy
	//unsigned char Character3[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00 };	// custom character 3, Apple, to enter the next level
	//unsigned char Character4[8] = { 0x01, 0x03, 0x07, 0x1F, 0x1F, 0x07, 0x03, 0x01 }; // custom character 4, when agent is invincible
	//unsigned char Character5[8] = { 0x01, 0x03, 0x05, 0x09, 0x09, 0x0B, 0x1B, 0x18 }; // custom character 5, when agent is in final level
	//unsigned char Character6[8] = { 0x0A, 0x0A, 0x1F, 0x11, 0x11, 0x0E, 0x04, 0x04 };
	//unsigned char Character7[8] = { 0x00, 0x00, 0x0A, 0x00, 0x04, 0x11, 0x0E, 0x00 };
	//unsigned char Character8[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };
	
	LCD_Init();
	
	LCD_Custom_Char(0, Character1);  /* Build Character1 at position 0 */
	LCD_Custom_Char(1, Character2);  /* Build Character2 at position 1 */
	//LCD_Custom_Char(2, Character3);  /* Build Character3 at position 2 */
	//LCD_Custom_Char(3, Character4);  /* Build Character4 at position 3 */
	//LCD_Custom_Char(4, Character5);  /* Build Character5 at position 4 */
	//LCD_Custom_Char(5, Character6);  /* Build Character6 at position 5 */
	//LCD_Custom_Char(6, Character7);  /* Build Character6 at position 6 */
	//LCD_Custom_Char(7, Character8);  /* Build Character6 at position 7 */

	//LCD_Set_Cursor(1, 0);		/*cursor at home position */
	//LCD_String("Custom char LCD");
	//LCD_Set_Cursor(2, 0);
	//
	//for(i=0;i<8;i++)		/* function will send data 1 to 8 to lcd */
	//{
		//LCD_Char(i);		/* char at 'i'th position will display on lcd */
		//LCD_Char(' ');		/* space between each custom char. */
	//}
	
	while(1)
	{
		if(collision_detection(agent_row%2+1, enemy_row, enemy_column))
		{
			playing = 0;
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Game Over");
			LCD_Set_Cursor(2, 0);
			LCD_String("Score:");
			LCD_Set_Cursor(2, 6);
			
			 // split score into digits(chars)
			char digits[10]={0};
			digits_of_reward(reward, digits);
			LCD_String(digits);
			
			_delay_ms(2000);
			initSerialPort();
			sendData(reward);
			return 0;
		}
		LCD_Clear();
		if(!playing)
		{
			LCD_Set_Cursor(1, 0);
			LCD_String("Press to");
			LCD_Set_Cursor(1, 9);
			LCD_Char(0);
			LCD_Char(1);
			LCD_Set_Cursor(2, 0);
			LCD_String("start the game");
			if(Jump_Flag == 1)
			{
				LCD_Clear();
				playing = 1;
				_delay_ms(100);
				Jump_Flag = 0;
			}
			_delay_ms(500);
		}
		
		
		if(playing)
		{
			// draw our agent
			if(Jump_Flag == 1)
			{
				agent_row++;
				_delay_ms(300);
				Jump_Flag = 0;
			}
			LCD_Set_Cursor(agent_row%2+1, 1);
			LCD_Char(0); // agent appearance will be changed if they catch the apple 

			// draw the enemy or any object; use _delay_ms() to control refresh rate
			LCD_Set_Cursor(enemy_row, enemy_column);
			LCD_Char(1);
			if(enemy_column != 0)
			{
				enemy_column -= 1;
				_delay_ms(300);
			}
			else
			{
				enemy_column = 15;
				enemy_row = enemy_Init();
			}
			
			reward++;
			_delay_ms(300);
		}
		
		//sendData(reward);
		//_delay_ms(500);
		//for(i=0; i<8; i++)
		//{
			//LCD_Shift_Left();
			//_delay_ms(500);
		//}
	}
}