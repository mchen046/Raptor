#include "includes/bit.h"
#include "includes/io.h"
#include "includes/io.c"
#include "includes/Timer.h"
#include "includes/functions.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT PORTB
#define dataPIN 5
#define latchPIN 0
#define clkPIN 7
	
#define dataLow()  PORT&=~_BV(dataPIN)
#define dataHigh() PORT|=_BV(dataPIN)
#define latchLow()  PORT&=~_BV(latchPIN)
#define latchHigh() PORT|=_BV(latchPIN)
#define clkLow()  PORT&=~_BV(clkPIN)
#define clkHigh() PORT|=_BV(clkPIN)

typedef unsigned char bool;
#define true 1
#define false 0

struct pos{
	unsigned char row, col;
};

bool rN, rE, rS, rWEST, disp, play, diffSet, nextBullet, eNextBullet;
bool seeded = false, flushEnemyCond = false, welcDisplayed = false, playDisplayed = false, pChanged = false, eChangedA = false, eChangedB = false, moveCond = false;;

unsigned char shipCol, eColA, eColB, pBulletPos, eBulletPosA, eBulletPosB, eCnt, difficulty = 0, flushCol, flushCol2, flushRow, flushRow2;
unsigned short menuCnt, score, playerBulletCnt, enemyBulletCnt, enemyPeriod = 1000, flushCnt = 0;
unsigned int seedCnt = 0;

struct pos ship[2], enemyShip[2][2];

const unsigned char* msgList[5] = {"Welcome to...   Raptor!", " - Embedded Systems Edition!", "Please select an option:", "0: Play         1: Difficulty", \
"2: Easter egg"};

const unsigned char* diffMsg[2] = {"0: Easy         1: Medium", "2: Hard         3: God Tier"};

//functions
void atEdge(){ //check if ship is at screen edge
	rN = rE = rS = rWEST = false;
	for(unsigned char i = 0; i<2; i++){
		if(ship[i].row==0){
			rN = true;
		}
		if(ship[i].row==7){
			rS = true;
		}
		if(ship[i].col==0){
			rWEST = true;
		}
		if(ship[i].col==7){
			rE = true;
		}
	}
}	

void bulletPositions(){
	if(!pChanged && nextBullet){
		pBulletPos--;
		pChanged = true;
	}
	if(eNextBullet){
		if(!eChangedA){
			if(eBulletPosA!=7){
				eBulletPosA++;
				eChangedA = true;
			}
			else if(eBulletPosA==7){
				eBulletPosA = enemyShip[0][1].row+1;
				eColA = enemyShip[0][1].col;
			}
		}
		
		if(!eChangedB){
			if(eBulletPosB!=7){
				eBulletPosB++;
				eChangedB = true;
			}
			else if(eBulletPosB==7){
				eBulletPosB = enemyShip[1][1].row+1;
				eColB = enemyShip[1][1].col;
			}
		}
	}
}

void columnBullets(unsigned char num){ //set Green to HIGH
	
	//setting Red
	clkLow();
	latchLow();
	unsigned char i;
	bool leave = false;
	for(i = 8; i>num+1 && !leave; i--){
		dataHigh();
		clkHigh();
		clkLow();
		if(i==0){
			leave = true;
			i++;
		}
	}
	dataLow();
	clkHigh();
	clkLow();
	i--;
	for(unsigned char j = i; j>=1; j--){
		dataHigh();
		clkHigh();
		clkLow();
	}
	
	//set all Green to HIGH
	clkLow(); //set all the red to HIGH
	latchLow();
	for(unsigned char i = 0; i<8; i++){
		dataHigh();
		clkHigh();
		clkLow();
	}
}

void columnGreen(unsigned char num){
	
	clkLow(); //set all the red to HIGH
	latchLow();
	for(unsigned char i = 0; i<8; i++){
		dataHigh();
		clkHigh();
		clkLow();
	}
	
	clkLow();
	latchLow();
	unsigned char i;
	bool leave = false;
	for(i = 8; i>num+1 && !leave; i--){
		dataHigh();
		clkHigh();
		clkLow();
		if(i==0){
			leave = true;
			i++;
		}
	}
	dataLow();
	clkHigh();
	clkLow();
	i--;
	for(unsigned char j = i; j>=1; j--){
		dataHigh();
		clkHigh();
		clkLow();
	}
}

