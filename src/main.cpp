#include "main.h"
#include <fstream>
#include "auton_assist_methods.h"
#include "globals.h"

// Constants
const bool TORQUE_THRESHOLD = 1.99;

// State variables
int liftState;
bool gripState;
bool hookState;
double gripHoldPosition;
bool initialized;

// Anti-doubles
bool prevUp;
bool prevB;

double lastVibrate = 0;

char autonMode = 'N'; // Stands for none

pros::vision_signature sigGoalRed = goalVision.signature_from_utility(1, 5607, 8193, 6900, -793, -297, -545, 3.7, 0);
pros::vision_signature sigGoalBlue = goalVision.signature_from_utility(2, -2909, -2315, -2612, 8851, 10215, 9533, 10.5, 0);
pros::vision_signature sigGoalYellow = goalVision.signature_from_utility(3, 431, 745, 588, -3343, -3041, -3192, 8.2, 0);

// Autons
void skillsAuton()
{
	// THE STRATEGY
	// 1. Grab the red goal and score it on the other side, pushing the yellow in the way
	// @ 40
	// 2. Grab the unmoved small yellow scoring on the red platform
	// @ 80
	// 3. Grab the yellow we pushed out of the way earlier and score that.
	// @ 120
	// 3. Grab the nearby blue and score that
	// @ 160
	// 4. Grab the tall yellow and hang
	// @ 230

    // Get the red goal
	driveViaIMU(.3, 0);
	grab();
	driveViaIMU(-.4, 0);
	liftSmall();

	// Line up with the yellow goal and push in corner
	turnViaIMU(93);
	driveViaIMU(-2.2, 93);
	
	// Line up with the bridge and get around the base
	driveViaIMU(.6, 93); // .9 > .7 > .95
	liftMax();
	turnViaIMU(-45);
	pros::delay(500);
	driveViaIMU(.5, -45); // Get near platform
	driveViaTime(2000, 600); // Make sure we are around the base
	
	// Score red
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Line up with far yellow and grab
	liftMin();
	turnViaIMU(33);
	pros::delay(600);
	driveViaSig(1.2, 3);
	grab();

	// Turn around and get to platfrom
	liftMax();
	turnViaIMU(-147);
	driveToRamp(2000, true); // 2000 ms, isRedRamp

	/*
	// Score the second yellow that we deal with
	liftScore();
	pros::delay(300);
	ungrab();

	// Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Get the yellow we pushed out of the way earlier
	turnViaIMU(-180);
	driveViaIMU(1, -180);
	turnViaIMU(-90);
	driveViaSig(.3, 3);
	grab();

	// Reverse of above
	driveViaIMU(-.3, -90);
	turnViaIMU(-180);
	liftMax();
	driveViaIMU(-1, -180);
	turnViaIMU(-90);

	// Score the yellow
	driveViaTime(1000, 600); // Make sure we are around the base
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, -90);

	// Get the blue from down south
	turnViaIMU(-180);
	liftMin();
	driveViaIMU(1.7, -180); // get near the blue
	driveViaSig(1, 2); // Finish the journey w/ the camera
	grab();

	// get to the bridge
	turnViaIMU(50);
	driveViaIMU(6, 50);
	driveToRamp(2000, false);

	// score the blue
	liftScore();
	pros::delay(300);
	ungrab();

	//Get off of platform
	driveViaTime(1000, -90);
	liftMax();
	pros::delay(600);
	driveViaIMU(-.5, 90);

	// Get the tall yellow
	turnViaIMU(90);
	driveViaSig(1, 3);
	grab();

	// Score and pull a judas
	turnViaIMU(90);
	driveToRamp(2000, false);
	liftScore();
	judas();

	*/

}

void compLeftAuton()
{
	driveViaIMU(.5, 0);
	pros::delay(750);
	driveViaIMU(-.5, 0);
}

void compForwardAuton()
{
	driveViaIMU(1.75, 0);
	grab();
	pros::delay(500);
	driveViaIMU(-1.5, 0);
}

void compRightAuton()
{
	driveViaIMU(.7, 0);
	grab();
	pros::delay(500);
	driveViaIMU(-.5, 0);
}

// For screwing around
void experimental()
{
	// Auton with f-gladiators
	// pros::delay(6000);
	// driveViaIMU(.9, 0);
	// turnViaIMU(-45);
	// driveViaIMU(1.3, -45);
	// grab();
	// pros::delay(200);
	// driveViaIMU(-1.5, -45);

	driveToRamp(2000, true);
}

