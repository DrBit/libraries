/*

Seed Counter

*/

// Inlcude for the self made library Stepper_ac
#include <Stepper_ac05.h> 

/////////////////////////////////////////////////////
//Calling the constructor for the Stepper_ac class 
/////////////////////////////////////////////////////
// Stepper_setup(int step_pin, int direction_pin);
// 
Stepper_ac motor1(13,12);		// Steing up motor1 
Stepper_ac motor2(11,10);		// Steing up motor2


void setup()
{
	// example
	motor1.set_direction (HIGH);
	motor2.set_direction (HIGH);
        motor1.set_init_position();    //sets the position back to 0
}

void loop()
{
		motor1.do_step ();
                delay (800);
}