void columnRed(unsigned char num){
	clkLow();
	latchLow();
	unsigned char i;
	bool leave = false;
	for(i = 8; i>num+1 && !leave; i--){
		dataHigh();
		clkHigh();
		clkLow();
		if(i==0){
			leave = true;
			i++;
		}
	}
	dataLow();
	clkHigh();
	clkLow();
	i--;
	for(unsigned char j = i; j>=1; j--){
		dataHigh();
		clkHigh();
		clkLow();
	}
	
	clkLow(); //set all the Green to HIGH
	latchLow();
	for(unsigned char i = 0; i<8; i++){
		dataHigh();
		clkHigh();
		clkLow();
	}
}

void disableMatrix(){
	//set cathodes to HIGH
	for(unsigned char i = 0; i<2; i++){
		clkLow();
		latchLow();
		for(unsigned char i = 0; i<8; i++){
			dataHigh();
			clkHigh();
			clkLow();
		}
	}
	
	//disable all rows
	clkLow();
	latchLow();
	for(unsigned char i = 0; i<8; i++){
		dataLow();
		clkHigh();
		clkLow();
	}
	latchHigh();
}

void dispArt(){ //displaying by row
	unsigned char welcomeArt[8] = {0, 112, 56, 28, 28, 56, 112, 0};
	for(unsigned char i = 0; i<8; i++){
		columnRed(i);
		output(welcomeArt[i]);
		//_delay_ms(100);
	}
	
	unsigned char welcomeArt2[8] = {0, 0, 64, 32, 32, 64, 0, 0};
	
	for(unsigned int i = 0; i<8; i++){
		columnGreen(i);
		output(welcomeArt2[i]);
		//_delay_ms(100);
	}
}

void dispMsg(const unsigned char* message){
	char a[32];
	for(unsigned char i = 0; i<strlen(message) + 32; i++){
		if(i<32){
			a[i]=message[i];
			LCD_DisplayString(32-i, a);
		}
		else if(i>=32){
			
			//delay_ms(10000);
			for(unsigned char j = 0; j<strlen(message)-32; j++){
				if(j<31){
					a[j] = a[j+1];
				}
				else if(j==31 && j<strlen(message)){
					//delay_ms(10000);
					a[j] = message[i];
				}
				else if(j==31 && j<strlen(message)){
					a[31] = 0;
				}
			}
			LCD_DisplayString(1, a);
			
		}
		//delay_ms(1000);
	}
}

void dispScore(const unsigned char* text){
	unsigned short tracker = 100;
	unsigned short scoreTemp = score;
	if(!playDisplayed){
		LCD_ClearScreen();
		LCD_DisplayString(1, text);
		playDisplayed = true;
	}

	LCD_Cursor(strlen(text) + 1);
	for(unsigned char i = 0; tracker!=1; ){
		if(scoreTemp>=tracker){
			scoreTemp -= tracker;
			i++;
		}
		else{
			if(i!=0)
			LCD_WriteData(i + '0');
			i = 0;
			tracker/=10;
			if(scoreTemp<10 && tracker==1){
				LCD_WriteData(scoreTemp + '0');
			}
		}
	}
	LCD_Cursor(0);
}

void dispShip(){ //display all ships
	//player ship
	for(unsigned char j = 0; j<2; j++){
		columnGreen(ship[j].col);
		output(_BV(ship[j].row));
	}
	
	//enemy ships
	for(unsigned char j = 0; j<2; j++){
		for(unsigned char k = 0; k<2; k++){
			columnGreen(enemyShip[j][k].col);
			output(_BV(enemyShip[j][k].row));
		}
	}
}

