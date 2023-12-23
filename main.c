#define F_CPU 16000000UL
#define LCD_Data_Dir		DDRD
#define LCD_Command_Dir		DDRB
#define LCD_Data_Port		PORTD
#define LCD_Command_Port	PORTB
#define RS			PORTB2
#define RW			PORTB3
#define EN			PORTB4
#define TRIGGER_PIN		PORTB5
#define ECHO_PIN		PORTB0
#define INTERRUPT_PIN		PORTC0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

volatile int Jump_Flag = 0;			// set global to use in: ISR()
volatile int Random_Seed = 182;		// set global to use in: enemy_init()
volatile int Game_Paused = 0;		// set global to use in: ISR()
volatile int Agent_Switch_Flag = 0;	// set global to use in: ISR()
volatile int reward = 0;			// set global to use in: enemy_init()

void LCD_Command(unsigned char cmnd);
void LCD_Char(unsigned char char_data);
void LCD_Init();
void LCD_String(char *str);
void LCD_String_xy(char row, char pos, char *str);
void LCD_Clear();
void LCD_Shift_Right();
void LCD_Shift_Left();
void LCD_Set_Cursor(char a, char b);
void LCD_Custom_Char(unsigned char loc, unsigned char *msg);
int Random_Num_Generator(int nMin, int nMax, int seed);
void initSerialPort();
void sendData(unsigned char character);
void sendString(char *str);
int enemy_Init(int which_enemy);
int collision_detection(int agent_row, int agent_column, int object_row, int object_column);
void digits_of_reward(int reward, char* digits);
void trigger_pulse();
uint16_t measure_distance();
void servo_spin();

void trigger_pulse()
{
	PORTB |= (1 << TRIGGER_PIN);
	_delay_us(10);
	PORTB &= ~(1 << TRIGGER_PIN);
}


uint16_t measure_distance()
{
	uint32_t count = 0;
	uint16_t distance = 0;
	trigger_pulse();
	while (!(PINB & (1 << ECHO_PIN))){
		_delay_us(10);
		PORTB |= (1 << TRIGGER_PIN);
		_delay_us(10);
		PORTB &= ~(1 << TRIGGER_PIN);
	}
	
	while (PINB & (1 << ECHO_PIN)) {
		count++;
		_delay_us(1);
	}
	
	distance = (uint16_t)(count * 0.034 / 2);
	return distance;
}


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


void LCD_Init ()			/* LCD Initialize function */
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


ISR (PCINT1_vect) //ISR for pin change external interrupt 1
{
	Jump_Flag = 1;
	if(Game_Paused)
	{
		Agent_Switch_Flag = 1;
	}
}


void initSerialPort()
{
	UCSR0B = 0x08;	//Set TxENO bit to 1
	UCSR0C = 0x06;	//Set UCZ01 bi UCZ00 bits to 1
	UBRR0L = 0x67;	//Set baud rate to 9600
}


void sendData (unsigned char character)
{
	while (!(UCSR0A & (1<<UDRE0)));
	{
		UDR0 = character;			//If UDR0 bit is empty transmit character
	}
}


void sendString (char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		sendData (str[i]);
	}
}


int enemy_Init(int which_enemy)
{
	int enemy_row = Random_Num_Generator(1, 2, reward);
	int enemy_column = 15;
	LCD_Set_Cursor(enemy_row, enemy_column);
	LCD_Char(which_enemy);
	return enemy_row;
}


