//
// EEPROM DB example
//
// This example creates a database, adds 10 records, and reads the 10 records
// displaying the results.  
//


//*********************
// DEFINE DATABASE
//*********************
#include "WProgram.h"
#include <EEPROM.h>
#include <DB.h>
#include <string.h>

DB db;

#define MY_TBL 1

struct MyRec {
// Containig
// X coarse - X fine
// Y coarse - Y fine
  unsigned int Xc;
  unsigned int Xf;
  unsigned int Yc;
  unsigned int Yf;
} mposition;

// Y
#define Yc1 0
#define Ys1 0
#define Yc2 7
#define Ys2 368
// X
#define Xc1 70
#define Xs1 1472
#define Xc2 79
#define Xs2 132
#define Xc3 87
#define Xs3 964
#define Xc4 95
#define Xs4 944
#define Xc5 103
#define Xs5 956

#define number_of_positions 20
// PROGMEM  prog_uchar name_of_array
// prog_uchar is an unsigned char (1 byte) 0 to 255
// prog_uint is an unsigned int (2 byte) 0 to 65,535
int x_axis_set[number_of_positions*2] = {
// Basic positions
0,0,			// 1- INIT position
6,0,			// 2- Blister position
213,1400,		// 3- Printer position
280,1,			// 4- Exit position
// 2 rows blisters
Xc1,Xs1,		// 5- Position Hole 1
Xc1,Xs1,		// 6- Position Hole 2
Xc2,Xs2,		// 7- Position Hole 3
Xc2,Xs2,		// 8- Position Hole 4
Xc3,Xs3,		// 9- Position Hole 5
Xc3,Xs3,		// 10- Position Hole 6
Xc4,Xs4,		// 11- Position Hole 7
Xc4,Xs4,		// 12- Position Hole 8
Xc5,Xs5,		// 13- Position Hole 9
Xc5,Xs5,			// 14- Position Hole 10
// 1row blisters
Xc1,Xs1,		// 5- Position Hole 1
Xc1,Xs1,		// 6- Position Hole 2
Xc2,Xs2,		// 7- Position Hole 3
Xc2,Xs2,		// 8- Position Hole 4
Xc3,Xs3		// 9- Position Hole 5
};


int y_axis_set[number_of_positions*2] = {
// Basic positions
0,0,			// 1- INIT position
0,0,			// 2- Blister position
3,834,			// 3- Printer position
0,0,			// 4- Exit position
// 2 rows blisters
Yc1,Ys1,		// 5- Position Hole 1
Yc2,Ys2,		// 6- Position Hole 2
Yc2,Ys2,		// 7- Position Hole 3
Yc1,Ys1,		// 8- Position Hole 4
Yc1,Ys1,		// 9- Position Hole 5
Yc2,Ys2,		// 10- Position Hole 6
Yc2,Ys2,		// 11- Position Hole 7
Yc1,Ys1,		// 12- Position Hole 8
Yc1,Ys1,		// 13- Position Hole 9
Yc2,Ys2,			// 14- Position Hole 10
// 1row blisters
Yc1,Ys1,		// 5- Position Hole 1
Yc2,Ys2,		// 6- Position Hole 2
Yc2,Ys2,		// 7- Position Hole 3
Yc1,Ys1		// 8- Position Hole 4
};

// advice: change names to Coarse & fine
int get_cycle_Xpos_from_index(int index) {
	return x_axis_set [(2*index) - 2];
}

int get_step_Xpos_from_index(int index) {
	return x_axis_set [(2*index) - 1];
}

int get_cycle_Ypos_from_index(int index) {
	return y_axis_set [(2*index) - 2];
}

int get_step_Ypos_from_index(int index) {
	return y_axis_set [(2*index) - 1];
}


void setup()
{
  Serial.begin(9600);
  Serial.println("EEPROM DB Library Demo");
  Serial.println();


  Serial.print("Creating Table...");
  db.create(MY_TBL,sizeof(mposition),number_of_positions);
  db.open(MY_TBL);
  Serial.print("Number of records in DB: ");Serial.println(db.nRecs(),DEC);
  Serial.println("DONE");
/*
  Serial.print("Creating records...");
  for (int recno = 1; recno <= number_of_positions; recno++)
  {

	mposition.Xc = get_cycle_Xpos_from_index(recno);
	mposition.Xf = get_step_Xpos_from_index(recno);
	mposition.Yc = get_cycle_Ypos_from_index(recno);
	mposition.Yf = get_step_Ypos_from_index(recno);

    Serial.print("CREATING RECNUM: "); Serial.println(recno);
    Serial.print("WRITING Xc: "); Serial.println(mposition.Xc);
	Serial.print("WRITING Xf: "); Serial.println(mposition.Xf);
	Serial.print("WRITING Yc: "); Serial.println(mposition.Yc);
	Serial.print("WRITING Yf: "); Serial.println(mposition.Yf);
    //db.append(DB_REC mposition);
    Serial.println("DONE");
  }
  Serial.println();
  Serial.println("Reading records from EEPROM...");
  Serial.println();
  */
  selectAll();
}

void loop()
{
}

void selectAll()
{
  if (db.nRecs()) Serial.println("-----");
  for (int i = 1; i <= db.nRecs(); i++)
  {
    db.read(i, DB_REC mposition);
    Serial.print("Memory position: "); Serial.print(i); 
	Serial.print(" * Xc: "); Serial.print(mposition.Xc);
	Serial.print(" * Xf: "); Serial.print(mposition.Xf);
	Serial.print(" ** Yc: "); Serial.print(mposition.Yc);
	Serial.print(" * Yf: "); Serial.println(mposition.Yf);
    Serial.println("-----");  
  } 
}