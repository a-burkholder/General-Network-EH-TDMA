

/* Constants and assumptions*/
const int TOTAL_NODES = 5;                                  // Total number of nodes in the network
const int TIME_SLOT = 500;                                  // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long CYCLE_LENGTH = TOTAL_NODES*TIME_SLOT;   // total length of one cycle
const int ERROR = 60;                                       // Transmission time error threshold
const String ID = "04";                                     // Each node knows its ID based on assumption
const int ENERGY_CHANCE = 80;                               // energy harvest rate



/* FLAGS... and stuff*/
bool led_state = false;   // unused
bool trans = false;       // unused
bool updated = false;     // tracks if we need to read a time for syncing or if we already did that
bool is_sync = false;     // keeps track of if the last read message was a sync


/* Timers */
unsigned long transmit_time;    // time in the cycle to transmit
unsigned long global_time = 0;  // global cycle time from the last sync message
long offset = 0;                // offset from the node's cycle to the global cycle
unsigned long start_clock = 0;  // the current node's equivalent time to the global cycle time
unsigned long last_time;        // the time at the last time it was checked


/* Transmition stuff */
String incoming = "Placeholder data input string";  // holds whole input
String data_in = "Placeholder data";                // the data coming in
String time_sent = "1500";                          // the global cycle time coming in
unsigned long time_in = "1500";                     // time that this node received the message, ideally same as time_sent

bool is_sent = false;   // checks if a message was sent this cycle
int clock_diff;         // used for calculating overlap between nodes
int overlap_flag;       // to hold the overlap info: 1 for behind, 2 for ok, 3 for ahead
bool is_overlap;        // to send the state mechine to SYNC_LIST if true

String data[TOTAL_NODES];

void baseFSM();
bool readData();
unsigned long cycleTime();

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
  static enum { SYNC_ALL, ACTIVE, SYNC_LIST } state = SYNC_ALL;
  switch(state){
    case SYNC_ALL:
      int cycle_Time = millis() % CYCLE_LENGTH;
      Serial.println("G" + cycle_Time);// send a general sync message 
      state = ACTIVE;
      break;

    case ACTIVE:
      // read data
      if(readData()){
        //--check for overlap errors in most recent message--//
        clock_diff = ((String)time_in).compareTo(time_sent); 
        if(clock_diff > ERROR){ // behind
          overlap_flag = 1;
        }
        else if (clock_diff > -ERROR){ // ok
          overlap_flag = 2;
        }
        else { // ahead
          overlap_flag = 3; 
        }
        data[0] = overlap_flag;
        if(overlap_flag != "2"){
          is_overlap = true;
        }
      }
      // check for errors
      // parse the data and check for any overlapp errors (1 behind, 2 ok, 3 ahead)
      for(int i = 0, i < (TOTAL_NODES-1)*3, i=i+3){
        data[i+1] = data_in.substring(i,i+3);
        if(data[i+1] != "2"){
          is_overlap = true;
        }
      }
      
      // if overlap and send SYNC_LIST to adjust
      if(is_overlap){
        state = SYNC_LIST;
      }
        
        // if all nodes dead, SYNCA
      break;
      
    case SYNC_LIST:
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

// cycleTime() -- Verified working
// helper function to keep the time in the range of one cycle and incorporate the offset
// also resets the is_sent variable so we can send a new message if we get to a new cycle
unsigned long cycleTime(){
  unsigned long time = (millis() + offset) % CYCLE_LENGTH;
  if(last_time > time){ // checks if the clock reset and resets is_sent
    is_sent = false;
  }
  last_time = time; // for next time we call the function
  return time;
}

