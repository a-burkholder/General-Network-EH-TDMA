
const int TOTAL_NODES = 10; // Total number of nodes in the network
const int ID = 4; // Each node knows its ID based on assumption
const int TIME_SLOT = 500; // amount of time per slot in milliseconds (ms) 10^-3
const int ERROR = 60; // Transmission time error threshold
const int ENERGY_CHANCE = 80; // energy harvest rate


/* FLAGS... and stuff*/
bool led_state = false;
bool trans = false;
bool firstFlag = false;
bool is_Sync = false;


/* Timers */
unsigned long sync_time = 0;
unsigned long current_time = 0;
unsigned long last_time = 0;
unsigned long wait_time = 0;

/* Transmition stuff */
String incoming = ""; 

bool energyAvailible();
void stateMachine();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  readData();

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
        state = SYNC;
      }
      break;
    
    case SYNC:
      //---read input stream---//
      
      //sync based on first message
      break;
    case WAIT:

      //check time slot
      current_time = millis();
      if(current_time >= wait_time){
        state = ACTIVE;
        trans = true;
      }
      

      //if not in time slot
      else{
        if(!trans){ // is this needed???
          
          //---read data---//
          /*
          if(Serial.available()){
            incoming = Serial.readStringUntil('\r');
            flag1 = incoming.substring(0,1); //grab first thing
          }

          if(flag1 == 'S'){  //if data is a general sync command
            state = SYNC;
          }
          */
        }
      }  
          
      //if in time slot
        state = ACTIVE;
      break;
    case ACTIVE:
      //check for overlap errors 
      //update the message
      //send the message
      if(energyAvailible()){
        state = WAIT;
      }
      else state = DEAD;



  }


}

// readData()
// Helper function that updates the variables that hold the data. Created to simplify code. (and improve efficency)
void readData(){
Serial.println("S" + (String)ID);
  //--Read the data--//
  if(Serial.available()){
    String incoming = Serial.readStringUntil('\r'); 
  }

  //--Check for sync or not--//
  String temp = incoming.substring(0,2); //grab first two things
  if(temp == "S " || temp == "S" + (String)ID){  //if data is a sync command
    is_Sync = true;  
  }
  else{

  }
  
}