void enemyMove(){
	//checks for border with screen and other enemy ships
	bool eN = false, eE = false, eS = false, eW = false;
	for(unsigned char i = 0; i<2; i++){
		for(unsigned char j = 0; j<2; j++){
			if(enemyShip[i][j].row==0){
				eN = true;
			}
			if(enemyShip[i][j].row==4){
				eS = true;
			}
			if(enemyShip[i][j].col==0){
				eW = true;
			}
			if(enemyShip[i][j].col==7){
				eE = true;
			}
		}
	}
		
	//set random direction
	int randNum = rand();
	unsigned char dir = randNum % 4;
	
	randNum = rand();
	unsigned char whichShip = randNum % 2;
	
	//check if enemy ships are touching each other //NOT WORKING
	//start with whichShip!!
	//just modify eN, eS, etc variables
	
	if(whichShip==0){
		for(unsigned char i = 0; i<2; i++){
			if(enemyShip[whichShip][i].row-1==enemyShip[1][0].row || enemyShip[whichShip][i].row-1==enemyShip[1][1].row){ //checking north
				eN = true;
			}
			if(enemyShip[whichShip][i].col+1==enemyShip[1][0].col || enemyShip[whichShip][i].col+1==enemyShip[1][1].col){ //checking east
				eE = true;
			}
			if(enemyShip[whichShip][i].row+1==enemyShip[1][0].row || enemyShip[whichShip][i].row+1==enemyShip[1][1].row){ //checking south
				eS = true;
			}
			if(enemyShip[whichShip][i].col-1==enemyShip[1][0].col || enemyShip[whichShip][i].col-1==enemyShip[1][1].col){ //checking west
				eW = true;
			}
		}
	}
	else if(whichShip==1){
		for(unsigned char i = 0; i<2; i++){
			if(enemyShip[whichShip][i].row-1==enemyShip[0][0].row || enemyShip[whichShip][i].row-1==enemyShip[0][1].row){ //checking north
				eN = true;
			}
			if(enemyShip[whichShip][i].col+1==enemyShip[0][0].col || enemyShip[whichShip][i].col+1==enemyShip[0][1].col){ //checking east
				eE = true;
			}
			if(enemyShip[whichShip][i].row+1==enemyShip[0][0].row || enemyShip[whichShip][i].row+1==enemyShip[0][1].row){ //checking south
				eS = true;
			}
			if(enemyShip[whichShip][i].col-1==enemyShip[0][0].col || enemyShip[whichShip][i].col-1==enemyShip[0][1].col){ //checking west
				eW = true;
			}
		}
	}
	
	//set period here
	if(eCnt==15){
		for(unsigned char i = 0; i<2; i++){
			if(!eN && dir==0){ //N
				enemyShip[whichShip][i].row-=1;
			}
			else if(!eE && dir==1){ //E
				enemyShip[whichShip][i].col+=1;
			}
			else if(!eS && dir==2){ //S
				enemyShip[whichShip][i].row+=1;
			}
			else if(!eW && dir==3){ //W
				enemyShip[whichShip][i].col-=1;
			}
		}
		eCnt = 0;
	}
	else if(eCnt<15){
		eCnt++;
	}		
}

void flushEnemy(bool flushEnemyCond){
	if(flushEnemyCond){
		columnRed(flushCol);
		output(_BV(flushRow));
		
		columnRed(flushCol2);
		output(_BV(flushRow2));
	}
}

void flushScreen(){
	for(unsigned char i = 0; i<8; i++){
		
		//set all Red to LOW
		clkLow(); 
		latchLow();
		for(unsigned char i = 0; i<8; i++){
			dataLow();
			clkHigh();
			clkLow();
		}
		
		//set all Green to HIGH
		clkLow();
		latchLow();
		for(unsigned char i = 0; i<8; i++){
			dataHigh();
			clkHigh();
			clkLow();
		}
		
		output(_BV(i));
		_delay_ms(399);
		if(i==0 || i==7){
			_delay_ms(100);
		}
	}
	disableMatrix();
}

void hitEnemy(unsigned char shipCol, unsigned char pBulletPos){
	//check if bullet hit enemy ship
	for(unsigned char i = 0; i<2; i++){
		if(enemyShip[i][1].col==shipCol && enemyShip[i][1].row==pBulletPos){
			//update score
			score++;
			
			//explosion animation and respawn enemy ship
			flushEnemyCond = true;
			
			flushCol = enemyShip[i][0].col;
			flushRow = enemyShip[i][0].row;
			
			flushCol2 = enemyShip[i][1].col;
			flushRow2 = enemyShip[i][1].row;
			
			//look for free position based on random col and random row
			unsigned char randCol, randRow;
			do{
				int randTemp = rand();
				randCol = randTemp % 8;
				
				randTemp = rand();
				randRow = randTemp % 3;
				
			}while(!isEmptyPos(randCol, randRow));
			
			for(unsigned char j = 0; j<2; j++){
				enemyShip[i][j].col=randCol;
				enemyShip[i][j].row=randRow;
				randRow++;
			}
		}
	}
}