// opcontrol
void setLift()
{
	if(master.getDigital(okapi::ControllerDigital::R1)) lift.moveVelocity(600);
	else if(master.getDigital(okapi::ControllerDigital::R2)) lift.moveVelocity(-600);
	else lift.moveVelocity(0);
}

void setDTSpeeds()
{
	// Store joysticks range = [-1, 1]
	double joyLY = master.getAnalog(okapi::ControllerAnalog::leftY);
	double joyRX = master.getAnalog(okapi::ControllerAnalog::rightX);

	// Filter joysticks
	if(abs(joyLY) < .1){
		joyLY = 0;
	}

	if(abs(joyRX) < .1){
		joyRX = 0;
	}

	// Convert joysticks to wheel speeds
	double wheelLeftSpeed = joyLY - joyRX;
	double wheelRightSpeed = joyLY + joyRX;
	double wheelBackSpeed = joyLY;

	// Filter wheel speeds (We got none right now)

	// Wheel speed assignments
	leftMotor.moveVelocity(wheelLeftSpeed * 600); // Speed is velocity pct * gearbox
	rightMotor.moveVelocity(wheelRightSpeed * 600);
	backMotor.moveVelocity(wheelBackSpeed * 600);
}

void setGrip(){
	// Grip variables
	if(master.getDigital(okapi::ControllerDigital::L1) && gripState == true){
		grip.set_value(false);
		gripState = false;
		master.rumble(".");
	}
	if(master.getDigital(okapi::ControllerDigital::L2) && gripState == false){
		grip.set_value(true);
		gripState = true;
		master.rumble("-");
	}
}

void setVibrate(){
	if(pros::millis() - lastVibrate > 750 && lift.getPosition() > 1){
		master.rumble(".");
		lastVibrate = pros::millis();
	}
}

void setHook(){
	// Store controller state
	bool buttonUp = master.getDigital(okapi::ControllerDigital::up);

	// State changer
	if(buttonUp && !prevUp){
		hookState = !hookState;
	}
	if(!hookState){
		hook.moveAbsolute(0, 100);
	}
	if(hookState){
		hook.moveAbsolute(6.5, 100);
	}

	// Update variables
	prevUp = buttonUp;
}

// PROS-called functions
void initialize() {
}

void competition_initialize()
{
}

void disabled() {}

void autonomous() {
	if(autonMode == 'X') skillsAuton();
	if(autonMode == '<') compLeftAuton();
	if(autonMode == '>') compRightAuton();
	if(autonMode == '^') compForwardAuton();
	if(autonMode == 'A') experimental();
}

void opcontrol() {
	if(!initialized){
		// Initialize stuff
		pros::lcd::initialize();

		// Configure the goal vision sensor
		goalVision.set_exposure(33);
		goalVision.set_led(65280);

		// Calibrate IMU
		master.setText(0, 0, "Calibrating...");
		imu.calibrate();
		while (imu.isCalibrating()){pros::delay(10);}
		master.clear();

		// Tare hook
		hook.moveVelocity(-100);
		while(!hookStop.get_value()){pros::delay(10);}
		hook.moveVelocity(0);
		hook.tarePosition();
		
		// Everything holds
		leftMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
		rightMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
		backMotor.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);
		lift.setBrakeMode(okapi::AbstractMotor::brakeMode::hold);

		// Render the prompt
		master.setText(0, 0, "Select a mode:");

		// Get the choice
		while(autonMode == 'N'){
			// Letter buttons
			if(master.getDigital(okapi::ControllerDigital::A)) autonMode = 'A';
			if(master.getDigital(okapi::ControllerDigital::B)) break;
			if(master.getDigital(okapi::ControllerDigital::X)) autonMode = 'X';
			if(master.getDigital(okapi::ControllerDigital::Y)) autonMode = 'Y';

			// Arrow buttons
			if(master.getDigital(okapi::ControllerDigital::left)) autonMode = '<';
			if(master.getDigital(okapi::ControllerDigital::right)) autonMode = '>';
			if(master.getDigital(okapi::ControllerDigital::up)) autonMode = '^';
			if(master.getDigital(okapi::ControllerDigital::down)) autonMode = 'V';
		}
		pros::delay(1000);
		master.clear();
	}
	initialized = true;

	pros::delay(1000);

	while (true) {
		setVibrate();
		setGrip();
		setHook();
		setDTSpeeds();
		setLift();
		pros::lcd::clear();
		pros::lcd::print(2, "Lift location:%4.2f", lift.getPosition());
		pros::delay(10);
	}
}
