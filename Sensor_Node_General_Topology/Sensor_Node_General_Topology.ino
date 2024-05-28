

/* Constants and assumptions*/
const String ZONE = "1";
const int TOTAL_NODES = 2;                                 // Total number of sensor nodes in the network
const int TIME_SLOT = 300;                                  // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long ERROR = 60;                             // Transmission time error threshold
const String ID = "01";                                     // Each node knows its ID based on assumption
const int ENERGY_CHANCE = 100;                             // energy harvest rate
const unsigned long CYCLE_LENGTH = (TOTAL_NODES+1)*TIME_SLOT;   // total length of one cycle
unsigned long TRANSMIT_TIME = (ID.toInt()) * TIME_SLOT; // time in the cycle to transmit TRANSMIT_TIME

/* FLAGS... and stuff*/
bool updated = false;       // tracks if we need to read a time for syncing or if we already did that
bool is_sync = false;       // keeps track of if the last read message was a sync
bool is_sent = false;       // checks if a message was sent this cycle
bool overlap_check = false; // we only check overlap if it gets a data packet

/* Timers */
long offset = 0;                    // offset from the node's cycle to the global cycle
unsigned long last_time = 0;        // the time at the last time it was checked
long global_time = 0;               // the time the previous node sent the message
unsigned long time_in = 0;          // time that this node received the message
unsigned long time_in_U = 0;        // time_in but updated to global time

/* Transmition stuff */
String data_in = ",E,";                // the data coming in

/* Function headers */
void nodeFSM();
bool energyAvailible();
bool readData();
unsigned long cycleTime();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(30);
}

void loop() {
  // put your main code here, to run repeatedly:
  nodeFSM();
  
}

// nodeFSM()
// The function that implements the states and their functionality in the nodes
void nodeFSM(){
  static enum { DEAD, SYNC, WAIT, ACTIVE } state = DEAD;
  switch (state) {
    case DEAD: // -- Verified working
      if(energyAvailible()) {
        //--reset flags--//
        updated = false;
        is_sync = false;
        is_sent = false;
        overlap_check = false;
        //--reset timers--//
        offset = 0;
        last_time = 0;
        global_time = 0;
        time_in = 0;
        time_in_U = 0;
        //--reset data--//
        data_in = ",E,";                
        // to sync
        state = SYNC;
        Serial.println("SYNC,");
      }
      break;
    
    case SYNC: // -- Verified working
      //--for if we are waiting for a message to get time from--//

      if(!updated && readData()){
        is_sent = false;
      }
      
      //--for if we have the time to sync off of--//
      if(updated){
        offset = global_time - time_in; 
        time_in_U = (time_in + offset) % CYCLE_LENGTH;
        state = WAIT;
        updated = false; // reset flag
      }
      break;

    case WAIT: // -- Verified working
      //--if in time slot--//
      if(cycleTime() == TRANSMIT_TIME && !is_sent){ 
        state = ACTIVE; 
      }
      
      //--if not in time slot--//
      else if(readData()){
        time_in_U = cycleTime();
        if (is_sync){
          state = SYNC;
          Serial.println("SYNC1,");
          break;
        }
      }
      break;

    case ACTIVE: // -- Verified working
      //--check for overlap errors--//
      int clock_diff =  time_in_U - global_time; 
      int is_overlap = 2; // base case, all good
      if(overlap_check){
        if(clock_diff > ERROR){ is_overlap = 1; }        // behind
        else if (clock_diff > -ERROR){ is_overlap = 3; } // ahead
      }
      //--make and send the data--//
      String message = "D," + (String)cycleTime() + "," + ID + (String)is_overlap + data_in;
      Serial.println(message);
      //--reset--//
      data_in = ",E,";
      is_sent = true;
      //--energy checking--//
      if(energyAvailible()){
        state = WAIT;
        break;
      }
      else {
        state = DEAD;
        break;
      }
      break;
  }
}

// energyAvailible() // -- Verified working
// RNG for if a node has energy or not, based on the chances of that node having energy
bool energyAvailible(){
  if(random(0,100) <= ENERGY_CHANCE){
    return true;
  } 
  else return false;
}

// readData() // -- Verified working
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
// If it reads data, returns true
// Test messages A,G,1234    1,D,1234,012,E,     A,S,1234,02,0102
bool readData(){
  if(Serial.available() > 0){
    String zone = Serial.readStringUntil(',');
    if(zone == "A" || zone == ZONE){
      updated = true;
      time_in = millis() % CYCLE_LENGTH;          // grabs the nodal time of receiving
      String type1 = Serial.readStringUntil(','); // grabs the type of message
      global_time = Serial.parseInt();            // grabs the global time from sender
      
      if(type1 == "D") {
        data_in = Serial.readStringUntil('\n');
        overlap_check = true;
        is_sync = false;
      }
      
      else if(type1 == "S") {
        long num_syncs = Serial.parseInt();
        String sync_list = Serial.readString();
        overlap_check = false;
          
        //--linear search through all node IDs--//   REPLACE WITH BINARY SEARCH EVENTUALLY
        for(int i = 1; i <= num_syncs; i++){
          String to_check = sync_list.substring((i*2-1), (i*2)+1);
          if(to_check == ID){ //if this node finds it's ID on the sync list
            is_sync = true;
            break;
          }
        }
      }

      else if(type1 == "G"){
        is_sync = true;
        overlap_check = false;
      }
      
      while(Serial.available() > 0){
        Serial.read(); // clears input buffer
      }
      return true; // if theres a message
    }
    while(Serial.available() > 0){
      Serial.read(); // clears input buffer
    }
  }
   
  return false; // if no message to read
}

// cycleTime() -- Verified working
// helper function to keep the time in the range of one cycle and incorporate the offset
// also resets the is_sent variable so we can send a new message if we get to a new cycle
unsigned long cycleTime(){
  long time = ((long)(millis() % CYCLE_LENGTH) + offset) % (long)CYCLE_LENGTH;
  if(last_time > time){ is_sent = false; } // checks if the clock reset and resets is_sent
  last_time = time; // for next time we call the function
  return (unsigned long)time;
}



