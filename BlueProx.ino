#include <nRF5x_BLE_API.h>
BLE           ble;

//Prototype Functions
bool pCompare(uint8_t *p1, uint8_t *p2, int plength);
void triplePulse(int pin);
void doublePulse(int pin);
void singlePulse(int pin);
void lock();
void unlock();
void unlatchTrunk();
void scanCallBack(const Gap::AdvertisementCallbackParams_t *params);
 
//Global Varibles
const int lockoutRSSI = -80; //RSSI to lockout veichle in dB
const int timeoutLimit = 300; //Number of times code runs before it locks
int lockOut = 1000; //Number of times code runs before the speical command can be sent again
uint8_t KeyName[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Known UID Frame (Namespace), see https://github.com/google/eddystone
uint8_t KeyInstance[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Known UID Frame (Instance)
uint8_t KeyInstance2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Known UID Frame (Instance), 2nd instance is optional
const int pinLock = D2; //Digital pins on nRF52, see  https://github.com/redbear/nRF5x/tree/master/nRF52832
const int pinUnlock = D3; //Digital pins on nRF52, see  https://github.com/redbear/nRF5x/tree/master/nRF52832
const int pinTrunk = D4; //Digital pins on nRF52, see  https://github.com/redbear/nRF5x/tree/master/nRF52832
const int delayTime = 150; //Time in milliseconds between pulses
const int singleDelayTime = 2000; //Time in milliseconds for the single pulse commmand

//Constant Varibles
uint8_t knownName[11]; //Size of the UID Namespace Frame 
uint8_t knownInstance[7]; //Size of the UID Instance Frame
uint8_t EddystonePrefix[] = {0xAA, 0xFE, 0x00}; //This is an Eddystone-UID beacon prefix
const int eLength = 3; //Eddystone-UID prefix length.
const int nLength = 10; //UID Namespace Frame  length.
const int iLength = 6; //UID Instance Frame  UID length.
int timeoutCount = 0; // Initalize lockout counter at zero
bool lockState = false; // lockState indicates the car's locked state
                        //Intialize as though the car is unlocked, if no device is detected as timeoutCount approaches timeoutLimit, the car will lock
bool sameval;
int lockOut0 = lockOut;

void setup() 
{
  //Serial.begin(9600);
  ble.init();
  ble.setScanParams(1000, 1000, 0, false);
  ble.startScan(scanCallBack);

  //Initialize digital pins as outputs
  pinMode(D13,OUTPUT);
  pinMode(pinLock,OUTPUT);
  pinMode(pinUnlock,OUTPUT);
  pinMode(pinTrunk,OUTPUT);

  //Initialize digital pins as LOW to prevent activations when booting
  digitalWrite(pinLock,LOW);
  digitalWrite(pinUnlock,LOW);
  digitalWrite(pinTrunk,LOW);
}

void loop() 
{
  ble.waitForEvent();

  // Start counting down lockOut to enable unlatchTrunk funtion
  if (lockOut>0)
  { 
    lockOut--;
  }

  // Start counting up timoutCount to lock car in next loop
  if (timeoutCount<timeoutLimit)
  { 
    timeoutCount++;
  }
    
  //If car is unlocked and the timeout counter is reached, then lock the car
  if (lockState == false && timeoutCount == timeoutLimit)
  { 
    lock();
  }
}

/**
 * @brief  Callback handle for scanning device
 *
 * @param[in]  *params   params->peerAddr            The peer's BLE address
 *                       params->rssi                The advertisement packet RSSI value
 *                       params->isScanResponse      Whether this packet is the response to a scan request
 *                       params->type                The type of advertisement
 *                                                   (enum from 0 ADV_CONNECTABLE_UNDIRECTED,ADV_CONNECTABLE_DIRECTED,ADV_SCANNABLE_UNDIRECTED,ADV_NON_CONNECTABLE_UNDIRECTED)
 *                       params->advertisingDataLen  Length of the advertisement data
 *                       params->advertisingData     Pointer to the advertisement packet's data
 */

void scanCallBack(const Gap::AdvertisementCallbackParams_t *params) 
{
  uint8_t index;
  // Eddystone Parser  
  if (params->advertisingDataLen == 26) // need at least 20 bytes for Eddystone-UID, see https://github.com/google/eddystone/tree/master/eddystone-uid but my advertising data is 26 characters
  { 
    for (index = 0; index < params->advertisingDataLen; index++)
    { 
        // Determine if Eddystone prefix exits.
        if (params->advertisingData[index] == 0xAA && params->advertisingData[index+1] == 0xFE && params->advertisingData[index+2] == 0x00)
        {
           uint8_t *UIDName = const_cast<uint8_t*>(params->advertisingData+index+4); //Store UID Namespace
           uint8_t *UIDInstance = const_cast<uint8_t*>(params->advertisingData+index+14); //Store UID Instance

          //Validate that both UID Namespace and Instance are the same as the known 
          if (pCompare(KeyName, UIDName, nLength) == true && pCompare(KeyInstance, UIDInstance, iLength) == true && lockoutRSSI < params->rssi)
          { 
            timeoutCount = 0;
            //Car must be locked to be unlocked, otherwise unlock command will be sent everytime this loop runs
            if (lockState == true)
            { 
                unlock();
            }
           }

           //Validate that both UID Namespace and Instance are the same as the known (Second Operation)
           else if (pCompare(KeyName, UIDName, nLength) == true && pCompare(KeyInstance2, UIDInstance, iLength) == true && lockOut == 0)
           { 
            unlatchTrunk();
           }
         }      
      }
   }
}

//Compare Pointers, return boolean with result
bool pCompare(uint8_t *p1, uint8_t *p2, int plength)
{
 for (int i = 0; i < plength; i++)
 {
   if (p1[i] == p2[i])
   {
      sameval = true;
   }
   else 
   {
      sameval = false;
      break;
   }
 }
 return sameval;   
}

//Pulse Pins
void triplePulse(int pin)
{
  for(int i=0; i<3; i++)
  {
    digitalWrite(pin, HIGH);
    delay(delayTime); 
    digitalWrite(pin, LOW);
    delay(delayTime);          
  }   
}

void doublePulse(int pin)
{
  for(int i=0; i<2; i++)
  {
    digitalWrite(pin, HIGH);
    delay(delayTime); 
    digitalWrite(pin, LOW);
    delay(delayTime);          
  }   
}

void singlePulse(int pin)
{
    digitalWrite(pin, HIGH);  
    delay(singleDelayTime);               
    digitalWrite(pin, LOW);    
}

//Locking Funtion
void lock() 
{
  digitalWrite(D13, LOW); 
  triplePulse(pinLock);    //Car needs to get lock command three times to honk 
  lockState = true;
}

//Unlocking Funtion
void unlock() 
{
  digitalWrite(D13, HIGH); 
  doublePulse(pinUnlock); 
  lockState = false;   
  timeoutCount = 0;  
}

//Trunk Unlatching Funtion
void unlatchTrunk() 
{
  singlePulse(pinTrunk); 
  lockOut = lockOut0;
}
