/********************************************
**  Name: Seed Counter 
**  Version: V0.5
**  Last Updated: 13/05/2011
*********************************************
**  REVISIONS
**
**  V0.5 - 13/05/2011
**  * Improvement of motor handeling with 4 new options: Full step,half step, quarter step, eighth step
**  * Improved counting steps, now we have number of complete cycles + number of steps
**    Example: Motor 1 count -> 20 turns + 2034 steps
*********************************************
*/


#include "WProgram.h"
#include "Stepper_ac05.h"				   		   

// SETUP	
//int step_pin -- Pin where the step control is connected
//int direction_pin -- Pin where the direction control is connected
//int sensor_pin -- Pin where the sensor output is connected (if any)
//					 -- Linear movement count steps and full cicles, example: 3 turns + 2345 steps
//					 -- Loop movement the counter reset itself after a full cycle, example:
//					 -- If a motor has 200 stpes the stpe number 201 will automaticalli go back to 0
//unsigned long motor_steps -- Total of steps of the motor
//int step_mode		 -- 4 modes available: 1=FULL, 2=HALF, 4=QUARTER, 8=EIGHTH							  
Stepper_ac::Stepper_ac(int step_pin, int direction_pin, int sensor_pin, unsigned long motor_steps, int step_mode)
{
  pinMode(step_pin, OUTPUT);
  pinMode(direction_pin, OUTPUT);
  pinMode(sensor_pin, INPUT);
  _step_pin = step_pin;
  _direction_pin = direction_pin;
  _sensor_pin = sensor_pin;
  _motor_steps = (motor_steps * step_mode);
  // Set position of the motor to 0;
  stepPosition = 0;
  _stepPosition = 0;
  stepCycle = 0;
  _stepCycle = 0;

}

void Stepper_ac::set_init_position()
{
  // Set position of the motor to 0;
  _stepPosition = 0;
  _stepCycle = 0;
}

// This function returns the number of cycles the motor did positive or negative also counts for left right
int Stepper_ac::get_steps()
{
	stepPosition = _stepPosition;
	return stepPosition;
}

// This function returns the number of cycles the motor did. Example
// motor of 200 steps did 230 steps clockwise so = 1 cycle + 30 steps
int Stepper_ac::get_steps_cycles()
{
	stepCycle = _stepCycle;
	return stepCycle;
}

// Sets direction of the motor movement
void Stepper_ac::set_direction(bool direction)
{
  _direction=direction;
  if(_direction){
	digitalWrite (_direction_pin, HIGH);
  }
  else{
    digitalWrite (_direction_pin, LOW);
  }	
}

// Reads the actual direction of the motor
bool Stepper_ac::get_direction()
{
	return digitalRead(_direction_pin);
}

// Order to make the motor move one step (with coun ting steps enabled)
void Stepper_ac::do_step()
{
  // Just as information, variable _stepPosition doesn't do anything in the code
  // It is just a mere information of position (in steps) to know where the motor is
  // And because its a private variable, each instance of motor has its own position
  // static unsigned char counter = 0;
  // Update position
  count_step(_direction);

  // Change the counter in order to add steps at it
  // like using a full 4 pins driver it would need 
  // at least 4 steps, or 8 for doubled steps
  // Since we are using easy driver we need to count 0 to 1 only (true false, just a normal pulse)
  // Stay within the steptab
  //counter &= B00000001; //Force count 0 to 1 because we have only 2 steps
  move_step();
}

void Stepper_ac::count_step(bool _temp_direction)
{
	if(_temp_direction){
		_stepPosition++;
	}else{
		_stepPosition--;
	}
	
	if (_stepCycle == 0) {  // If we are in the first turn we see if we gowing forward or backwards
		if (_stepPosition == (_motor_steps + 1)){  // we chek the upper limit (if there was no limit (0) nothing will happen)
			_stepCycle++; 
			_stepPosition = 1;
		}else if (_stepPosition == -(_motor_steps + 1)) {  // we chek the lower limit, -1 
			_stepCycle--;
			_stepPosition = -1;
		}
	}else if (_stepCycle > 0) {  // Means we go forwards so count in the positive range
		if (_stepPosition == _motor_steps + 1){  // we chek the upper limit (if there was no limit (0) nothing will happen)
			_stepCycle++; 
			_stepPosition = 1;
		}else if (_stepPosition == 0) {  // we chek the lower limit, -1 
			_stepCycle--;
			_stepPosition = _motor_steps;
		}
	}else if (_stepCycle < 0) {  // Means we go backwards so count in the negative range
		if (_stepPosition == -(_motor_steps + 1)){  // In negative range this is the lower limit
			_stepCycle--; 
			_stepPosition = -1;
		}else if (_stepPosition == 0) {  // In negative range this is the upper limit
			_stepCycle++;
			_stepPosition = -_motor_steps;
		}
	}
}

int theDelay = 1;

// This order will move the motor one step in the designed direction but without updating the counter
void Stepper_ac::move_step()
{
  //Using arduino code might take too long to put the pin on and off  (OLD approach)
  //digitalWrite (_step_pin, Easy_steptab[pos]);
  
  // This is where we send to the easy driver a pulse with the right Width of Minimum 1.0 µs
  digitalWrite (_step_pin, HIGH);
  delayMicroseconds(theDelay);   // This is a trick to generate exactly 1uS delay in Arduino
  digitalWrite (_step_pin, LOW);
  delayMicroseconds(theDelay);   // This is a trick to generate exactly 1uS delay in Arduino
  // Output the fast way
  //SM_PORT |= ((temp<<4)&0xF0);
  //SM_PORT &= ((temp<<4)|0x0F);
}

// function to find the initial position of a motor thru the sensor
void Stepper_ac::initiate()
{
	set_direction (false);   // Goes back till we find the sensor
	while (sensor_check())
	{
		do_step();
	}
	set_init_position();
}

// Checks whether the sensor atached to the motor detects something
bool Stepper_ac::sensor_check()
{
	//bool a = false;
	//a = digitalRead(_sensor_pin);
	// Because its a barrier sensor the pin its inverted, so we have 0 when we have a sensor
	// hence the "!" in front of the digitalRead, so we invert the signal ** RECHEK!!!
	return (!digitalRead(_sensor_pin));
}

void Stepper_ac::wait_till_reach_position(long m_steps) 
{
	while (get_steps() != m_steps) {
        // wait doing nothing until we arrive to the deposit
    } 
}
