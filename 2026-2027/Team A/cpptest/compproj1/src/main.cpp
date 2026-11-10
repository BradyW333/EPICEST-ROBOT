#include "main.h"
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/misc.h"
#include "pros/misc.hpp"
#include "pros/motors.h"
#include <string>
#include <algorithm>
#include <cmath>

//vars
bool pid_cal_driving = false;
bool pid_cal_lat = true; //true lat ang false

//drivetrain motor groups
pros::Motor front_left_motor(-12, pros::E_MOTOR_GEAR_BLUE); 
pros::Motor back_left_motor(-11, pros::E_MOTOR_GEAR_BLUE); 

pros::Motor front_right_motor(19, pros::E_MOTOR_GEAR_BLUE); 
pros::Motor back_right_motor(20, pros::E_MOTOR_GEAR_BLUE); 

pros::MotorGroup left_motors({front_left_motor, front_right_motor}); // left motors on ports 1 (Front reversed), 2 (5.5W forwards), and 3 (Back reversed)
pros::MotorGroup right_motors({front_left_motor, front_right_motor}); // right motors on ports 4 (forwards), 5 (5.5W reversed), and 6 (Back forwards)

//Cascade motor group
pros::Motor left_cascade_motor(-2, pros::E_MOTOR_GEAR_BLUE); 
pros::Motor right_cascade_motor(10, pros::E_MOTOR_GEAR_BLUE); 

//intake and banshee claw
pros::Motor intake_motor(3,pros::E_MOTOR_GEAR_BLUE);

pros::Motor banshee_lift(1,pros::E_MOTOR_GEAR_BLUE); //5.5v

pros::Motor banshee_claw(4,pros::E_MOTOR_GEAR_BLUE); //5.5v

//controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

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

//pros::Distance vertical_distance(1);
//pros::Distance horizontal_distance(1);
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



//vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_sensor, lemlib::Omniwheel::NEW_2, -0.875); //NEED TO CONFIGURE DISTANCE!@!!

//odom settings
lemlib::OdomSensors sensors(&vertical_tracking_wheel, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            nullptr, // horizontal tracking wheel 1, set to nullptr as we don't have a second one
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

    // pros::Task screen_task([]() {
    //     while (true) {
    //         const lemlib::Pose pose = chassis.getPose();
    //         const double target = 90.0;

    //         // Wrap error into approximately -180 to +180 degrees
    //         double error = target - pose.theta;
    //         while (error > 180) error -= 360;
    //         while (error < -180) error += 360;

    //         pros::lcd::print(0, "X: %.2f", pose.x);
    //         pros::lcd::print(1, "Y: %.2f", pose.y);
    //         pros::lcd::print(2, "Theta: %.2f", pose.theta);
    //         pros::lcd::print(3, "Error: %.2f", error);

    //         pros::delay(50);
    //     }
    // });

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




void opcontrol() {
    // loop forever
    bool intakeon = false;
    bool intakereverse = false;
    while (true) {
        // get left y and right x positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        int left2 = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2);
        int right2 = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2);
        int right1 = controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1);
        // move the robot
        if(abs(leftY)<4 | left2==1 ){
            chassis.arcade(leftY, rightX);
        } else {
            chassis.curvature(leftY, rightX);
        }
        
        if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R2)){
            intakeon = !intakeon;
        }

        intakereverse = right1 == 1;

        intake_motor.move(intakereverse ? -127 : (intakeon ? 127 : 0));
        // delay to save resources
        pros::delay(25);
    }
}

