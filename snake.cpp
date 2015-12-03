#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

/** standard U of A library settings, assuming Atmel Mega SPI pins */
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

/** specify pins */
#define JOYSTICK_VERT  	0   // Analog input A0 - vertical
#define JOYSTICK_HORIZ 	1   // Analog input A1 - horizontal

//initialize global variables
int buttonPin = 9;
int buttonValue;
int score;
int level;
int length;

typedef struct {
	int y;
	int x;
	int dir;
	/**direction:
	 * 	1 = UP
	 *  2 = DOWN
	 *  3 = LEFT
	 *  4 = RIGHT
	 */
} SnakeMove;

typedef struct {
	uint16_t y;
	uint16_t x;
} coordinates;

//helper function for changing levels
//rounds up to a multiple of 10
int round_to_10(int num) {
	int rem = num % 10;
	return num + 10 - (num%10);
}

SnakeMove head[1000];
SnakeMove fill;	

coordinates random_apple() {
	int i = 1;
	coordinates apple;
	apple.x = ((rand() %124) *4) %124;
	apple.y = ((rand() %156) *4) % 144;
	
	// to avoid the apple going to the walls and out of bounds
	if (apple.x <= 0 || apple.x >= 124) {
		apple.x = ((rand() %124) * 4) % 124;
	}
	
	if (apple.y <= 0 || apple.y >= 144) {
		apple.y = ((rand() %144) *4) % 144;
	} 
	while (i < length/4) {
		//to avoid placing an apple on top of the snake's body
		if (apple.x == head[i].x && apple.y == head[i].y) {
			apple.x = ((rand() %124) *4) %124;
			apple.y = ((rand() %156) *4) % 144;
		}
	}
	return apple;
}

void game_over();

