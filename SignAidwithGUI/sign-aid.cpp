// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <Gestures.h>

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
bool onArm2;
myo::Arm whichArm2;
bool isUnlocked2;
bool lockState = true;
bool isDropped = false;
std::string gesture = "empty";


//GUI dependencies
using namespace System;
using namespace System::ComponentModel;
using namespace System::Threading;
using namespace System::Windows::Forms;
using namespace System::Collections;
using namespace System::Data;
using namespace System::Drawing;

namespace signaid_gui {

	//TTS Method
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
        onArm, onArm2 = false;
		isUnlocked, isUnlocked2 = false;
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

		if (isUnlocked, isUnlocked2 == true)
             myo->unlock(myo::Myo::unlockHold); //keep Myo unlocked after first unlock

		if (isUnlocked, isUnlocked2 == true && pose2 == myo::Pose::doubleTap){ //lock Myo again with double tap
			myo->lock();
		}
    }

    // onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
                   myo::WarmupState warmupState)
    {
        onArm, onArm2 = true;
		whichArm, whichArm2 = arm;
    }

    // onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
    {
        onArm, onArm2 = false;
    }

    // onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
    void onUnlock(myo::Myo* myo, uint64_t timestamp)
    {
		isUnlocked, isUnlocked2 = true;
    }

    // onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
    void onLock(myo::Myo* myo, uint64_t timestamp)
    {
		isUnlocked, isUnlocked2 = false;
    }

    // These values are set by onArmSync() and onArmUnsync() above.
	bool onArm;
    myo::Arm whichArm;

    // This is set by onUnlocked() and onLocked() above.
    bool isUnlocked;

    // These values are set by onOrientationData() and onPose() above.
    myo::Pose currentPose;
};

/*******************FORM*******************/
	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	// This delegate enables asynchronous calls for setting the text property on a label.
    delegate void SetTextDelegate(String^ text);

	  private:
        Thread^ thread1;

	public: System::Windows::Forms::Label^  label1; 
	private: System::Windows::Forms::Button^  button1; //start button

	//color control buttons
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button4;
	private: System::Windows::Forms::Button^  button5;
	private: System::Windows::Forms::Button^  button6;
	private: System::Windows::Forms::Button^  button7;
	private: System::Windows::Forms::Button^  button8;
	private: System::Windows::Forms::Button^  button9;
	
	private: System::Windows::Forms::Button^  button10;//color changer drop down button
	private: System::Windows::Forms::PictureBox^  pictureBox1; //title picture
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel1; //holds color buttons
	private: System::Windows::Forms::Timer^  timer1; //timer that prints "Listening..." a few seconds after a gesture is performed
	private: System::Windows::Forms::Timer^  timer2; //timer that controls color drop down menu
	private: System::ComponentModel::IContainer^  components;

	public:
	 // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

		// Print out the orientation
       std::cout << '[' << "ROLL: " << roll << ']'
                 << '[' << "PITCH: " << pitch << ']'
                 << '[' << "YAW: " << yaw << ']';
				 
		// Print out accelerometer data
		//std::cout << "[" << "X: " << xAccel << ']' 
			//	  << '[' << "Y: " << yAccel << ']'
				//  << '[' << "Z: " << zAccel << ']';

		// Print out gyroscope data
		//std::cout << "[" << "X: " << xGyro << ']'
				// << '[' << "Y: " << yGyro  << ']'
				//std::cout << '[' << "Z: " << zGyro  << ']';

	  //print lock state
	   if(isUnlocked2 == false){
		   lockState = false;
		   this->SetText("Locked");   
	   }
	  else if(lockState == false && isUnlocked2 == true && gesture.compare("empty") == 0){
		  lockState = true;
		  this->SetText("Ready"); //only prints this after unlocking
	  }

	   //get gesture
	   gesture = getGesture(pose2, roll, pitch, yaw, xAccel, yAccel, zAccel, xGyro, yGyro, zGyro); 

	   if(gesture.compare("empty") != 0){

			//print the gesture
			String^ gesture2 = gcnew String(gesture.c_str());
			this->SetText(gesture2);

			//speak the gesture
			std::wstring speak;
			speak.assign(gesture.begin(), gesture.end());
			TextToSpeak(speak);
	   }
	   
        if (onArm2) {
            // Print out the lock state, the currently recognized pose, and which arm Myo is being worn on.

            //std::string poseString = currentPose.toString();

            std::cout << '[' << (isUnlocked2 ? "unlocked" : "locked  ") << ']'
                      << '[' << (whichArm2 == myo::armLeft ? "L" : "R") << ']';
                
        } else {
            // Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
            std::cout << '[' << std::string(8, ' ') << ']' << "[?]" << '[' << std::string(14, ' ') << ']';
        }

        std::cout << std::flush;
    }

	private:
    void SetText(String^ text) //updates label 1 on the new thread launched by button1_click
    {
		
        // InvokeRequired required compares the thread ID of the
        // calling thread to the thread ID of the creating thread.
        // If these threads are different, it returns true.
        if (this->label1->InvokeRequired)
        {
            SetTextDelegate^ d = 
				gcnew SetTextDelegate(this, &MyForm::SetText);
            this->Invoke(d, gcnew array<Object^> { text });
        }
        else
        {
            this->label1->Text = text;
        }

		if(isUnlocked2 == true){
			timer1->Start(); 
		}
    }

	private:
    void button1_Click(Object^ sender, EventArgs^ e)
    {
		//launch a new thread
		this->thread1 =
			gcnew Thread(gcnew ThreadStart(this,&MyForm::ThreadProcSafe));
		this->thread1->Start();

		//make title picture not visible after launch
		this->pictureBox1->Visible = false;

		//make start button not visible after launch
		button1->Visible = false;
    }

	private:
    void ThreadProcSafe() //thread to run Myo commands
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

		this->SetText("Unable to find a Myo!");
        throw std::runtime_error("Unable to find a Myo!");
    }

    // We've found a Myo.
	this->SetText("Ready");
		
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
        print();
    }

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
		this->SetText("Unable to find a Myo");
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
    }

    }

