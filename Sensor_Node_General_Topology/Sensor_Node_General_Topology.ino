

/* Constants and assumptions*/
const int TOTAL_NODES = 5;                                  // Total number of nodes in the network
const int TIME_SLOT = 500;                                  // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long CYCLE_LENGTH = TOTAL_NODES*TIME_SLOT;   // total length of one cycle
const int ERROR = 300;                                       // Transmission time error threshold
const String ID = "04";                                     // Each node knows its ID based on assumption
const int ENERGY_CHANCE = 1000;                               // energy harvest rate



/* FLAGS... and stuff*/
bool led_state = false;   // unused
bool trans = false;       // unused
bool updated = false;     // tracks if we need to read a time for syncing or if we already did that
bool is_sync = false;     // keeps track of if the last read message was a sync


/* Timers */
unsigned long transmit_time;    // time in the cycle to transmit
unsigned long receive_time;
//unsigned long global_time = 0;  // global cycle time from the last sync message
long offset = 0;                // offset from the node's cycle to the global cycle
//unsigned long start_clock = 0;  // the current node's equivalent time to the global cycle time
unsigned long last_time;        // the time at the last time it was checked


/* Transmition stuff */
String data_in;                // the data coming in
long time_sent;                // the time the previous node sent the message
unsigned long time_in;         // time that this node received the message, ideally same as time_sent

bool is_sent = false;   // checks if a message was sent this cycle
int clock_diff;         // used for calculating overlap between nodes
int is_overlap;         // to hold the overlap info: 1 for behind, 2 for ok, 3 for ahead

bool energyAvailible();
void nodeFSM();
bool readData();
unsigned long cycleTime();

void setup() {
  // put your setup code here, to run once:
  transmit_time = (ID.toInt() - 1) * TIME_SLOT;
  receive_time = transmit_time - TIME_SLOT;
  Serial.begin(9600);
  Serial.setTimeout(3000);
  Serial.println(transmit_time);
}

void loop() {
  // put your main code here, to run repeatedly:
  nodeFSM();
}


// energyAvailible() // -- Verified working
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
  static enum { DEAD, SYNC, WAIT, ACTIVE } state = DEAD;
  switch (state) {
    case DEAD: // -- Verified working
    Serial.println("DEAD");
      if(energyAvailible()) {
        updated = false;
        state = SYNC;
        Serial.println("SYNC");
      }
      break;
    
    case SYNC: // -- Verified working
      //--for if we are waiting for a message to get time from--//
      if(!updated){
        if(readData()){
          time_in = millis() % CYCLE_LENGTH;
          updated = true;
        }
      }
      
      //--for if we have the time to sync off of--//
      if(updated){
        offset = time_sent - (millis() % CYCLE_LENGTH); // for cycleTime() function
        time_in = time_in + offset;
        state = WAIT;
        updated = false;
        Serial.println("WAIT");
      }
      break;

    case WAIT: // -- Verified working
      //--if in time slot--//
      if(cycleTime() >= transmit_time && !is_sent){
        state = ACTIVE;
        Serial.println("ACTIVE");
        break;
      }
      //--if not in time slot--//
      else{
        
        if(readData()){
          updated = true;
          time_in = millis() % CYCLE_LENGTH;
          if (is_sync){
            state = SYNC;
            Serial.println("SYNC");
            break;
          }
        }
      }  
      break;

    case ACTIVE: // -- Verified working
      //--check for overlap errors--//
      clock_diff = time_in - receive_time; 
      Serial.println(receive_time);
      Serial.println(time_in);
      Serial.println(clock_diff);
      Serial.println(offset);
      Serial.println(cycleTime());
      if(clock_diff > ERROR){ // behind
        is_overlap = 1;
      }
      else if (clock_diff > -ERROR){ // ok
        is_overlap = 2;
      }
      else { // ahead
        is_overlap = 3; 
      }

      String message = "D," + (String)cycleTime() + "," + ID + (String)is_overlap + data_in; // update the message
      Serial.println(message); // send the message
      data_in = "E";
      is_sent = true;

      // energy checking
      if(energyAvailible()){
        state = WAIT;
        Serial.println("WAIT");
        break;
      }
      else {
        state = DEAD;
        break;
      }
      break;
  }
}

// readData() // -- Verified working
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
// If it reads data, returns true
bool readData(){
  if(Serial.available()){ // is there anything to read?
    String type = Serial.readStringUntil(',');
    time_sent = Serial.parseInt();

    //--if data--//
    if(type == "D"){
      data_in = Serial.readStringUntil('\n');
      is_sync = false;
    }

    //--if targeted sync--// 
    else if(type == "S"){
      long num_syncs = Serial.parseInt();
      String sync_list = Serial.readString();
      
      //--linear search through all node IDs--//   REPLACE WITH BINARY SEARCH EVENTUALLY
      for(int i = 1; i <= num_syncs; i++){
        String to_check = sync_list.substring((i*2-1), (i*2)+1);
        if(to_check == ID){ //if this node finds it's ID on the sync list
          is_sync = true;
          break;
        }
      }
    }

    //--if general sync--//
    else if(type == "G"){
      is_sync = true;
    }

    Serial.readStringUntil('\r'); // clears input buffer
    return true; // if theres a message
  }
  return false; // if no message to read
}

// cycleTime() -- Verified working
// helper function to keep the time in the range of one cycle and incorporate the offset
// also resets the is_sent variable so we can send a new message if we get to a new cycle
unsigned long cycleTime(){
  unsigned long time = (millis() % CYCLE_LENGTH) + offset;
  if(last_time > time){ // checks if the clock reset and resets is_sent
    is_sent = false;
  }
  last_time = time; // for next time we call the function
  return time;
}



