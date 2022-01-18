#include "main.h"
#include "globals.h"

int sgn(double d) // Mimimcs the mathematical sgn function
{
	if(d < 0){return -1;}
	if(d > 0){return 1;}
	return 0;
}

// Auton assist methods //

void driveViaIMU(double dist, double rotation)
{
	dist *= 39.3701 / (2.75 * PI); // To in. then to rev

	// Configure controllers
	auto turnController = okapi::IterativeControllerFactory::posPID(.003, .0004, .0001); // PID for angular speed int aSpeed; int speed; okapi::EKFFilter kFilter;
	auto speedController = okapi::IterativeControllerFactory::posPID(1, 0, 0);

	turnController.setTarget(rotation);
	speedController.setTarget(dist);

	// Configure filters
	okapi::EKFFilter speedFilter;
	okapi::EKFFilter turnFilter;

	// reset all motor encoders to zero
	backMotor.tarePosition();
	leftMotor.tarePosition();
	rightMotor.tarePosition();

	double d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;

	while (std::fabs(dist - d) > .3){
		d = (leftMotor.getPosition() + rightMotor.getPosition()) / 2;
		double speedInput = speedFilter.filter(d);
		double speed = speedController.step(speedInput);
		speed *= 600; // Scale from pct to wheel speed
		std::cout << "Input: " << speedInput << std::endl;
		std::cout << "Speed: " << speed << std::endl;

		double a = imu.get();
		double turnSpeedInput = turnFilter.filter(imu.get());
		double turnSpeed = turnController.step(turnSpeedInput);

		leftMotor.moveVelocity(speed - turnSpeed);
		rightMotor.moveVelocity(speed + turnSpeed);
		backMotor.moveVelocity(speed);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaTime(double ms, double vel, double rotation){
	double startTime = pros::millis();
	okapi::EKFFilter kFilter;
	while (pros::millis() - startTime < ms){
		int aSpeed = (rotation - kFilter.filter(imu.get())) * 30;
		leftMotor.moveVelocity(vel - aSpeed);
		rightMotor.moveVelocity(vel + aSpeed);
		backMotor.moveVelocity(vel);
		pros::delay(5);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

// NEEDS TUNING
void driveViaGPS(double locx, double locy)
{
	// Data smoothing filters
	okapi::EKFFilter kFilterRot;
	okapi::EKFFilter kFilterDist;

	// Variables
	double targetRotation; // Rotation to hold throughout the movement
	auto status = gps.get_status();
	double xDiff = locx - status.x;
	double yDiff = locy - status.y;
	double dist = std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2));

	// Update the target angle
	if (xDiff == 0) targetRotation = 90;
	else targetRotation = std::atan(yDiff/xDiff) * 180 / PI;
	
	targetRotation = -(targetRotation + 90);
	if (sgn(xDiff) == -1) targetRotation += 180;
	targetRotation *= -1;

	// Create a PID distance controller
	// auto distController = okapi::IterativeControllerFactory::posPID(.001, .0001, .0001);
	// distController.setTarget(0);

	// Until we reach our destination, set the speed
	while(dist > .1){
		// Update the distance
		status = gps.get_status();
		xDiff = locx - status.x;
		yDiff = locy - status.y;
		dist = kFilterDist.filter(std::sqrt(std::pow(xDiff, 2) + std::pow(yDiff, 2)));

		// Calculate our speed, which is a constant for now
		double speed = 500; // distController.step(dist) + 10; // 10 is added for testing's sake

		// Get the rotation from the gps
		double angularSpeed = (targetRotation - kFilterRot.filter(imu.get())) * 30; // NOTE: We can use gyro to maintain heading if gps sucks

		// Assign the calculated wheel values
		leftMotor.moveVelocity(speed - angularSpeed);
		rightMotor.moveVelocity(speed + angularSpeed);
		backMotor.moveVelocity(speed);

		pros::delay(5); // Delay for the other tasks
	}

	// Stop the robot after the movement is near the target location
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);
	backMotor.moveVelocity(0);
}

void driveViaVision(bool isRamp, double vel, double rotation, double dist){

	//Filters
	okapi::EKFFilter kFilterRot;
	okapi::EKFFilter kFilterDist;

	// Variables
	if(isRamp == true){
		
	} else{

	}
}

void turnViaIMU(double rotation)
{
	auto turnController = okapi::IterativeControllerFactory::posPID(.5, 0, .075); // PID for angular speed int aSpeed; int speed; okapi::EKFFilter kFilter;
	turnController.setTarget(heading);
	okapi::EKFFilter turnFilter;

	while (std::fabs(heading - turnFilter.filter(imu.get())) > .5){
		double aSpeed = turnController.step(turnFilter.getOutput());
		leftMotor.moveVelocity(-600 * aSpeed);
		rightMotor.moveVelocity(600 * aSpeed);
	}
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);

	/*
	double error = heading - imu.get();
	int rotation;
	backMotor.moveVelocity(0);
	while(std::fabs(error) > 2) // keeps turning until within 10 degrees of objective
	{
		if (std::fabs(error) < 40){
		// if within 40 degrees of objective, the motors start slowing
		// and the speed never drops below 20
		rotation = (12 * error);
		} else {
		// otherwise maintain fast turning speed of 90
		rotation = 540 * sgn(error);
		}

		rightMotor.moveVelocity(rotation);
		leftMotor.moveVelocity(-rotation);

		pros::delay(5);
		error = heading - imu.get();
	}
	// these next lines attempt to slow down the robot's rotational momentum
	// might be better just to put the motors into braking mode
	rotation = -15 * sgn(error);
	leftMotor.moveVelocity(rotation);
	rightMotor.moveVelocity(-rotation);
	pros::delay(50);
	leftMotor.moveVelocity(0);
	rightMotor.moveVelocity(0);*/
}

void grab()
{
	grip.set_value(true);
	pros::delay(300);
}

void ungrab() // NOTE: This has no wait, unlike the function above
{
	grip.set_value(false);
}

void liftMin() {lift.moveAbsolute(0, 90);}
void liftSmall() {lift.moveAbsolute(.4, 90);} // 0, .5, 1.7
void liftMax() {lift.moveAbsolute(1.75, 90);}
void liftScore() {lift.moveAbsolute(1, 90);}
void liftHang() {lift.moveAbsolute(1, 90);} // theoretically overshoots by .2

void scoreGoal()
{
	//Ensure lift is up
	liftScore();
	// Release Goal
	ungrab();
	// Lift the lift back to max
	liftMax();
}

// TODO: Pretty sure this is still UNTESTED / not ready
void judas()
{
	hook.moveAbsolute(7, 100);
}	