void hitPlayer(){
	//check for collision with bullets
	for(unsigned char i = 0; i<2; i++){
		if((eColA == ship[0].col && eBulletPosA == ship[i].row) || (eColB == ship[0].col && eBulletPosB == ship[i].row)){
			//display two red dots at player ship pos
			columnRed(ship[i].col);
			if(i==0){
				output(_BV(ship[i].row) + _BV(ship[i].row+1));
			}
			else if(i==1){
				output(_BV(ship[i].row) + _BV(ship[i].row-1));
			}
			_delay_ms(5000);
			
			//explode (flash the columns and rows)
			
			for(unsigned char j = ship[0].row; j>=1; j--){
				columnRed(ship[0].col);
				output(_BV(j-1));
				_delay_ms(200);
				if(j==1){
					disableMatrix();
				}
			}
			//dispArt();
			LCD_ClearScreen();
			LCD_DisplayString(1, "You died!");
			LCD_Cursor(0);
			_delay_ms(7777);
			LCD_ClearScreen();
			LCD_DisplayString(1, "Final Score: ");
			dispScore("Final Score: ");
			_delay_ms(15000);
			initPos();
		}
	}
	
	//check for collision with other ships
	
	for(unsigned int i = 0; i<2; i++){
		for(unsigned int j = 0; j<2; j++){
			for(unsigned int k = 0; k<2; k++){
				if(ship[i].col==enemyShip[j][k].col && ship[i].row==enemyShip[j][k].row){
					//display two red dots at player ship pos
					columnRed(ship[i].col);
					output(_BV(ship[i].row) + _BV(ship[i].row+1));
					_delay_ms(5000);
					
					for(unsigned char j = ship[0].row; j>=1; j--){
						columnRed(ship[0].col);
						output(_BV(j-1));
						_delay_ms(200);
						if(j==1){
							disableMatrix();
						}
					}
					LCD_ClearScreen();
					LCD_DisplayString(1, "You died!");
					LCD_Cursor(0);
					_delay_ms(7777);
					LCD_ClearScreen();
					LCD_DisplayString(1, "Final Score: ");
					dispScore("Final Score: ");
					_delay_ms(15000);
					initPos();
				}
			}
		}
	}
	
}

bool isEmptyPos(unsigned char randCol, unsigned char randRow){
	//check player ship
	for(unsigned char i = 0; i<2; i++){
		if(randCol==ship[i].col && randRow==ship[i].row){
			return false;
		}
	}
	
	//check enemy ships
	for(unsigned char i = 0; i<2; i++){
		for(unsigned char j = 0; j<2; j++){
			if(randCol==enemyShip[i][j].col && randRow==enemyShip[i][j].row){
				return false;
			}
		}
	}
	
	//check bullets
	if((randCol==shipCol) || (randCol==eColA && randRow==eBulletPosA) || (randCol==eColB && randRow==eBulletPosB)){
		return false;
	}
	
	return true;
}

void output(unsigned char led){
	clkLow();
	latchLow();
	bool set = false;
	for(unsigned char i = 7; i>=0 && !set; i--){
		if (GetBit(led,(7-i)))  //bit_is_set doesn’t work on unsigned int so we do this instead
		dataHigh();
		else
		dataLow();
		clkHigh();
		clkLow();
		if(i==0){
			set = true;
			i++;
		}
	}
	latchHigh();
}

