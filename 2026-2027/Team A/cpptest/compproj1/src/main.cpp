#include "main.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/misc.h"
#include "pros/motors.h"

 //motor groups
pros::Motor front_left_motor(-8, pros::E_MOTOR_GEAR_BLUE); 
pros::Motor middle_left_motor(9, pros::E_MOTOR_GEAR_BLUE); // 5.5W For
pros::Motor back_left_motor(-10, pros::E_MOTOR_GEAR_BLUE); 

pros::Motor front_right_motor(1, pros::E_MOTOR_GEAR_BLUE); 
pros::Motor middle_right_motor(-2, pros::E_MOTOR_GEAR_BLUE); // 5.5W Rev
pros::Motor back_right_motor(3, pros::E_MOTOR_GEAR_BLUE); 

pros::MotorGroup left_motors({front_left_motor, front_right_motor, middle_left_motor}); // left motors on ports 1 (Front reversed), 2 (5.5W forwards), and 3 (Back reversed)
pros::MotorGroup right_motors({front_left_motor, front_right_motor, middle_left_motor}); // right motors on ports 4 (forwards), 5 (5.5W reversed), and 6 (Back forwards)

//drivetrain settings
lemlib::Drivetrain drivetrain(&left_motors, // left motor group
                              &right_motors, // right motor group
                              12, // 12 inch track width
                              lemlib::Omniwheel::OLD_325_HALF, // using old 3.25" omnis
                              450, // drivetrain rpm is 450
                              2 // horizontal drift is 2 (for now)
);//wheelbase is 12"

//imu port on port 20
pros::Imu imu(20);

//old encoder code
//optical shaft encoder odoms (add comma true if reverse)
//pros::adi::Encoder vertical_encoder('A', 'B', true);//create an optical shaft encoder connected to ports 'A' and 'B'
//pros::adi::Encoder horizontal_encoder('C', 'D');//create an optical shaft encoder connected to ports 'C' and 'D'

pros::Rotation vertical_sensor(13);
pros::Rotation horizontal_sensor(-4);

//test for reversal
/**
void initialize() {
    pros::lcd::initialize(); // initialize brain screen
    while (true) { // infinite loop
        pros::lcd::print(1, "Rotation Sensor: %i", vertical_sensor.get_position());
        // print measurements from the rotation sensor
        pros::lcd::print(2, "Rotation Sensor: %i", horizontal_sensor.get_position());
        pros::delay(10); // delay to save resources. DO NOT REMOVE
    }
}*/

//horizontal tracking wheel (tune distances relative to center)
lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_sensor, lemlib::Omniwheel::NEW_2, -1.75); //NEED TO CONFIGURE TS!@!!
//vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_sensor, lemlib::Omniwheel::NEW_2, -0.875); //NEED TO CONFIGURE TS!@!!

//odom settings
lemlib::OdomSensors sensors(&vertical_tracking_wheel, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            &horizontal_tracking_wheel, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor 
);

// lateral PID controller
lemlib::ControllerSettings lateral_controller(10, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              3, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                              20 // maximum acceleration (slew)
);

// angular PID controller
lemlib::ControllerSettings angular_controller(12, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              12, // derivative gain (kD)
                                              0, // anti windup
                                              0, // small error range, in degrees
                                              0, // small error range timeout, in milliseconds
                                              0, // large error range, in degrees
                                              0, // large error range timeout, in milliseconds
                                              0 // maximum acceleration (slew)
);

//inputcurve for throttle input
lemlib::ExpoDriveCurve throttle_curve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve steer_curve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);

//create chassis
lemlib::Chassis chassis(drivetrain, // drivetrain settings
                        lateral_controller, // lateral PID settings
                        angular_controller, // angular PID settings
                        sensors, // odometry sensors
                        &throttle_curve,
                        &steer_curve
);

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */


void initialize() {
    pros::lcd::initialize();
    chassis.calibrate();

    pros::Task screen_task([]() {
        while (true) {
            const lemlib::Pose pose = chassis.getPose();
            const double target = 90.0;

            // Wrap error into approximately -180 to +180 degrees
            double error = target - pose.theta;
            while (error > 180) error -= 360;
            while (error < -180) error += 360;

            pros::lcd::print(0, "X: %.2f", pose.x);
            pros::lcd::print(1, "Y: %.2f", pose.y);
            pros::lcd::print(2, "Theta: %.2f", pose.theta);
            pros::lcd::print(3, "Error: %.2f", error);

            pros::delay(50);
        }
    });
}


/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	//tune angular controller
    // set position to x:0, y:0, heading:0
    chassis.setPose(0, 0, 0);
    // turn to face heading 90 with a very long timeout
    chassis.turnToHeading(90, 100000);

	
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */


pros::Controller controller(pros::E_CONTROLLER_MASTER);

void opcontrol() {
    // loop forever
    while (true) {
        // get left y and right x positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        int rightBut = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
        // move the robot
        if(abs(leftY)<4 | rightBut==1 ){
            chassis.arcade(leftY, rightX);
        } else {
            chassis.curvature(leftY, rightX);
        }
        

        // delay to save resources
        pros::delay(25);
    }
}