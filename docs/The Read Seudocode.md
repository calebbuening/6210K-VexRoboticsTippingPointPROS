The Actual Seudocode for the Robot of This Year

Things to consider:
* Currently, if the robot brain looses connection during the auto, it is all over. Is there and easy way to fix this?
* We need to be able to easily implement any of my crazy ideas if they work

Parts of the robot:
* Drivetrain **via two Okapi motor groups in a chassis** (6 motors)
* Ring intake **via a Okapi motor** (1 motor)
* Ring director **via a Okapi motor** (1 motor)
* Pneumatics for both mogo grabbers **via PROS ADI Digital Out** (2 pneumatics)

Seudocode:
* [ ] (exparimental) Assign following values via NN

* [ ] Driving code
* [ ] Lift code

* [ ] Print to brain code
* [ ] Print to controller code

* [x] Data Logging code
* [ ] Error Logging code
* [x] Copycat logging code

Driving code: Split stick

L:100, R:100  >  100: 0
L:100, R:-100 >  0  : 100
L:100, R:0    >  100: 100
L:0  , R:100  >  100:-100
L:0  , R:-100 > -100: 100

LW = L + R
RW = L - R