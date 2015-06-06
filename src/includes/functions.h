#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//functions
void atEdge();
void columnBullets(unsigned char num);
void columnGreen(unsigned char num);
void columnRed(unsigned char num);
void disableMatrix();
void dispArt();
void dispMsg(const unsigned char* message);
void dispScore(const unsigned char* text);
void dispShip();
void enemyMove();
void flushScreen();
void hitEnemy(unsigned char shipCol, unsigned char pBulletPos);
void hitPlayer();
unsigned char isEmptyPos(unsigned char randCol, unsigned char randRow);
void output(unsigned char led);

//state machines
void display();
void move();
void softReset();
void bullet(unsigned char pBulletPos, unsigned char eBulletPosA, unsigned char eBulletPosB);
void menu();

#endif