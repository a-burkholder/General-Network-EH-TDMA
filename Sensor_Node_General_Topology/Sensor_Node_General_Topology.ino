


const int TOTAL_NODES = 5; // Total number of nodes in the network
const String ID = "04"; // Each node knows its ID based on assumption
const int TIME_SLOT = 500; // amount of time per slot in milliseconds (ms) 10^-3
const int ERROR = 60; // Transmission time error threshold
const int ENERGY_CHANCE = 80; // energy harvest rate


/* FLAGS... and stuff*/
bool led_state = false;
bool trans = false;
bool updated = false; // tracks if we need to read a time for syncing or if we already did that
bool is_sync = false; // keeps track of if the last read message was a sync


/* Timers */

unsigned long transmit_time; // time in the cycle to transmit
unsigned long cycle_time = 0; // global cycle time from the last sync message

unsigned long wait_time = 0; // time to wait before next active state
unsigned long start_wait = 0; // start time of the waiting




unsigned long sync_time = 0; // unused
unsigned long current_time = 0; // unused



/* Transmition stuff */
String incoming = ""; // holds whole input
String data_in = ""; // the data coming in
String time_in = ""; // the global cycle time coming in

bool energyAvailible();
void stateMachine();

void setup() {
  // put your setup code here, to run once:
  transmit_time = (ID-1) * TIME_SLOT;
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(cycleTime());

}


// energyAvailible()
// RNG for if a node has energy or not, based on the chances of that node having energy
bool energyAvailible(){
  if(random(0,100) <= ENERGY_CHANCE){
    return true;
  } 
  else return false;
}

// stateMachine()
// The function that implements the states and their functionality in the nodes
void stateMachine(){
  static enum { DEAD, SYNC, WAIT, ACTIVE } state = DEAD;
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
          updated = true;
        }
      }
      
      if(updated){
        cycle_time = time_in; // sync based on message
        wait_time = transmit_time - cycle_time; // find time needed to wait
        start_wait = millis();
        state = WAIT;
      }

      updated = false;
      break;

    case WAIT:
      // check if in time slot
      if(millis() - start_wait >= wait_time){
        state = ACTIVE;
        break;
      }
      
      // if not in time slot
      else{
        if(readData()){
          updated = true;
          if (is_sync){
            state = SYNC;
            break;
          }
        }
      }  
      break;

    case ACTIVE:
      // check for overlap errors 
      // update the message
      // send the message

      // energy checking
      if(energyAvailible()){
        state = WAIT;
        break;
      }
      else {
        state = DEAD;
        break:
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
      time_in = incoming.substring(1,incoming.length()); //maybe broken bc bad indexing
    }

    else if(sync_flag == "S"){  //if normal sync 
      int num_syncs = incoming.substring(1,3).toInt(); //number of nodes we need to search through
      //--linear search through all node IDs--//   REPLACE WITH BINARY SEARCH EVENTUALLY
      for(int i = 3; i < (num_syncs*2)+3; i+=2){
        String to_check = incoming.substring(i, i+2);
        if(to_check == ID){ //if this node finds it's ID on the sync list
          is_sync = true;
          time_in = incoming.substring((num_syncs*2)+3, incoming.length());
          break;
        }
      }
    }

    else{ //if not sync 
      is_sync = false;
      int data_idx = incoming.indexOf(',',2); //the index of the comma separating the data from the cycle time
      time_in = incoming.substring(1,data_idx);
      data_in = incoming.substring(data_idx + 1, incoming.length());
    }
  }
  return isMessage;
}


// cycleTime()
// helper function to keep the time in the range of one cycle
unsigned long cycleTime(){
  return millis()%(((TOTAL_NODES)*TIME_SLOT));
}



