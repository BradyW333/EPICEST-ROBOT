# ---------------------------------------------------------------------------- #
#                                                                              #
# 	Module:       main.py                                                      #
# 	Author:       jvito                                                        #
# 	Created:      9/15/2026, 1:03:55 PM                                        #
# 	Description:  V5 project                                                   #
#                                                                              #
# ---------------------------------------------------------------------------- #

# Library imports
from vex import *

brain = Brain()
right_motor = Motor(Ports.PORT13)
left_motor = Motor(Ports.PORT15)
display = brain.screen
vision = AiVision(Ports.PORT17,AiVision.ALL_TAGS)

dt = DriveTrain(left_motor,right_motor)

def autonomous():
    brain.screen.clear_screen()
    brain.screen.print("autonomous code")
    # place automonous code here

def vision_tick():
    tags = vision.take_snapshot(AiVision.ALL_TAGS,20)
    display.clear_screen()
    display.set_cursor(0,0)
    for tag in tags:
        display.print(str(tag.id) + " " + str(tag.centerX) + " " + str(tag.centerY) + " " + str(tag.width) + " " + str(tag.height))
        display.new_line()
    

def user_control():
    brain.screen.clear_screen()
    brain.screen.print("driver control")
    # place driver control in this while loop
    while True:
        wait(20, MSEC)

# create competition instance
#comp = Competition(user_control, autonomous)

# actions to do when the program starts
brain.screen.clear_screen()
print("project run")
while True:
    vision_tick()