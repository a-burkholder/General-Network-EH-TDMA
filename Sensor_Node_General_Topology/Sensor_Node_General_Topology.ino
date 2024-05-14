


const int TIME_SLOT = 500; // In milliseconds (ms) 10^-3
const int ERROR = 60; // Transmission Time
const int ENERGY_CHANCE = 80;


/* FLAGS... and stuff*/
bool led_state = false;
bool trans = false;
bool firstFlag = false;


/* Timers */
unsigned long current_time = 0;
unsigned long last_time = 0;
unsigned long wait_time = 0;

/* Transmition stuff */
String incoming = ""; 

bool energyAvailible();
void stateMachine();

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  

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
      //read input stream
      //sync based on first message
      break;
    case WAIT:

      //check time slot
      current_time = millis();
      if(current_time >= wait_time){
        state = ACTIVE;
        trans = true;
      }
      
      
      String flag1;

      //if not in time slot
      else{
        if(!trans){ // is this needed???
          
          //---read data---//
          if(Serial.available()){
            incoming = Serial.readStringUntil('\r');
            flag1 = incoming.substring(0,1); //grab first thing
          }

          if(flag1 == 'S'){  //if data is a general sync command
            state = SYNC;
          }
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