void main_game() {
	level = 1;
	score = 0;
	int length = 8;
	int delayTime;
	int init_delay = 140;
	int m;
	
	head[0].x = 64;	// initial head position
	head[0].y = 80;
	head[0].dir = 1;
		
	int init_vert, init_horiz;
	init_vert = analogRead(JOYSTICK_VERT);
	init_horiz = analogRead(JOYSTICK_HORIZ);
	
	tft.initR(INITR_BLACKTAB);
	tft.fillRect(0, 0, 128, 160, 0x0000);
	
	//chooseing the position of the apple
	coordinates choose_apple = random_apple();
	
	while(true) {
		
		// footer : where score will be placed
		tft.fillRect(0, 147, 128, 13, 0xFFFF);
		tft.setCursor(3, 150);
		tft.setTextColor(0x0000);
		tft.print("Score: ");
		tft.print(score);

		tft.fillRect(choose_apple.x, choose_apple.y, 4, 4, 0x00FF);
		
		
		int vert, horiz, select;

		vert = analogRead(JOYSTICK_VERT);       // will be 0-1023
		horiz = analogRead(JOYSTICK_HORIZ);     // will be 0-1023

		int vertical, horizontal;
		
		//helps for the body not to move diagonally
		if (vert != init_vert) {
			vertical = vert;
			horizontal = init_horiz;
		}
		
		else if(horiz != init_horiz) {
			horizontal = horiz;
			vertical = init_vert;
		}
		
		int i;
		int j;
		int buffer_x[length/4];
		int buffer_y[length/4];
		
		/** going up **/
		if(head[0].dir == 1) {
			i = 0;
			while(i < (length/4)) {
				buffer_x[i] = head[i].x;
				buffer_y[i] = head[i].y;
				i++;
			}
			head[0].y = head[0].y - 4;
			tft.fillRect(head[0].x, head[0].y, 4, 4, 0xFFFF);
			
			j = 0;
			while(j<(length/4)-1) {
				head[j+1].x = buffer_x[j];
				head[j+1].y = buffer_y[j];
				tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
				j++;	
			}
			tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);
			
			//left
			if ((horizontal < init_horiz - 10) && (vertical == init_vert)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}

				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].x == head[m].x + 4) && (head[0].y == head[m].y)) {
						game_over();
					}
					m++;
				}
				
				head[0].dir = 3; //the direction will now go left
			}
		
		
			//right
			else if((horizontal > init_horiz + 10) && (vertical == init_vert)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}
				
				m = 1;
				while(m < length/4) {
					if ((head[0].x + 4 == head[m].x) && (head[0].y == head[m].y)) {
						game_over();
					}
					m++;
				}
			
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].y == head[m].y + 4) && (head[0].x == head[m].x)){
						game_over();
					}
					m++;
				}
				
				head[0].dir = 4; //the direction will now go to the right
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);
			}
			delay(20);
		}
		
		/** going down **/
		else if(head[0].dir == 2) {
			i = 0;
			while(i < (length/4)) {
				buffer_x[i] = head[i].x;
				buffer_y[i] = head[i].y;
				i++;
			}
			
			head[0].y = head[0].y + 4;
			tft.fillRect(head[0].x, head[0].y, 4, 4, 0xFFFF);
			
			j = 0;
			while(j<(length/4)-1) {
				head[j+1].x = buffer_x[j];
				head[j+1].y = buffer_y[j];
				tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
				j++;	
			}
			tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);			
			
			//left
			if((horizontal < init_horiz - 10) && (vertical == init_vert)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}	
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].x == head[m].x + 4) && (head[0].y == head[m].y)) {
						game_over();
					}
					m++;
				}
				
				head[0].dir = 3; //the direction will now go left
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);		
			}
		
		
			//right
			else if((horizontal > init_horiz + 10) && (vertical == init_vert)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].x + 4 == head[m].x) && (head[0].y == head[m].y)) {
						game_over();
					}
					m++;
				}
				
				head[0].dir = 4; //the direction will now go to the right
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);
				
			}
			
			//game over condition when snake hits its body
			m = 1;
			while(m < length/4) {
				if ((head[0].y + 4 == head[m].y) && (head[0].x == head[m].x)) {
					game_over();
				}
				m++;
			}
			delay(20);
		}
		
		/** going left **/
		else if(head[0].dir == 3) {
			i = 0;
			while (i < (length/4)) {
				buffer_x[i] = head[i].x;
				buffer_y[i] = head[i].y;
				i++;
			}
			
			// move head[0] to the left
			head[0].x = head[0].x - 4;
			tft.fillRect(head[0].x, head[0].y, 4, 4, 0xFFFF); 
			
			//the rest of the body follows
			j = 0;
			while(j<(length/4)-1) {
				head[j+1].x = buffer_x[j];
				head[j+1].y = buffer_y[j];
				tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
				j++;	
			}
			tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);
			
			//up
			if((vertical < init_vert - 10) && (horizontal == init_horiz)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].y == head[m].y + 4) && (head[0].x == head[m].x)){
					game_over();
					}
				m++;
				}
				
				head[0].dir = 1; //the direction will now go up
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);	
			}
			
			//down
			else if((vertical > init_vert + 10) && (horizontal == init_horiz)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].y + 4 == head[m].y) && (head[0].x == head[m].x)){
					game_over();
					}
				m++;
				}
				
				head[0].dir = 2; //the direction will now go down
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);	
			}
					//game over condition when snake hitsits body
			m = 1;
			while(m < length/4) {
				if ((head[0].x == head[m].x + 4) && (head[0].y == head[m].y)) {
					game_over();
				}
				m++;
			}
		delay(20);
		}
		
		/** going right **/
		else if(head[0].dir == 4) {
			i = 0;
			while (i < (length/4)) {
				buffer_x[i] = head[i].x;
				buffer_y[i] = head[i].y;
				i++;
			}
			
			// move head[0] to the right
			head[0].x = head[0].x + 4;
			tft.fillRect(head[0].x, head[0].y, 4, 4, 0xFFFF); 

			//the rest of the body follows
			j = 0;
			while(j<(length/4)-1) {
				head[j+1].x = buffer_x[j];
				head[j+1].y = buffer_y[j];
				tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
				j++;	
			}
			tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);
			
			//up
			if((vertical < init_vert - 10) && (horizontal == init_horiz)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}	
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].y == head[m].y + 4) && (head[0].x == head[m].x)){
					game_over();
					}
				m++;
				}
							
				head[0].dir = 1; //the direction will now go up
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);	
			}
			
			//down
			else if((vertical > init_vert + 10) && (horizontal == init_horiz)) {
				j = 0;
				while(j<(length/4)-1) {
					head[j+1].x = buffer_x[j];
					head[j+1].y = buffer_y[j];
					tft.fillRect(head[j+1].x, head[j+1].y, 4, 4, 0xFFFF);
					j++;	
				}
				
				//gameover condition: when snake hits body
				m = 1;
				while(m < length/4) {
					if ((head[0].y + 4 == head[m].y) && (head[0].x == head[m].x)){
					game_over();
					}
				m++;
				}
				
				head[0].dir = 2; //the direction will now go down
				tft.fillRect(buffer_x[(length/4)-1], buffer_y[(length/4)-1], 4, 4, 0x0000);	
			}
			
			//game over condition when snake hits its body
			m = 1;
			while(m < length/4) {
				if ((head[0].x + 4 == head[m].x) && (head[0].y == head[m].y)) {
					game_over();
				}
				m++;
			}
			
			delay(20);
		}
		
		/**EATING THE APPLE**/
		if (choose_apple.x == head[0].x && choose_apple.y == head[0].y) {
			length = length + 4;
			choose_apple = random_apple();
			score++;
		}
		
		/**GAME OVER CONDITION: when snake hits the wall**/
		if (head[0].x == 0 || head[0].x == 128) {
			game_over();
			break;
		}
		
		if (head[0].y == 0 || head[0].y == 144) {
			game_over();
			break;
		}
		
		/** LEVEL CHANGE: change level every 10 apples **/
		if(delayTime > 30) {
			//the delay time decreases by 20 every level up
			delayTime = init_delay - 2* round_to_10(score);
		}
		
		if(delayTime < 30) {
			//minumum delay time (for the max level)
			delayTime = 30;
		}
		
		delay(delayTime);
		/** print out the values */
		Serial.print("delay ");
		Serial.print(delayTime);
		Serial.print("score:");
		Serial.println(score);
	}
}

