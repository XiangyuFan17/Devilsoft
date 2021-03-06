// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

//EMG dependencies
#include <array>

//MessageBox dependencies
#include <Windows.h>
#pragma comment(lib,"User32.lib")

//TTS dependencies
#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <sapi.h>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

//forward declarations
void TextToSpeak(std::wstring phrase);

//global variables
float roll, pitch, yaw = 0;
float xAccel, yAccel, zAccel = 0;
float xGyro, yGyro, zGyro = 0;
myo::Pose pose2;
bool wavedIn, wavedOut = false; //for excuse me gesture
bool circle1, circle2 = false; //for circle gesture

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : onArm(false), isUnlocked(false), currentPose()
    {
    }

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        onArm = false;
        isUnlocked = false;
    }

    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        using std::max;
        using std::min;

        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
        yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                        1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
    }

	// onAccelerometerData is called whenever new acceleromenter data is provided
	void onAccelerometerData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &accel) {

		xAccel = accel.x();
		yAccel = accel.y();
		zAccel = accel.z();
	
	}

	// onGyroscopeData is called whenever new gyroscope data is provided
	void onGyroscopeData(myo::Myo *myo, uint64_t timestamp, const myo::Vector3< float > &gyro) {

		xGyro = gyro.x();
		yGyro = gyro.y();
		zGyro = gyro.z();
		
	}

    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose, pose2 = pose;

		if (isUnlocked == true)
             myo->unlock(myo::Myo::unlockHold); //keep Myo unlocked after first unlock

		if (isUnlocked == true && pose2 == myo::Pose::doubleTap){ //lock Myo again with double tap
			myo->lock();
		}
    }

    // onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
                   myo::WarmupState warmupState)
    {
        onArm = true;
        whichArm = arm;
    }

    // onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }

    // onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
    void onUnlock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = true;
    }

    // onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
    void onLock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = false;
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

		// Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
       std::cout << '[' << "ROLL: " << roll << ']'
                 << '[' << "PITCH: " << pitch << ']'
                 << '[' << "YAW: " << yaw << ']' << std::endl;
				 

		// Print out accelerometer data
		//std::cout << "[" << " ACCEL X: " << xAccel << ']'
			//	  << '[' << "ACCEL Y: " << yAccel << ']' 
				//  << '[' << "ACCEL Z: " << zAccel << ']' << std::endl;

		// Print out gyroscope data
		//std::cout << "[" << "GYRO X: " << xGyro << ']'
			//	 << '[' << "GYRO Y: " << yGyro  << ']'
				// << '[' << "GYRO Z: " << zGyro  << ']' << std::endl;
	  
	   /*****Back*****/
	   if (pose2 == myo::Pose::waveIn && zAccel < 0){
			TextToSpeak(L"Back");
			MessageBox(0, "Back", "Gesture", MB_OK);
		}
	   
	   /*****Circle*****/
		if (zGyro > 220)
			circle1 = true;
		else if (zGyro < -220)
			circle2 = true;
		if (circle1 == true && circle2 == true){
			TextToSpeak(L"Circle");
			MessageBox(0, "Circle", "Gesture", MB_OK);
			circle1 = false;
			circle2 = false;
		}

		/*****Hello*****/
		if (pose2 == myo::Pose::fingersSpread && pitch >= 1.1 && pitch <= 1.3 ){
			TextToSpeak(L"Hello");
			MessageBox(0, "Hello", "Gesture", MB_OK);
		}

		/*****Goodbye*****/
		if (pose2 == myo::Pose::fingersSpread && pitch >= -0.8 && pitch <= -0.4 ){
			TextToSpeak(L"Goodbye");
			MessageBox(0, "Goodbye", "Gesture", MB_OK);
		}

		/*****Help*****/
		if (pose2 == myo::Pose::fist && pitch >= 0.4 && pitch <= 0.7){
			TextToSpeak(L"Help");
			MessageBox(0, "Help", "Gesture", MB_OK);
		}

		/*****Excuse Me*****/
		if (pose2 == myo::Pose::waveIn)
			wavedIn = true;
		else if (pose2 == myo::Pose::waveOut)
			wavedOut = true;
		if (wavedIn == true && wavedOut == true){
			TextToSpeak(L"Excuse Me");
			MessageBox(0, "Excuse Me", "Gesture", MB_OK);
			wavedIn = false;
			wavedOut = false;
		}
		if (isUnlocked == false){
			wavedIn = false;
			wavedOut = false;
		}

        if (onArm) {
            // Print out the lock state, the currently recognized pose, and which arm Myo is being worn on.

            // Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
            // output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
            // that we can fill the rest of the field with spaces below, so we obtain it as a string using toString().
            std::string poseString = currentPose.toString();

            std::cout << '[' << (isUnlocked ? "unlocked" : "locked  ") << ']'
                      << '[' << (whichArm == myo::armLeft ? "L" : "R") << ']';
                      // '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
        } else {
            // Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
            std::cout << '[' << std::string(8, ' ') << ']' << "[?]" << '[' << std::string(14, ' ') << ']';
        }

        std::cout << std::flush;
    }

    // These values are set by onArmSync() and onArmUnsync() above.
    bool onArm;
    myo::Arm whichArm;

    // This is set by onUnlocked() and onLocked() above.
    bool isUnlocked;

    // These values are set by onOrientationData() and onPose() above.
    myo::Pose currentPose;
};

void TextToSpeak(std::wstring phrase){
	
	ISpVoice * pVoice = NULL;

    if (FAILED(::CoInitialize(NULL)))
        std::cout << "Text to Speak Failed";

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if( SUCCEEDED( hr ) )
    {
		hr = pVoice->Speak(phrase.c_str(), 0, NULL);
        pVoice->Release();
        pVoice = NULL;
    }

    ::CoUninitialize();

}

int main(int argc, char** argv)
{

    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.hello-myo");

    std::cout << "Attempting to find a Myo..." << std::endl;

    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
    // if that fails, the function will return a null pointer.
    myo::Myo* myo = hub.waitForMyo(10000);

    // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        throw std::runtime_error("Unable to find a Myo!");
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
	
	// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
    DataCollector collector;

    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);

    // Finally we enter our main loop.
    while (1) {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
        hub.run(1000/20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        collector.print();
    }

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}
