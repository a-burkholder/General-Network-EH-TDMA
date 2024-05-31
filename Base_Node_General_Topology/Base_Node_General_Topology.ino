

/* Constants and assumptions*/
const int num_zones = 1;
const String ZONES[num_zones] = {"01"};
const int TOTAL_NODES = 2;                                 // Total number of nodes in the network
const int TIME_SLOT = 400;                                  // amount of time per slot in milliseconds (ms) 10^-3
const unsigned long CYCLE_LENGTH = (TOTAL_NODES+1) * TIME_SLOT; // total length of one cycle
const int ERROR = 80;                                       // Transmission time error threshold
const int ENERGY_CHANCE = 80;                               // energy harvest rate
const int TRANSMIT_TIME = TIME_SLOT * TOTAL_NODES;


/* FLAGS... and stuff*/
bool is_sent = false;              // checks if a message was sent this cycle
bool is_overlap = false;           // to send the state mechine to SYNC_LIST if true          


/* Timers */
unsigned long transmit_time;      // time in the cycle to transmit
long offset = 0;                  // offset from the node's cycle to the global cycle
unsigned long last_time = 0;      // the time at the last time it was checked
long time_sent;                   // the time the previous node sent the message
unsigned long time_in;            // local arrival time, then converted to global arrival time, ideally the same as time_sent
int clock_diff;                   // used for calculating overlap between nodes
unsigned long last_packet_in;     // used for checking if we are not getting messages. if no messages in 3 cycles, reset the network


/* Transmition stuff */
String data_in;                 // the data coming in
long num_syncs = 0;             // the number of syncs to send out if sync list
String sync_list = "";          // the list containing all the nodes that need to be resynced

/*  Data stuff  */
String data[TOTAL_NODES];


void baseFSM();
bool readData();
unsigned long cycleTime();
bool isInZone(String zone);

void setup() {
  // put your setup code here, to run once:
  transmit_time = (TOTAL_NODES) * TIME_SLOT;
  Serial.begin(9600);
  Serial.setTimeout(30);
}

void loop() {
  // put your main code here, to run repeatedly:
  baseFSM();
}


// baseFSM()
// The function that implements the states and their functionality in the base
void baseFSM(){
  static enum { SYNC_ALL, ACTIVE, SYNC_LIST } state = SYNC_ALL;
  switch(state){
    case SYNC_ALL:
      Serial.println("A,G," + (String)cycleTime());// send a general sync message 
      state = ACTIVE;
      break;

    case ACTIVE:
      if(readData()){
        Serial.println("time in = " + (String)time_in + ",");
        // parse the data and check for any overlapp errors (1 behind, 2 ok, 3 ahead)
        sync_list = "";
        
        for(int i = data_in.length(); i >= 1; i -= 4){
          if(data_in.substring(i - 1, i) != "2"){
            //add to list
            sync_list = sync_list + data_in.substring(i + 1, i + 3);
            num_syncs++;
            is_overlap = true;
          }
        }
        
        //--check for overlap errors in most recent message--//
        clock_diff = abs((long)time_in - time_sent);
        if(clock_diff > ERROR){ // behind
          sync_list = sync_list + data_in.substring(1,3);
          is_overlap = true;
          num_syncs++;
        }
        
        else if (clock_diff < -ERROR){ // ahead
          is_overlap = true; 
          num_syncs++;
          sync_list = sync_list + data_in.substring(1,3);
        }
      }

      else if(cycleTime() == TRANSMIT_TIME && !is_sent){
        is_sent = true;
        state = SYNC_LIST;
      }

      // if all nodes dead, SYNCA
      if(millis() - last_packet_in > 3 * CYCLE_LENGTH){
        state = SYNC_ALL;
        break;
      }
      break;
      
    case SYNC_LIST:
      // send a sync to specific nodes
      if(is_overlap){
        Serial.println("A,S," + (String)cycleTime() + "," + (String)num_syncs + "," + sync_list);
        sync_list = "";
        is_overlap = false;
        num_syncs = 0;
      }
      state = ACTIVE;
      break;
  }
}

// readData()
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
// If it reads data, returns true
bool readData(){
  if(Serial.available() > 0){ // is there anything to read?
    String zone = Serial.readStringUntil(',');
    if(isInZone(zone)){
      last_packet_in = millis();
      String type = Serial.readStringUntil(',');
      time_in = cycleTime();
      if(type == "D"){
        time_sent = Serial.parseInt();
        Serial.readStringUntil(',');
        //--if data--//
        data_in = "";
        for(int i = 0; i < TOTAL_NODES; i++ ){
          data[i] = "E";
        }
        while(true){
          String p_data = Serial.readStringUntil(',');
          if(p_data == "E"){
            Serial.readStringUntil('\r');         
            break;// D,4500,102,092,082,072,063,052,042,033,022,012,E,
          }
          int data_idx = p_data.substring(0,2).toInt();
          data[data_idx-1] = p_data;
          data_in = data_in + "," + p_data;
        }
        return true;
      } 
    }
  }
  return false; // if no message to read
}

// cycleTime() -- Verified working
// helper function to keep the time in the range of one cycle and incorporate the offset
// also resets the is_sent variable so we can send a new message if we get to a new cycle
unsigned long cycleTime(){
  unsigned long time = millis() % CYCLE_LENGTH;
  if(last_time > time){ // checks if the clock reset and resets is_sent
    is_sent = false;
  }
  last_time = time; // for next time we call the function
  return time;
}


// inInZone()
// helper to find if the message is for this node
bool isInZone(String zone){
  if(zone == "A"){ return true; }
  int num_zones_in = zone.length();
  for(int i = 0; i < num_zones; i++){
    for(int j = 0; j < num_zones_in; j++){
      String to_check = zone.substring(j, j+2);
      if(ZONES[i] == to_check){
        return true;
      }
    }
  }
  return false;
}