//pid calibration func 
void pidcal(){
    bool pid_cal_lat = false;
        bool pid_cal_kp = true;
        double angkP = 2;
        double angkD = 10;
        double latkP = 2;
        double latkD = 10;

        bool testing = false;
        bool dirty = true;
        uint32_t last_print = 0;
        uint32_t test_start = 0;
        uint32_t settled_since = 0;
        lemlib::Pose start(0, 0, 0);
        double lat_prev = 0;
        double ang_prev = 0;
        double overshoot = 0;
        double last_settle = 0;
        double last_overshoot = 0;

        auto wrap = [](double e) {
            while (e > 180) e -= 360;
            while (e < -180) e += 360;
            return e;
        };
        auto clamp127 = [](double v) { return std::clamp(v, -127.0, 127.0); };

        while (true) {
            const lemlib::Pose pose = chassis.getPose();
            const uint32_t now = pros::millis();

            if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT) && !testing) {
                pid_cal_lat = !pid_cal_lat;
                dirty = true;
            }
            if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)) {
                pid_cal_kp = !pid_cal_kp;
                dirty = true;
            }
            if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP) ||
                controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)) {
                const double dir = controller.get_digital(pros::E_CONTROLLER_DIGITAL_UP) ? 1 : -1;
                double& gain = pid_cal_lat ? (pid_cal_kp ? latkP : latkD) : (pid_cal_kp ? angkP : angkD);
                gain = std::max(0.0, gain + dir * (pid_cal_kp ? 0.5 : 1.0));
                dirty = true;
            }
            if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R1) && !testing) {
                testing = true;
                start = pose;
                test_start = now;
                settled_since = 0;
                overshoot = 0;
                lat_prev = pid_cal_lat ? 24 : 0;
                ang_prev = pid_cal_lat ? 0 : 90;
            }
            if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B) && testing) {
                testing = false;
                chassis.arcade(0, 0, true);
            }

            double lat_err = 0;
            double ang_err = 0;

            if (testing) {
                const double h = start.theta * M_PI / 180.0;
                if (pid_cal_lat) {
                    const double tx = start.x + 24 * std::sin(h);
                    const double ty = start.y + 24 * std::cos(h);
                    lat_err = (tx - pose.x) * std::sin(h) + (ty - pose.y) * std::cos(h);
                    ang_err = wrap(start.theta - pose.theta);
                } else {
                    ang_err = wrap(start.theta + 90 - pose.theta);
                }

                const double fwd = pid_cal_lat ? clamp127(latkP * lat_err + latkD * (lat_err - lat_prev)) : 0;
                const double turn = clamp127(angkP * ang_err + angkD * (ang_err - ang_prev));
                lat_prev = lat_err;
                ang_prev = ang_err;
                chassis.arcade(fwd, turn, true);

                const double primary = pid_cal_lat ? lat_err : ang_err;
                overshoot = std::max(overshoot, -primary);

                bool done = false;
                if (std::fabs(primary) < 1) {
                    if (settled_since == 0) settled_since = now;
                    if (now - settled_since >= 100) {
                        last_settle = settled_since - test_start;
                        done = true;
                    }
                } else {
                    settled_since = 0;
                }
                if (now - test_start > 4000) {
                    last_settle = -1;
                    done = true;
                }
                if (done) {
                    testing = false;
                    last_overshoot = overshoot;
                    chassis.arcade(0, 0, true);
                    printf("lat kP %.2f kD %.2f | ang kP %.2f kD %.2f | settle %.0f ms overshoot %.2f\n",
                        latkP, latkD, angkP, angkD, last_settle, last_overshoot);
                }
            }

            pros::lcd::print(0, "X: %.2f  Y: %.2f", pose.x, pose.y);
            pros::lcd::print(1, "Theta: %.2f", pose.theta);
            pros::lcd::print(2, "Mode: %s %s", pid_cal_lat ? "LATERAL 24in" : "ANGULAR 90deg", testing ? "(running)" : "");
            pros::lcd::print(3, "Error: %.2f", pid_cal_lat ? lat_err : ang_err);
            pros::lcd::print(4, "Lat kP %.2f kD %.2f", latkP, latkD);
            pros::lcd::print(5, "Ang kP %.2f kD %.2f", angkP, angkD);
            if (last_settle < 0) {
                pros::lcd::print(6, "Last: TIMEOUT  overshoot %.2f", last_overshoot);
            } else {
                pros::lcd::print(6, "Last: %.0f ms  overshoot %.2f", last_settle, last_overshoot);
            }

            if (dirty && now - last_print >= 60) {
                controller.print(0, 0, "%s %s: %.2f      ",
                                pid_cal_lat ? "Lat" : "Ang",
                                pid_cal_kp ? "kP" : "kD",
                                pid_cal_lat ? (pid_cal_kp ? latkP : latkD) : (pid_cal_kp ? angkP : angkD));
                last_print = now;
                dirty = false;
            }

            pros::delay(10);
        }
}