int collision_detection(int agent_row, int agent_column, int object_row, int object_column)
{
	if(agent_row == object_row && agent_column == object_column)
	{
		return 1;
	}
	else
	{
		return 0;
	}

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


void servo_spin()
{
	DDRB |= 1 << PINB1; // Set pin 9 on arduino to output
	
	TCCR1A |= (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);
	
	ICR1 = 24999;
	for(int i = 0; i < 3; i++)
	{
		OCR1A = 4899;
		
		_delay_ms(1000);
		
		OCR1A = 1199;
		
		_delay_ms(1000);
	}
}


int main()
{
	// initialize interrupt pin
	DDRC |= (1 << INTERRUPT_PIN);
	PORTC &= ~(1 << INTERRUPT_PIN);
	
	cli();
	PCICR |=  0b00000010;	// Enables Ports C Pin Change Interrupts
	PCMSK1 |= 0b00000001;	// PCINT0(PC0)
	EICRA = 0x02;			//make INTO falling edge triggered
	sei();
	
	// initialize variables
	char agent_row = 1;
	int agent_column = 0;
	int bullet_row = -1;
	int bullet_column = -1;
	int enemy_row = Random_Num_Generator(1, 2, Random_Seed);
	int enemy_column = 15;
	int playing = 0;
	int which_agent = 0;
	int invincibility = 0;
	int have_bullet = 0;
	int game_level = 0;
	int level_up_flag = 0;
	int round_counter = 0;
	int bonus_round = 0;
	int enemy_hp = 1;
	int final_round = 0;
	int enemy_bullet_row = -1;
	int enemy_bullet_column = -1;
	
	// define custom chars
	unsigned char Character1[8] = {  0x0E, 0x0C, 0x06, 0x1F, 0x0E, 0x1B, 0x1B, 0x11  };  // custom character 1, agent
	unsigned char Character2[8] = {  0x0E, 0x1F, 0x15, 0x15, 0x1F, 0x0E, 0x0A, 0x0E  };  // custom character 2, enemy
	unsigned char Character3[8] = {  0x18, 0x08, 0x1C, 0x0B, 0x1C, 0x08, 0x18, 0x00  };	 // custom character 3, second appearance of agent
	unsigned char Character4[8] = {  0x04, 0x0A, 0x15, 0x0E, 0x0E, 0x15, 0x0A, 0x04  };  // custom character 4, third appearance of agent, toggle invincibility
	unsigned char Character5[8] = {  0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00  };  // custom character 5, heart, to add 1k reward
	unsigned char Character6[8] = {  0x0E, 0x1B, 0x1F, 0x03, 0x0B, 0x07, 0x13, 0x0E  };	 // custom character 6, second appearance of enemy
	unsigned char Character7[8] = {  0x0E, 0x0E, 0x04, 0x1E, 0x06, 0x0F, 0x09, 0x1B  };  // custom character 7, third appearance of enemy
	unsigned char Character8[8] = {  0x00, 0x1C, 0x12, 0x11, 0x12, 0x1C, 0x00, 0x00  };  // custom character 8, the bullet
	
	LCD_Init();
	
	LCD_Custom_Char(0, Character1);
	LCD_Custom_Char(1, Character2);
	LCD_Custom_Char(2, Character3);
	LCD_Custom_Char(3, Character4);
	LCD_Custom_Char(4, Character5);
	LCD_Custom_Char(5, Character6);
	LCD_Custom_Char(6, Character7);
	LCD_Custom_Char(7, Character8); 


	DDRB |= (1 << TRIGGER_PIN);
	DDRB &= ~(1 << ECHO_PIN);
	
	while(1)
	{
		while(measure_distance() < 5)
		{
			Game_Paused = 1;
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("GAME PAUSED");
			LCD_Set_Cursor(2, 0);
			LCD_String("try: ");
			LCD_Set_Cursor(2, 6);
			LCD_Char(2);
			LCD_Set_Cursor(2, 7);
			LCD_Char(' ');
			LCD_Set_Cursor(2, 8);
			LCD_Char(3);
			if(Agent_Switch_Flag)
			{
				LCD_Clear();
				LCD_Set_Cursor(1, 0);
				LCD_String("Feature Found.");
				LCD_Set_Cursor(2, 0);
				LCD_String("Agent Changed");
				if(which_agent == 0)
				{
					LCD_Custom_Char(0, Character3); // change to the second agent
					which_agent = 1;
					invincibility = 0;
					have_bullet = 1;
				}
				else if(which_agent == 1)
				{
					LCD_Custom_Char(0, Character4); // change to the third agent,  the third agent is invincible
					which_agent = 2;
					invincibility = 1;
					have_bullet = 0;
				}
				else
				{
					LCD_Custom_Char(0, Character1); // change to the first agent
					which_agent = 0;
					invincibility = 0;
					have_bullet = 0;
				}
				
				Agent_Switch_Flag = 0;
				_delay_ms(1000);
				break;
			}
			_delay_ms(1000);
		}
		Game_Paused = 0;
		
		
		if((collision_detection(agent_row%2+1, agent_column, enemy_row, enemy_column)) && (invincibility == 0) && (bonus_round == 0))
		{
			playing = 0;
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Game Over");
			LCD_Set_Cursor(1, 10);
			LCD_Char(1);
			LCD_Set_Cursor(2, 0);
			LCD_String("Score:");
			LCD_Set_Cursor(2, 6);
			// split score into digits(chars)
			char digits[10]={0};
			digits_of_reward(reward, digits);
			LCD_String(digits);
			_delay_ms(2500);
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Sending Game");
			LCD_Set_Cursor(2, 0);
			LCD_String("Data ...");
			servo_spin();
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Successful.");
			LCD_Set_Cursor(2, 0);
			LCD_String("see 'score.txt'");
			initSerialPort();
			sendData(reward);
			return 0;
		}
		if((collision_detection(agent_row%2+1, agent_column, enemy_bullet_row, enemy_bullet_column)) && (invincibility == 0) && (bonus_round == 0))
		{
			playing = 0;
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Game Over");
			LCD_Set_Cursor(1, 10);
			LCD_Char(1);
			LCD_Set_Cursor(2, 0);
			LCD_String("Score:");
			LCD_Set_Cursor(2, 6);
			// split score into digits(chars)
			char digits[10]={0};
			digits_of_reward(reward, digits);
			LCD_String(digits);
			_delay_ms(2500);
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Sending Game");
			LCD_Set_Cursor(2, 0);
			LCD_String("Data ...");
			servo_spin();
			LCD_Clear();
			LCD_Set_Cursor(1, 0);
			LCD_String("Successful.");
			LCD_Set_Cursor(2, 0);
			LCD_String("see 'score.txt'");
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
				playing = 1;
				Jump_Flag = 0;
				LCD_Clear();
				LCD_Set_Cursor(1, 0);
				LCD_String("Level 1, Start");
				_delay_ms(1500);
				LCD_Clear();
			}
			_delay_ms(500);
		}
		
		
		if(playing)
		{
			LCD_Set_Cursor(1, 0);
			char digits[10]={0};
			digits_of_reward(reward, digits);
			LCD_String(digits);
			if(level_up_flag == 1 && game_level < 2)
			{
				game_level += 1;
				level_up_flag = 0;
				LCD_Clear();
				LCD_Set_Cursor(1, 0);
				LCD_String("Level up!");
				_delay_ms(2000);
				if(game_level == 1)
				{
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Level 2, Start");
					_delay_ms(1500);
					LCD_Clear();
				}
				if(game_level == 2)
				{
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Level 3, Start");
					_delay_ms(1500);
					LCD_Clear();
				}
			}
			
			
			
			if(bonus_round == 1)
			{
				agent_column = 4;
				// draw our agent
				if(Jump_Flag == 1)
				{
					agent_row++;
					_delay_ms(300);
					Jump_Flag = 0;
				}
				LCD_Set_Cursor(agent_row%2+1, agent_column);
				LCD_Char(0);

				// draw the enemy or any object; use _delay_ms() to control refresh rate
				LCD_Set_Cursor(enemy_row, enemy_column);
				LCD_Char(4);
				if(enemy_column != 0)
				{
					enemy_column -= 1;
					_delay_ms(300);
				}
				else
				{
					bonus_round = 0;
				}
				if(collision_detection(agent_row%2+1, agent_column, enemy_row, enemy_column))
				{
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Score +100!");
					reward += 100;
					bonus_round = 0;
					enemy_column = 15;
					enemy_row = enemy_Init(1);
					_delay_ms(1000);
				}
				_delay_ms(300);
				
			}
			else if(final_round == -1 || final_round == 1)
			{
				if(final_round == -1)
				{
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Final Round,");
					LCD_Set_Cursor(2, 0);
					LCD_String("Start");
					_delay_ms(1500);
					LCD_Clear();
				}
				final_round = 1;
				agent_column = 4;
				// draw our agent
				if(Jump_Flag == 1)
				{
					agent_row++;
					_delay_ms(200);
					Jump_Flag = 0;
				}
				LCD_Set_Cursor(agent_row%2+1, agent_column);
				LCD_Char(0);

				// draw the enemy or any object; use _delay_ms() to control refresh rate
				LCD_Set_Cursor(enemy_row, enemy_column);
				LCD_Char(4);
				if(enemy_column != 0)
				{
					enemy_row = enemy_row % 2 + 1;
					enemy_column -= 1;
					_delay_ms(150);
				}
				else
				{
					final_round = 0;
					game_level = 1;
					level_up_flag =1;
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Missed it");
					LCD_Set_Cursor(2, 0);
					LCD_String("Restart L.3");
					_delay_ms(1000);
					continue;
				}
				if(collision_detection(agent_row%2+1, agent_column, enemy_row, enemy_column))
				{
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("You Won!");
					_delay_ms(2000);
					reward += 1000;
					final_round = 0;
					playing = 0;
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Game Over");
					LCD_Set_Cursor(1, 10);
					LCD_Char(4);
					LCD_Set_Cursor(2, 0);
					LCD_String("Score:");
					LCD_Set_Cursor(2, 6);
					// split score into digits(chars)
					char digits[10]={0};
					digits_of_reward(reward, digits);
					LCD_String(digits);
					_delay_ms(2500);
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Sending Game");
					LCD_Set_Cursor(2, 0);
					LCD_String("Data ...");
					servo_spin();
					LCD_Clear();
					LCD_Set_Cursor(1, 0);
					LCD_String("Successful.");
					LCD_Set_Cursor(2, 0);
					LCD_String("see 'score.txt'");
					initSerialPort();
					sendData(reward);
					return 0;
				}
				_delay_ms(300);
			}
			else if(game_level == 0)
			{
				
				agent_column = 4;
				// draw our agent
				if(Jump_Flag == 1)
				{
					agent_row++;
					_delay_ms(300);
					Jump_Flag = 0;
				}
				LCD_Set_Cursor(agent_row%2+1, agent_column);
				LCD_Char(0);
				
				
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
					round_counter += 1;
					enemy_column = 15;
					enemy_row = enemy_Init(1);
				}
				
				if(have_bullet == 1)
				{
					if(bullet_row == -1)
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					LCD_Set_Cursor(bullet_row, bullet_column);
					LCD_Char(7);
					if(bullet_column != 15)
					{
						bullet_column += 1;
					}
					else
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					if((collision_detection(bullet_row, bullet_column, enemy_row, enemy_column-1)) || (collision_detection(bullet_row, bullet_column, enemy_row, enemy_column)))
					{
						enemy_hp -= 1;
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
						if(enemy_hp == 0)
						{
							enemy_column = 15;
							enemy_row = enemy_Init(1);
							round_counter += 1;
							enemy_hp = 1;
							reward += 7;
							LCD_Clear();
							LCD_Set_Cursor(1, 0);
							LCD_String("score +7");
							LCD_Set_Cursor(2, 0);
							LCD_String("Score:");
							LCD_Set_Cursor(2, 6);
							// split score into digits(chars)
							char digits[10]={0};
							digits_of_reward(reward, digits);
							LCD_String(digits);
							_delay_ms(300);
						}
					}
				}
				
				
				if(round_counter >= 4) // do a bonus round, then level up
				{
					bonus_round = 1;
					level_up_flag = 1;
					round_counter = 0;
				}
				reward++;
				
				_delay_ms(300);
			}
			
			else if(game_level == 1)
			{
				agent_column = 6;
				// draw our agent
				if(Jump_Flag == 1)
				{
					agent_row++;
					_delay_ms(300);
					Jump_Flag = 0;
				}
				LCD_Set_Cursor(agent_row%2+1, agent_column);
				LCD_Char(0);

				// draw the enemy or any object; use _delay_ms() to control refresh rate
				LCD_Set_Cursor(enemy_row, enemy_column);
				LCD_Char(5);
				if(enemy_column != 0)
				{
					enemy_column -= 1;
					_delay_ms(100);
				}
				else
				{
					round_counter += 1;
					enemy_column = 15;
					enemy_row = enemy_Init(5);
				}
				
				if(have_bullet == 1)
				{
					if(bullet_row == -1)
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					LCD_Set_Cursor(bullet_row, bullet_column);
					LCD_Char(7);
					if(bullet_column != 15)
					{
						bullet_column += 1;
					}
					else
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					if((collision_detection(bullet_row, bullet_column, enemy_row, enemy_column-1)) || (collision_detection(bullet_row, bullet_column, enemy_row, enemy_column)))
					{
						enemy_hp -= 1;
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
						if(enemy_hp == 0)
						{
							enemy_column = 15;
							enemy_row = enemy_Init(1);
							round_counter += 1;
							enemy_hp = 1;
							reward += 15;
							LCD_Clear();
							LCD_Set_Cursor(1, 0);
							LCD_String("score +15");
							LCD_Set_Cursor(2, 0);
							LCD_String("Score:");
							LCD_Set_Cursor(2, 6);
							// split score into digits(chars)
							char digits[10]={0};
							digits_of_reward(reward, digits);
							LCD_String(digits);
							_delay_ms(300);
						}
					}
				}
				if(round_counter >= 8) // do a bonus round, then level up
				{
					bonus_round = 1;
					level_up_flag = 1;
					round_counter = 0;
				}
				reward++;
				_delay_ms(100);
			}
			else if(game_level == 2)
			{
				agent_column = 7;
				// draw our agent
				if(Jump_Flag == 1)
				{
					agent_row++;
					_delay_ms(100);
					Jump_Flag = 0;
				}
				LCD_Set_Cursor(agent_row%2+1, agent_column);
				LCD_Char(0);

				// draw the enemy or any object; use _delay_ms() to control refresh rate
				LCD_Set_Cursor(enemy_row, enemy_column);
				LCD_Char(6);
				if(enemy_column != 0)
				{
					enemy_column -= 1;
					_delay_ms(100);
				}
				else
				{
					round_counter += 1;
					enemy_column = 15;
					enemy_row = enemy_Init(6);
				}
				
				if(have_bullet == 1)
				{
					if(bullet_row == -1)
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					LCD_Set_Cursor(bullet_row, bullet_column);
					LCD_Char(7);
					if(bullet_column != 15)
					{
						bullet_column += 1;
					}
					else
					{
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
					}
					if((collision_detection(bullet_row, bullet_column, enemy_row, enemy_column-1)) || (collision_detection(bullet_row, bullet_column, enemy_row, enemy_column)))
					{
						enemy_hp -= 1;
						bullet_column = agent_column+1;
						bullet_row = agent_row%2+1;
						if(enemy_hp == 0)
						{
							enemy_column = 15;
							enemy_row = enemy_Init(1);
							round_counter += 1;
							enemy_hp = 1;
							reward += 21;
							LCD_Clear();
							LCD_Set_Cursor(1, 0);
							LCD_String("score +21");
							LCD_Set_Cursor(2, 0);
							LCD_String("Score:");
							LCD_Set_Cursor(2, 6);
							// split score into digits(chars)
							char digits[10]={0};
							digits_of_reward(reward, digits);
							LCD_String(digits);
							_delay_ms(300);
						}
					}
				}
				
				if(enemy_bullet_row == -1)
				{
					enemy_bullet_column = enemy_column-1;
					enemy_bullet_row = enemy_row;
				}
				LCD_Set_Cursor(enemy_bullet_row, enemy_bullet_column);
				LCD_Char(1);
				if(enemy_bullet_column >= 0)
				{
					enemy_bullet_column -= 2;
				}
				else
				{
					enemy_bullet_column = enemy_column-1;
					enemy_bullet_row = enemy_row;
				}
				
				if(round_counter >= 10) // do the final round
				{
					final_round = -1;
					round_counter = 0;
				}
				reward++;
				_delay_ms(150);
			}
			else
			{
				return 0;
			}
		}
	}
}
