
//* Constants and assumptions*/
const int TOTAL_NODES = 5; // Total number of nodes in the network
const int TIME_SLOT = 500; // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long CYCLE_LENGTH = TOTAL_NODES*TIME_SLOT; // total length of one cycle
const int ERROR = 60; // Transmission time error threshold
const String ID = "04"; // Each node knows its ID based on assumption
const int ENERGY_CHANCE = 80; // energy harvest rate



/* FLAGS... and stuff*/
bool led_state = false;
bool trans = false;
bool updated = false; // tracks if we need to read a time for syncing or if we already did that
bool is_sync = false; // keeps track of if the last read message was a sync


/* Timers */
unsigned long transmit_time; // time in the cycle to transmit
unsigned long global_time = 0; // global cycle time from the last sync message
long offset = 0; // offset from the node's cycle to the global cycle

unsigned long start_clock = 0; // the current node's equivalent time to the global cycle time


unsigned long sync_time = 0; // unused
unsigned long current_time = 0; // unused


/* Transmition stuff */
String incoming = ""; // holds whole input
String data_in = ""; // the data coming in
String time_in = ""; // the global cycle time coming in


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

}

// baseFSM()
// The function that implements the states and their functionality in the base
void baseFSM(){
  static enum { SYNCA, ACTIVE, SYNC1 } state = SYNCA;
  switch(state){
    case SYNCA:
      int cycle_Time = millis() % CYCLE_LENGTH;
      Serial.println("G" + cycle_Time);// send a general sync message 
      state = ACTIVE;
      break;

    case ACTIVE:
      // read data
      // check for errors
        // if overlap and send SYNC1 to adjust
        // if all nodes dead, SYNCA
      break;
      
    case SYNC1:
      // send a sync to specific nodes
      // to ACTIVE
      break;

  }
}

// readData()
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
// If it reads data, returns true
bool readData(){
  bool isMessage = false;

  //--Read the data--//
  if(Serial.available()){
    incoming = Serial.readStringUntil('\r'); 
    isMessage = true;

    int data_idx = incoming.indexOf(',',2); //the index of the comma separating the data from the cycle time
    time_in = incoming.substring(1,data_idx);
    data_in = incoming.substring(data_idx + 1, incoming.length());
    
  }
  Serial.flush();
  return isMessage;
}