void welcome() {
	int select;
	
	pinMode(buttonPin, INPUT);
    digitalWrite(buttonPin, HIGH);
    
   	tft.initR(INITR_BLACKTAB);

    //SNAKE
	tft.fillRect(0, 0, 128, 160, 0x8811);
	tft.fillRect(10, 10, 108, 34, 0xFFFF);
	tft.setCursor(20, 16);
	tft.setTextColor (0x8811);
	tft.setTextSize(3);
	tft.println("SNAKE");
	
	//PRESS BUTTON TO CONTINUE
	tft.setCursor(29, 51);
	tft.setTextSize(1);
	tft.setTextColor(0xFFFF);
	tft.println("PRESS BUTTON");
	tft.setCursor(32, 62);
	tft.print("TO CONTINUE");
	
	//START
	tft.fillRect(46, 90, 40, 13, 0xFFFF);
	tft.setTextSize(1);
	tft.setTextColor(0x8811);
	tft.setCursor(52, 93);
	tft.print("START");
	
	while(true) {
		buttonValue = digitalRead(buttonPin);
		
		if (buttonValue == LOW) {
			main_game();
        }
	}
}

void game_over() {

	pinMode(buttonPin, INPUT);
    digitalWrite(buttonPin, HIGH);
   	
	delay(500);
	tft.fillRect(0, 0, 128, 160, 0x0000);
	tft.setCursor(10, 15);
	tft.setTextColor(0x07E0);
	tft.setTextSize(2);
	tft.println("GAME OVER");

	tft.setCursor(41, 60);	
	tft.setTextSize(1.8);
	tft.setTextColor(0xF800);
	tft.print("SCORE: ");
	tft.println(score);
	
	tft.fillRect(35, 85, 58, 13, 0xFFFF);
	tft.setCursor(38, 88);
	tft.setTextColor(0x0000);
	tft.setTextSize(1.5);
	tft.println("New Game?");
	
	tft.setCursor(20, 102);
	tft.setTextColor(0xF800);
	tft.setTextSize(1);
	tft.println("Press button to");
	tft.setCursor(40, 111);
	tft.setTextSize(1);
	tft.println("Continue");
	
	
	while(true) {
		buttonValue = digitalRead(buttonPin);
		
		if (buttonValue == LOW) {
			main_game();
        }
	}
}

int main() {
	init();
	Serial.begin(9600);
	
	welcome();
	
	return 0;
}
