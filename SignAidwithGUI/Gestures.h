/******This file contains all of the currently implemented gestures*****/

#include <string>
#include <myo\myo.hpp>
using std::string;

bool wavedIn, wavedOut = false; //for excuse me gesture
bool circle1, circle2 = false; //for circle gesture

std::string getGesture(myo::Pose pose, float roll, float pitch, float yaw,
					   float xAccel, float yAccel, float zAccel,
					   float xGyro, float yGyro, float zGyro){
		

		/*****Thank you*****/
		 if (pose == myo::Pose::waveIn && zAccel < 0){
		   return "Thank you!";
		}

		/*****Circle*****/
		if (zGyro > 220)
			circle1 = true;
		else if (zGyro < -220)
			circle2 = true;
		if (circle1 == true && circle2 == true){
			circle1 = false;
			circle2 = false;
			return "Circle";
		}

		/*****Excuse Me*****/
		if (pose == myo::Pose::waveIn)
			wavedIn = true;
		else if (pose == myo::Pose::waveOut)
			wavedOut = true;
		if (wavedIn == true && wavedOut == true){
			wavedIn = false;
			wavedOut = false;
			return "Excuse Me";
		}
	
		/*****Goodbye*****/
		if (pose == myo::Pose::fingersSpread && pitch >= -0.8 && pitch <= -0.4 ){
			return "Goodbye";
		}

		/*****Hello*****/
		if (pose == myo::Pose::fingersSpread && pitch >= 1.1 && pitch <= 1.3 ){
			return "Hello";
		}
		
		/*****Help*****/
		if (pose == myo::Pose::fist && pitch >= 0.4 && pitch <= 0.7){
			return "Help";	
		}

		return "empty";

}