//state machines
enum bullet_states {bullet_init, bullet_wait, bullet_dispPlayer, bullet_dispEnemy, bullet_dispBoth} bullet_state;
void bullet(unsigned char pBulletPos, unsigned char eBulletPosA, unsigned char eBulletPosB){ //start at peak of ship
	if(play){
		dispShip();
		
		//player bullets
		if(pBulletPos!=0  && ship[0].row!=0){
			columnRed(shipCol);
			output(pBulletPos);
		}
		
		//enemy bullets;
		columnRed(eColA);
		output(eBulletPosA);
		
		columnRed(eColB);
		output(eBulletPosB);
	}
	
	switch(bullet_state){
		case -1:
			bullet_state = bullet_init;
			break;
		case bullet_init:
			bullet_state = bullet_wait;
			break;
		case bullet_wait:
			if(playerBulletCnt<2 || enemyBulletCnt<enemyPeriod){
				bullet_state = bullet_wait;
				if((playerBulletCnt+1)==2){
					nextBullet = true;
				}
				if((enemyBulletCnt+1)==enemyPeriod){
					eNextBullet = true;
				}
			}
			else if(!(playerBulletCnt<2) && !(enemyBulletCnt<enemyPeriod)){
				bullet_state = bullet_dispBoth;
			}
			else if(!(playerBulletCnt<2) && enemyBulletCnt<enemyPeriod){
				bullet_state = bullet_dispPlayer;
			}
			else if(playerBulletCnt<2 && !(enemyBulletCnt<enemyPeriod)){
				bullet_state = bullet_dispEnemy;
			}
			break;
		case bullet_dispPlayer:
			bullet_state = bullet_wait;
			break;
		case bullet_dispEnemy:
			bullet_state = bullet_wait;
			break;
		case bullet_dispBoth:
			bullet_state = bullet_wait;
			break;
		default:
			bullet_state = bullet_init;
			break;
	}
	switch(bullet_state){
		case bullet_init:
			break;
		case bullet_wait:
			playerBulletCnt++;
			enemyBulletCnt++;
			break;
		case bullet_dispPlayer:
			playerBulletCnt = 0;
			nextBullet = false;
			break;
		case bullet_dispEnemy:
			enemyBulletCnt = 0;
			eNextBullet = false;
			break;
		case bullet_dispBoth:
			playerBulletCnt = 0;
			enemyBulletCnt = 0;
			eNextBullet = false;
			nextBullet = false;
			break;
		default:
			break;
	}
}

enum disp_states {disp_init, disp_test} disp_state;
void display(){ //test code for LED Matrix
	switch(disp_state){
		case -1:
			disp_state = disp_init;
			break;
		case disp_init:
			disp_state = disp_test;
			break;
		case disp_test:
			disp_state = disp_test;
			break;
		default:
			disp_state = disp_init;
			break;
	}
	switch(disp_state){
		case disp_init:
			break;
		case disp_test:
			//display stuff on LED matrix
			for(unsigned char j = 0; j<8; j++){
				for(unsigned char i = 128; i>=1; i/=2){
					softReset();
					columnGreen(j);
					output(i);
					_delay_ms(100);
				}
				j++;
				bool leave = false;
				for(unsigned char i = 1; !leave;){
					softReset();
					columnGreen(j);
					output(i);
					_delay_ms(100);
					if(i!=128){
						i*=2;
					}
					else if(i==128){
						leave = true;
					}
				}
			}
			break;
		default:
			break;
	}
}

