

/* Constants and assumptions*/
const int TOTAL_NODES = 5; // Total number of nodes in the network
const int TIME_SLOT = 500; // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long CYCLE_LENGTH = TOTAL_NODES*TIME_SLOT; // total length of one cycle
const int ERROR = 60; // Transmission time error threshold
const String ID = "04"; // Each node knows its ID based on assumption
const int ENERGY_CHANCE = 1000; // energy harvest rate



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
unsigned long last_time; // the time at the last time it was checked


/* Transmition stuff */
String incoming = "Placeholder data input string"; // holds whole input
String data_in = "Placeholder data"; // the data coming in
String time_sent = "1500"; // the global cycle time coming in
unsigned long time_in = "1500"; // time that this node received the message, ideally same as time_sent

bool is_sent = false; // have you sent a message this cycle
int clock_diff; // used for calculating overlap
int is_overlap; // to hold the overlap info: 1 for behind, 2 for ok, 3 for ahead

bool energyAvailible();
void stateMachine();
bool readData();
unsigned long cycleTime();

void setup() {
  // put your setup code here, to run once:
  transmit_time = (ID.toInt() - 1) * TIME_SLOT;
  Serial.begin(9600);

  
}

void loop() {
  // put your main code here, to run repeatedly:
  nodeFSM();
}


// energyAvailible()
// RNG for if a node has energy or not, based on the chances of that node having energy
bool energyAvailible(){
  if(random(0,100) <= ENERGY_CHANCE){
    return true;
  } 
  else return false;
}

// nodeFSM()
// The function that implements the states and their functionality in the nodes
void nodeFSM(){
  static enum { DEAD, SYNC, WAIT, ACTIVE } state = WAIT;
  switch (state) {
    case DEAD:
      if(energyAvailible()) {
        updated = false;
        state = SYNC;
      }
      break;
    
    case SYNC:
      // read input stream
      if(!updated){
        if(readData()){
          time_in = cycleTime();
          updated = true;
        }
      }
      
      if(updated){
        global_time = time_sent.toInt(); // sync based on message
        start_clock = millis();
        offset = ((int)global_time - (int)start_clock);
        state = WAIT;
      }

      updated = false;
      break;

    case WAIT: // -- Verified working
      // check if in time slot
      if(cycleTime() >= transmit_time && !is_sent){
        state = ACTIVE;
        break;
      }
      
      // if not in time slot
      
      else{
        Serial.println("Waiting...");
        if(readData()){
          updated = true;
          time_in = cycleTime();
          if (is_sync){
            state = SYNC;
            break;
          }
        }
      }  
      break;

    case ACTIVE: // -- Verified working
      // check for overlap errors 
      clock_diff = ((String)time_in).compareTo(time_sent); 
      if(clock_diff > ERROR){ // behind
        is_overlap = 1;
      }
      else if (clock_diff > -ERROR){ // ok
        is_overlap = 2;
      }
      else { // ahead
        is_overlap = 3; 
      }

      // update the message
      String message = "D" + (String)cycleTime() + ID + (String)is_overlap + data_in;

      // send the message
      Serial.println(message);
      is_sent = true;

      // energy checking
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

// readData()
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
// If it reads data, returns true
bool readData(){
  bool isMessage = false;

  //--Read the data--//
  if(Serial.available()){
    String incoming = Serial.readStringUntil('\r'); 
    isMessage = true;

    //--Check for sync or not--//
    String sync_flag = incoming.substring(0,1); //grab first character
    if (sync_flag == "G"){  //if general sync 
      is_sync = true;
      time_sent = incoming.substring(1,incoming.length()); //maybe broken bc bad indexing
    }

    else if(sync_flag == "S"){  //if normal sync 
      int num_syncs = incoming.substring(1,3).toInt(); //number of nodes we need to search through
      //--linear search through all node IDs--//   REPLACE WITH BINARY SEARCH EVENTUALLY
      for(int i = 3; i < (num_syncs*2)+3; i+=2){
        String to_check = incoming.substring(i, i+2);
        if(to_check == ID){ //if this node finds it's ID on the sync list
          is_sync = true;
          time_sent = incoming.substring((num_syncs*2)+3, incoming.length());
          break;
        }
      }
    }

    else{ //if not sync 
      is_sync = false;
      int data_idx = incoming.indexOf(',',2); //the index of the comma separating the data from the cycle time
      time_sent = incoming.substring(1,data_idx);
      data_in = incoming.substring(data_idx + 1, incoming.length());
    }
  }
  Serial.flush();
  return isMessage;
}

// cycleTime() -- Verified working
// helper function to keep the time in the range of one cycle and incorporate the offset
// also resets the is_sent variable so we can send a new message if we get to a new cycle
unsigned long cycleTime(){
  unsigned long time = (millis() + offset) % CYCLE_LENGTH;
  if(last_time > time){
    is_sent = false;
  }
  last_time = time;
  return time;
}



