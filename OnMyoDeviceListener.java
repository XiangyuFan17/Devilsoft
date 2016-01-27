import com.thalmic.myo.DeviceListener;
import com.thalmic.myo.FirmwareVersion;
import com.thalmic.myo.Myo;
import com.thalmic.myo.Pose;
import com.thalmic.myo.Quaternion;
import com.thalmic.myo.Vector3;
import com.thalmic.myo.enums.Arm;
import com.thalmic.myo.enums.WarmupResult;
import com.thalmic.myo.enums.WarmupState;
import com.thalmic.myo.enums.XDirection;
public class OnMyoDeviceListener implements DeviceListener {

 @Override
 public void onAccelerometerData(Myo arg0, long arg1, Vector3 arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onArmSync(Myo arg0, long arg1, Arm arg2, XDirection arg3, float arg4, WarmupState arg5) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onArmUnsync(Myo arg0, long arg1) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onBatteryLevelReceived(Myo arg0, long arg1, int arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onConnect(Myo arg0, long arg1, FirmwareVersion arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onDisconnect(Myo arg0, long arg1) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onEmgData(Myo arg0, long arg1, byte[] arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onGyroscopeData(Myo arg0, long arg1, Vector3 arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onLock(Myo arg0, long arg1) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onOrientationData(Myo arg0, long arg1, Quaternion arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onPair(Myo arg0, long arg1, FirmwareVersion arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onPose(Myo myo, long timestamp, Pose pose) {
 	System.out.println(String.format("Myo switched to pose %s.", pose.toString()));
 	 
 }

 @Override
 public void onRssi(Myo arg0, long arg1, int arg2) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onUnlock(Myo arg0, long arg1) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onUnpair(Myo arg0, long arg1) {
 	// TODO Auto-generated method stub
 	 
 }

 @Override
 public void onWarmupCompleted(Myo arg0, long arg1, WarmupResult arg2) {
 	// TODO Auto-generated method stub
 	 
 }

}