enum menu_states {menu_init, menu_wait, menu_play, menu_difficulty, menu_egg, menu_waitDiff, menu_wait2, menu_beforePlay} menu_state;
void menu(){ //display on LCD display
	if(!play){
		dispArt();
	}
	switch(menu_state){
		case -1:
			menu_state = menu_init;
			break;
		case menu_init:
			menu_state = menu_wait;
			break;
		case menu_wait:
			//counter to create random seed
			if(!seeded){
				if(!GetBit(PINA, 1)){
					seedCnt++;
				}
				else if(GetBit(PINA, 1)){
					srand(seedCnt);
					seeded = true;
				}
			}
			if(GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){ //if A0 is 1
				menu_state = menu_waitDiff;
				menuCnt = 1000;
			}
			else if(!GetBit(PINA, 0) && GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){ //if A1 is 1
				menu_state = menu_beforePlay;
			}
			else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && GetBit(PINA, 2) && !GetBit(PINA, 3)){ //if A2 is 1
				menu_state = menu_egg;
			}
			else{
				menu_state = menu_wait;
			}
			break;
		case menu_waitDiff:
			if(GetBit(PINA, 0)){
				menu_state = menu_waitDiff;
			}
			else if(!GetBit(PINA, 0)){
				menu_state = menu_difficulty;
			}
			break;
		case menu_wait2:
			if(GetBit(PINA, 0) || GetBit(PINA, 1) || GetBit(PINA, 2) || GetBit(PINA, 3)){
				menu_state = menu_wait2;
			}
			else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){
				menu_state = menu_wait;
			}
			break;
		case menu_beforePlay:
			if(GetBit(PINA, 1)){
				menu_state = menu_beforePlay;
			}
			else if(!GetBit(PINA, 1)){
				menu_state = menu_play;
			}
			break;
		case menu_play:
			if(play){
				menu_state = menu_play;
				break;
			}
			else if(!play){
				menu_state = menu_wait;
			}
			break;
		case menu_difficulty:
			if(diffSet){
				menu_state = menu_wait2;
				menuCnt = 1000;
				diffSet = false;
			}
			else if(!diffSet){
				menu_state = menu_difficulty;
			}
			break;
		case menu_egg:
			if(GetBit(PINA, 4)){
				menu_state = menu_wait2;
			}
			else if(!GetBit(PINA, 4)){
				menu_state = menu_wait;
			}
			break;
		default:
			menu_state = menu_init;
			break;
	}
	switch(menu_state){
		case menu_init:
			break;
		case menu_wait: //menu selection screen
			play = false;
			
			menuCnt++;
			if(menuCnt==1001){
				LCD_ClearScreen();
				LCD_DisplayString(1, msgList[0]);
			}
			
			if(menuCnt==2000){
				LCD_ClearScreen();
				LCD_DisplayString(1, msgList[1]);
			}

			if(menuCnt==3000){
				LCD_ClearScreen();
				LCD_DisplayString(1, msgList[2]);
			}
			
			if(menuCnt==4000){
				LCD_ClearScreen();
				LCD_DisplayString(1, msgList[3]);
			}
			
			if(menuCnt==5000){
				LCD_ClearScreen();
				LCD_DisplayString(1, msgList[4]);
				menuCnt = 0;
			}
			break;
		case menu_beforePlay:
			dispScore("Score: ");
			play = true; //play the game
			moveCond = false;
			break;
		case menu_play:
			dispScore("Score: ");
			play = true; //play the game
			moveCond = true;
			break;
		case menu_difficulty:
			play = false;
			//dispArt();
			menuCnt++;
			if(menuCnt==1001){
				LCD_ClearScreen();
				LCD_DisplayString(1, diffMsg[0]);
			}

			else if(menuCnt==2000){
				LCD_ClearScreen();
				LCD_DisplayString(1, diffMsg[1]);
				menuCnt = 0;
			}
			
			if(GetBit(PINA, 0) || GetBit(PINA, 1) || GetBit(PINA, 2) || GetBit(PINA, 3)){
				if(!GetBit(PINA, 0) && GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){ //lowest difficulty
					enemyPeriod = 1000;
				}
				else if(GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){
					enemyPeriod = 750;
				}
				else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && GetBit(PINA, 2) && !GetBit(PINA, 3)){
					enemyPeriod = 400;
				}
				else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && GetBit(PINA, 3)){
					enemyPeriod = 200;
				}
				diffSet = true;
			}
			break;
		case menu_waitDiff:
			break;
		case menu_wait2:
			break;
		case menu_egg:
			display();
			break;
		default:
			menu_state = menu_init;
			break;
	}
}

enum move_states {move_init, move_wait, move_play, move_north, move_east, move_south, move_west, move_wait2} move_state;
void move(){
	atEdge();
	switch(move_state){ //transitions
		case -1:
			move_state = move_init;
			break;
		case move_init:
			move_state = move_wait;
			break;
		case move_wait:
			if(moveCond){
				if(GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){ //move north
					move_state = move_north;
				}
				else if(!GetBit(PINA, 0) && GetBit(PINA, 1) && !GetBit(PINA, 2) && !GetBit(PINA, 3)){ //move east
					move_state = move_east;
				}
				else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && GetBit(PINA, 2) && !GetBit(PINA, 3)){ //move south
					move_state = move_south;
				}
				else if(!GetBit(PINA, 0) && !GetBit(PINA, 1) && !GetBit(PINA, 2) && GetBit(PINA, 3)){ //move west
					move_state = move_west;
				}
				else{
					move_state = move_wait;
				}
			}
			//dispShip(); //this statement was causing excessive power consumption
			break;
		case move_north:
			move_state = move_wait;
			break;
		case move_east:
			move_state = move_wait;
			break;
		case move_south:
			move_state = move_wait;
			break;
		case move_west:
			move_state = move_wait;
			break;
		case move_wait2:
			if(GetBit(PINA,0) || GetBit(PINA,1) || GetBit(PINA,2) || GetBit(PINA,3)){
				move_state = move_wait2;
			}
			else{
				move_state = move_wait;
			}
			break;
		default:
			move_state = move_init;
			break;
	}
	switch(move_state){ //actions
		case move_init:
			break;
		case move_wait:
			break;
		case move_north:
			if(!rN){
				for(unsigned char i = 0; i<2; i++){
					ship[i].row-=1; //update ship's position
				}
			}
			break;
		case move_east:
			if(!rE){
				for(unsigned char i = 0; i<2; i++){
					ship[i].col+=1; //update ship's position
				}
			}
			break;
		case move_south:
			if(!rS){
				for(unsigned char i = 0; i<2; i++){
					ship[i].row+=1; //update ship's position
				}
			}
			break;
		case move_west:
			if(!rWEST){
				for(unsigned char i = 0; i<2; i++){
					ship[i].col-=1; //update ship's position
				}
			}
			break;
		case move_wait2:
			break;
		default:
			break;
	}
}

