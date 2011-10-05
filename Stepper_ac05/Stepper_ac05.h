
#ifndef Stepper_ac_h
#define stepper_ac_h

#include "WProgram.h"


								   
class Stepper_ac
{
  public:
    // functions
    Stepper_ac(int step_pin, int direction_pin, int sensor_pin, unsigned long motor_steps, int step_mode);
    // chek if we can do unsigned long motor_steps into INT
	void set_init_position();
    void do_step();
	void count_step(bool _temp_direction);
	//int get_position();
	int get_steps();
	int get_steps_cycles();
	bool get_direction();
	void set_direction(bool direction);
	void move_step();
	void initiate();
	bool sensor_check();
	void wait_till_reach_position(long position);
	// VARs
	long stepPosition;  // check if it can be int
	int stepCycle;
	unsigned char Easy_steptab[];

  private:
    int _step_pin;
    int _direction_pin;
	int _sensor_pin;
	unsigned long _motor_steps;		//! Numer of stpes of the motor (if using a different type of motor)
	long _stepPosition;				//! Position of stepper motor (relative to starting position as zero)
	// chek if it can be int
	int _stepCycle;
	bool _direction;			
};

#endif