#pragma region Windows Form Designer generated code

		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->button6 = (gcnew System::Windows::Forms::Button());
			this->button7 = (gcnew System::Windows::Forms::Button());
			this->button8 = (gcnew System::Windows::Forms::Button());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->button10 = (gcnew System::Windows::Forms::Button());
			
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid));
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();

			this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
			this->tableLayoutPanel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->Anchor = System::Windows::Forms::AnchorStyles::Top;
			pictureBox1->Image = Image::FromFile(String::Concat(System::Environment::GetFolderPath(System::Environment::SpecialFolder::Personal),"\\title.png"));
			pictureBox1->SizeMode = PictureBoxSizeMode::StretchImage;
			this->pictureBox1->Location = System::Drawing::Point((ClientSize.Width / 2) - (pictureBox1->Width /2), (ClientSize.Height / 2) - (pictureBox1->Height / 2));
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(515, 444);
			this->pictureBox1->TabIndex = 2;
			this->pictureBox1->TabStop = false;
			// 
			// label1
			// 
			this->label1->Font = (gcnew System::Drawing::Font(L"Arciform", 80, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Pixel, 
				static_cast<System::Byte>(0)));
			this->label1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->label1->ForeColor = System::Drawing::Color::White;
			this->label1->Location = System::Drawing::Point(12, 263);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(726, 180);
			this->label1->TabIndex = 0;
			this->label1->Text = L"";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// button1
			// 
			this->button1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(72)), static_cast<System::Int32>(static_cast<System::Byte>(79)), 
				static_cast<System::Int32>(static_cast<System::Byte>(89)));
			this->button1->Cursor = System::Windows::Forms::Cursors::Hand;
			this->button1->FlatAppearance->BorderSize = 0;
			this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button1->Font = (gcnew System::Drawing::Font(L"Arciform", 32, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Pixel, 
				static_cast<System::Byte>(0)));
			this->button1->Anchor = System::Windows::Forms::AnchorStyles::Bottom;
			this->button1->ForeColor = System::Drawing::Color::White;
			this->button1->Location = System::Drawing::Point(326, 557);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(150, 40);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Start";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button2->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(238)), static_cast<System::Int32>(static_cast<System::Byte>(107)), 
				static_cast<System::Int32>(static_cast<System::Byte>(73)));
			this->button2->FlatAppearance->BorderSize = 0;
			this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button2->Location = System::Drawing::Point(211, 3);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(98, 24);
			this->button2->TabIndex = 3;
			this->button2->UseVisualStyleBackColor = false;
			this->button2->Margin = System::Windows::Forms::Padding(0);
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// button3
			// 
			this->button3->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(198)), static_cast<System::Int32>(static_cast<System::Byte>(108)), 
				static_cast<System::Int32>(static_cast<System::Byte>(134)));
			this->button3->FlatAppearance->BorderSize = 0;
			this->button3->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button3->Location = System::Drawing::Point(315, 3);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(98, 24);
			this->button3->TabIndex = 4;
			this->button3->UseVisualStyleBackColor = false;
			this->button3->Margin = System::Windows::Forms::Padding(0);
			this->button3->Click += gcnew System::EventHandler(this, &MyForm::button3_Click);
			// 
			// button4
			// 
			this->button4->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button4->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(78)), static_cast<System::Int32>(static_cast<System::Byte>(53)), 
				static_cast<System::Int32>(static_cast<System::Byte>(75)));
			this->button4->FlatAppearance->BorderSize = 0;
			this->button4->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button4->Location = System::Drawing::Point(3, 3);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(98, 24);
			this->button4->TabIndex = 5;
			this->button4->UseVisualStyleBackColor = false;
			this->button4->Margin = System::Windows::Forms::Padding(0);
			this->button4->Click += gcnew System::EventHandler(this, &MyForm::button4_Click);
			// 
			// button5
			// 
			this->button5->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button5->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)), 
				static_cast<System::Int32>(static_cast<System::Byte>(73)));
			this->button5->FlatAppearance->BorderSize = 0;
			this->button5->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button5->Location = System::Drawing::Point(107, 3);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(98, 24);
			this->button5->TabIndex = 6;
			this->button5->UseVisualStyleBackColor = false;
			this->button5->Margin = System::Windows::Forms::Padding(0);
			this->button5->Click += gcnew System::EventHandler(this, &MyForm::button5_Click);
			// 
			// button6
			// 
			this->button6->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button6->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(110)), static_cast<System::Int32>(static_cast<System::Byte>(199)), 
				static_cast<System::Int32>(static_cast<System::Byte>(179)));
			this->button6->FlatAppearance->BorderSize = 0;
			this->button6->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button6->Location = System::Drawing::Point(627, 3);
			this->button6->Name = L"button6";
			this->button6->Size = System::Drawing::Size(98, 24);
			this->button6->TabIndex = 7;
			this->button6->UseVisualStyleBackColor = false;
			this->button6->Margin = System::Windows::Forms::Padding(0);
			this->button6->Click += gcnew System::EventHandler(this, &MyForm::button6_Click);
			// 
			// button7
			// 
			this->button7->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button7->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(82)), static_cast<System::Int32>(static_cast<System::Byte>(200)), 
				static_cast<System::Int32>(static_cast<System::Byte>(122)));
			this->button7->FlatAppearance->BorderSize = 0;
			this->button7->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button7->Location = System::Drawing::Point(731, 3);
			this->button7->Name = L"button7";
			this->button7->Size = System::Drawing::Size(103, 24);
			this->button7->TabIndex = 8;
			this->button7->UseVisualStyleBackColor = false;
			this->button7->Margin = System::Windows::Forms::Padding(0);
			this->button7->Click += gcnew System::EventHandler(this, &MyForm::button7_Click);
			// 
			// button8
			// 
			this->button8->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button8->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(109)), static_cast<System::Int32>(static_cast<System::Byte>(92)), 
				static_cast<System::Int32>(static_cast<System::Byte>(125)));
			this->button8->FlatAppearance->BorderSize = 0;
			this->button8->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button8->Location = System::Drawing::Point(419, 3);
			this->button8->Name = L"button8";
			this->button8->Size = System::Drawing::Size(98, 24);
			this->button8->TabIndex = 9;
			this->button8->UseVisualStyleBackColor = false;
			this->button8->Margin = System::Windows::Forms::Padding(0);
			this->button8->Click += gcnew System::EventHandler(this, &MyForm::button8_Click);
			// 
			// button9
			// 
			this->button9->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->button9->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(123)), 
				static_cast<System::Int32>(static_cast<System::Byte>(131)));
			this->button9->FlatAppearance->BorderSize = 0;
			this->button9->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button9->Location = System::Drawing::Point(523, 3);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(98, 24);
			this->button9->TabIndex = 10;
			this->button9->UseVisualStyleBackColor = false;
			this->button9->Margin = System::Windows::Forms::Padding(0);
			this->button9->Click += gcnew System::EventHandler(this, &MyForm::button9_Click);
			// 
			// button10
			// 
			this->button10->Cursor = System::Windows::Forms::Cursors::Hand;
			this->button10->BackgroundImage =  Image::FromFile(String::Concat(System::Environment::GetFolderPath(System::Environment::SpecialFolder::Personal),"\\color.png"));
			this->button10->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
			this->button10->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			this->button10->FlatAppearance->BorderSize = 0;
			this->button10->Location = System::Drawing::Point(12, 12);
			this->button10->Name = L"button10";
			this->button10->Size = System::Drawing::Size(25, 30);
			this->button10->TabIndex = 3;
			this->button10->Text = L"";
			this->button10->UseVisualStyleBackColor = true;
			this->button10->Click += gcnew System::EventHandler(this, &MyForm::button10_Click);
			// 
			// tableLayoutPanel1
			// 
			this->tableLayoutPanel1->BackColor = System::Drawing::Color::White;
			this->tableLayoutPanel1->ColumnCount = 8;
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent, 
				12.5F)));
			this->tableLayoutPanel1->Controls->Add(this->button9, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button8, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button7, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button6, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button5, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button4, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button3, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->button2, 0, 0);
			this->tableLayoutPanel1->Dock = System::Windows::Forms::DockStyle::Top;
			this->tableLayoutPanel1->Location = System::Drawing::Point(0, 0);
			this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
			this->tableLayoutPanel1->RowCount = 1;
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 100)));
			this->tableLayoutPanel1->Size = System::Drawing::Size(562, 0);
			this->tableLayoutPanel1->TabIndex = 12;
			// 
			// timer1
			// 
			this->timer1->Interval = 5000;
			this->timer1->Tick += gcnew System::EventHandler(this, &MyForm::timer1_Tick);
			// 
			// timer2
			// 
			this->timer2->Tick += gcnew System::EventHandler(this, &MyForm::timer2_Tick);
			// 
			// 
			// MyForm
			// 
			this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(110)), static_cast<System::Int32>(static_cast<System::Byte>(199)), 
				static_cast<System::Int32>(static_cast<System::Byte>(179)));
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(750, 640);
			this->MinimumSize = System::Drawing::Size(750, 650);	
			this->Controls->Add(this->button10);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->tableLayoutPanel1);	
			this->Controls->Add(this->label1);
			this->Name = L"MyForm";
			this->Text = L"Sign Aid - Sign Language Translator";
			this->tableLayoutPanel1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion

		private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) {
					 timer1->Stop();	
					 this->label1->Text = "Listening...";
		}

		private: System::Void timer2_Tick(System::Object^  sender, System::EventArgs^  e) {
					 if(isDropped == false){
						this->tableLayoutPanel1->Height += 5;	
						this->button10->Top += 5;
					 }
					 if(isDropped == true){
						 this->tableLayoutPanel1->Height -= 5;
						 this->button10->Top -= 5;
					 }

					 if( this->tableLayoutPanel1->Height == 25 && isDropped == false){
						 timer2->Stop();	
						 isDropped = true;
					 }

					 if( this->tableLayoutPanel1->Height == 0 && isDropped == true){
						 timer2->Stop();	
						 isDropped = false;
					 }
		}

		private: System::Void button10_Click(System::Object^  sender, System::EventArgs^  e) {
					 timer2->Start();

		 }

		private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(238)), static_cast<System::Int32>(static_cast<System::Byte>(107)), 
				static_cast<System::Int32>(static_cast<System::Byte>(73)));

		 }
		private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(198)), static_cast<System::Int32>(static_cast<System::Byte>(108)), 
				static_cast<System::Int32>(static_cast<System::Byte>(134)));
		 }	
		private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(78)), static_cast<System::Int32>(static_cast<System::Byte>(53)), 
				static_cast<System::Int32>(static_cast<System::Byte>(75)));
		 }
		private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(255)), static_cast<System::Int32>(static_cast<System::Byte>(188)), 
				static_cast<System::Int32>(static_cast<System::Byte>(73)));

		 }
		private: System::Void button6_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(110)), static_cast<System::Int32>(static_cast<System::Byte>(199)), 
				static_cast<System::Int32>(static_cast<System::Byte>(179)));
		 }
		private: System::Void button7_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(82)), static_cast<System::Int32>(static_cast<System::Byte>(200)), 
				static_cast<System::Int32>(static_cast<System::Byte>(122)));
		 }
		private: System::Void button8_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(109)), static_cast<System::Int32>(static_cast<System::Byte>(92)), 
				static_cast<System::Int32>(static_cast<System::Byte>(125)));
		 }
		private: System::Void button9_Click(System::Object^  sender, System::EventArgs^  e) {
					 this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(123)), 
				static_cast<System::Int32>(static_cast<System::Byte>(131)));
		 }

	};
}

[STAThread]
int main(array<System::String ^> ^args)
{
	 Application::EnableVisualStyles();
	 Application::Run(gcnew signaid_gui::MyForm());
}