enum softRest_states {softReset_init, softReset_wait, softReset_reset, softReset_wait2} softReset_state;
void softReset(){
	switch(softReset_state){
		case -1:
			softReset_state = softReset_init;
			break;
		case softReset_init:
			softReset_state = softReset_wait;
			break;
		case softReset_wait:
			if(GetBit(PINA, 4)){
				softReset_state = softReset_reset;
			}
			else if(!GetBit(PINA, 4)){
				softReset_state = softReset_wait;
			}
			break;
		case softReset_reset:
			softReset_state = softReset_wait2;
			break;
		case softReset_wait2:
			if(GetBit(PINA, 4)){
				softReset_state = softReset_wait2;
			}
			else if(!GetBit(PINA, 4)){
				softReset_state = softReset_wait;
			}
			break;
		default:
			softReset_state = softReset_wait;
			break;
	}
	switch(softReset_state){
		case -1:
			softReset_state = softReset_init;
			break;
		case softReset_init:
			softReset_state = softReset_wait;
			break;
		case softReset_wait:
			break;
		case softReset_reset:
			flushScreen();
			initPos();
			enemyPeriod = 1000;
			break;
		case softReset_wait2:
			break;
		default:
			break;
	}
}

void initPos(){
	//define player ship initial pos
	ship[0].row = 6;
	ship[0].col = 4;
	ship[1].row = 7;
	ship[1].col = 4;

	//define enemy ships initial pos
	enemyShip[0][0].row = 1;
	enemyShip[0][0].col = 1;
	enemyShip[0][1].row = 2;
	enemyShip[0][1].col = 1;

	enemyShip[1][0].row = 1;
	enemyShip[1][0].col = 6;
	enemyShip[1][1].row = 2;
	enemyShip[1][1].col = 6;
	
	move_state = -1;
	bullet_state = -1;
	menu_state = -1;
	
	eNextBullet = false;
	nextBullet = false;
	playerBulletCnt = 0;
	eCnt = 0;
	enemyBulletCnt = 0;
	score = 0;
	playDisplayed = false;
	play = false;
	welcDisplayed = false;
	
	eBulletPosA = enemyShip[0][1].row+1;
	eBulletPosB = enemyShip[1][1].row+1;
	
	eColA = enemyShip[0][1].col;
	eColB = enemyShip[1][1].col;
}

int main(void){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00; //SPI Shift Register / LED Matrix
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	initPos();
	LCD_init();
	TimerSet(77);
	TimerOn();
	
	while(1){
		softReset();
		menu();
		if(play){
			shipCol = ship[0].col;
			for(pBulletPos = ship[0].row+1; pBulletPos>=1;){ //player bullet position iteration
				move();
				enemyMove();
				hitPlayer();
				hitEnemy(shipCol, pBulletPos);
				flushEnemy(flushEnemyCond);
				softReset();
				pChanged = false; eChangedA = false; eChangedB = false;
				while(!TimerFlag){
					flushEnemy(flushEnemyCond);
					bullet(_BV(pBulletPos-1), _BV(eBulletPosA), _BV(eBulletPosB));
					bulletPositions();
				}
				hitEnemy(shipCol, pBulletPos);
				if(pBulletPos==1){ //makes sure bullet goes off screen
					pBulletPos = 0;
				}
				if(flushCnt<11){
					flushCnt++;
				}
				if(!(flushCnt<11)){
					flushEnemyCond = false;
					flushCnt = 0;
				}
				TimerFlag = 0;
			}
		}
	}
}