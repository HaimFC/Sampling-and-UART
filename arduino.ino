//---------------------------------------Definitions------------------------------------------------------------------------------

#define DATA 0b11000011         			     				//Data to send                 ||--------------------------------------||
#define SIZE 8			 		                    			//Data size                    ||Able to change data size between 0-12 || 
#define ANDBIT 0b011111111  									//number of 1 digits is SIZE ||--------------------------------------||

//--------------------------------------------------------------------------------------------------------------------------------
#define BIT_TIME 20 											//1/50 clk duty cycle          ||--------------------------------------||
#define SAMP_NUM 3                                  			//Sample Num		           ||Able to change the protocol preference||
#define CALC_TIME BIT_TIME/(SAMP_NUM+2)							//calc formula			       ||--------------------------------------||

//---------------------------------------Variables for tx function----------------------------------------------------------------

int txCase = 0;                                     			//0-Idle, 1-start, 2-data, 3-parity, 4-stop
int countTra = 0;								    			//size of the transfered data
unsigned int dataToSend = DATA;  								//initialize data to send
int parityCountTX = 0;                              			//parity value

//---------------------------------------Variables for rx function----------------------------------------------------------------

int rxCase = 0;									    			//0-Idle, 1-start, 2-data, 3-parity, 4-stop, 5-error
int countRec = 0;												//size of the recieved data
unsigned int recievedData=0;   					    			//initialize data to recieve
int parityCountRX = 0;                              			//parity value
int parityNum = 0;												//keep value of parity after checking if corrupted (1 or 0)
int time_to_wait = 0;  							    			//time to wait if data was corrupted
unsigned long currentRX = 0;									//keep the current time in the reciever

//---------------------------------------Variables for sampling function---------------------------------------------------------

unsigned int sampledData = 0;                       			//keep the sampled data
int sampleCounter = 0;											//count the sampled data size
unsigned long currentFUNC = 0;                      			//keep the current time in the function
int mask = 0;

//---------------------------------------Variables for time function-------------------------------------------------------------

unsigned long currentTX = 0;									//keep the current time in the tranfer

//-------------------------------------------------------------------------------------------------------------------------------

void setup() {
  	Serial.begin(9600);			 								//BAUD_RATE = 9600 
  	pinMode(5, OUTPUT);   		 								//initialize Tx - analog pin
  	pinMode(6,INPUT);				 							//initialize Rx - analog pin
    digitalWrite(5, HIGH);
  	bitWrite(mask, 0, 0);										//create mask variable for modular code
  	bitWrite(mask, SAMP_NUM, 0);
  	for(int i = 1; i <= SAMP_NUM; i++)
      bitWrite(mask, i, 1);
}

void loop() {
    
    usart_tx();        			 				    			//tranfer function
    usart_rx();         						    			//reciever function
}

void usart_tx()
{ 
  	boolean leastSign = bitRead(dataToSend, countTra);  		//lsb bit
  	long randTime = random(200, 1000);                          //generate random number of time
  	switch (txCase) {
    case 0:														//Idle case
      if(time(randTime))                                        //random time passed
        txCase = 1;                                             //change state to start
      break;
      
    case 1:														//start state
      if(!time(BIT_TIME))										//transfer start bit for BIT_TIME 
        digitalWrite(5, LOW);
      else														//BIT_TIME passed
        txCase = 2;												//change state to data     
      break;
      
    case 2:														//data state
      if(!time(BIT_TIME))  										//transfer data by index from lsb to msb for BIT_TIME 
      {
        if(leastSign)											//digit equal to 1
          digitalWrite(5, HIGH);
        else
          digitalWrite(5, LOW);									//digit equal to 0
      }
      else    													//BIT_TIME passed
      {
        countTra++;												//raise by one the data index										 
        if(leastSign)											//only after the BIT_TIME passes count "on the fly"
           parityCountTX++;
      }
      if(countTra == SIZE)										//if finished to transfer the data
      {
        countTra = 0;
        txCase = 3;												//change state to parity
        if(parityCountTX%2 == 0)								//check parity value
          parityNum = 1;										//for even 1 digits case
        else
          parityNum = 0;										//for odd 1 digits case
        parityCountTX = 0;										//nullify for next transfer

      }
      break;
      
    case 3:														//parity state
      if(!time(BIT_TIME))										//transfer the parity digit for BIT_TIME
      {
        if(parityNum)											//for 1 digits case
          digitalWrite(5, HIGH);								//tranfer 1 
        else													//for 0 digits case
          digitalWrite(5, LOW);									//tranfer 0 
      }
      else														//BIT_TIME passed
        txCase = 4;												//change state to stop
      break;
      
    case 4:														//stop state
      if(!time(BIT_TIME))										//transfer the stop digit - 1 for BIT_TIME
        digitalWrite(5, HIGH); 
      else														//BIT_TIME passed
        txCase = 0;												//all data transfered, change back to IDLE	
      break;
      
    default:													//any other case - nullify variables
      	countTra = 0;											
      	parityCountTX = 0;
      	txCase = 0;												//change case to IDLE	
      break;
	}
}
void usart_rx()
{
  long timerRx = millis();                                      ////time since arduino turned on
  int sample = 0;												//initialize sample flag
  
  switch (rxCase) {		
    
  case 0:														//IDLE state
    if(!digitalRead(6))											//when the digital bit is 0
    	rxCase = 1;												//change state to start
    break;
    
  case 1:														//start state
    sample = sampling();										//start to sample digit
    if(sample == 1 || sample == -1)								//in case of error
    {
      rxCase = 5;												//change state to error
      time_to_wait = BIT_TIME * 10;								//wait BIT_TIME * 10 bits that left
      currentRX = millis();										//check the current time
    }
    else if(sample == 0)										//if the sample is 0 the start state is finished	
      rxCase = 2;												//change state to data      
    break;
    
  case 2:
    sample = sampling();										//start to sample digit
    if(sample == 1 || sample == 0)								//correct options
    {
      if(sample == 1)											//option 1 - digit is 1
      {
        bitWrite(recievedData, countRec, 1);					//add to recieve data by index value 1
        parityCountRX++;
      }
      else
        bitWrite(recievedData, countRec, 0);					//add to recieve data by index value 0
      
      countRec++;												//increase the data integer index
      if(countRec == SIZE+1)									//data size is DATA SIZE + parity bit
      {
        countRec = 0;											//nullify for next data
        rxCase = 3;											    //change the state to parity
      }
    }
    else if(sample == -1)										//error case
    {
      rxCase = 5;												//change state to error
      time_to_wait = (10-countRec)*BIT_TIME;					//wait the BIT_TIME*digits that left to read
      currentRX = millis();										//save the last time that error occured
      countRec = 0;												//nullify for next data
    }
    break;
    
  case 3:														//parity case
    if(parityCountRX%2 != 1)									//in case that parity is incorrect
    {
      rxCase = 5;												//change state to error
      time_to_wait = BIT_TIME;									//wait the stop bit
      parityCountRX = 0;										//nullify for next data
    }
    else														//in case that parity is correct							
    {											
      rxCase = 4;												//change state to stop												
      parityCountRX = 0;										//nullify for next data
    }
    break;
    
  case 4:														//stop case	
    sample = sampling();										//start to sample stop digit
    if(sample == 0 || sample == -1)								//incorrect sample
    {
      rxCase = 5;												//change to error case
      time_to_wait = 0;											//no time left to wait
      currentRX = millis();										//save the last time that error occured
    }
    else if(sample == 1)										//correct sample
    {
      recievedData = recievedData & ANDBIT;						//remove parity bit
      Serial.println(recievedData, BIN);						//print the full date
      rxCase = 0;  												//change to IDLE case
    }
    break;
    
  case 5: 														//error case			 						 
    if (timerRx - currentRX >= time_to_wait)   					//wait for the time that left by the last digit that recieved     
    {
      countRec = 0;												//nullify for next data
      recievedData = 0;
      parityCountRX = 0;
      rxCase = 0;												//change to IDLE case
    }
    break;
    
  default:
      countRec = 0;												//nullify for next data
      recievedData = 0;
      parityCountRX = 0;
      rxCase = 0;												//change to IDLE case
    break;
  }
}

bool time(long waitingTime)										//Time function for tx Function
{
  long timer = millis(); 				 						//time since arduino turned on
  if (timer - currentTX >= waitingTime)           				//waitingTime is passed
  {
    	currentTX = millis();									//save current time
    	return true;											//return the time is passed
  }
  else															//time didnt pass yet, return false
  		return false;									
}

int sampling()
{ 
  long timer = millis(); 				 						//time since arduino turned on
  if (timer - currentFUNC >= CALC_TIME)           	    		//CALC_TIME is passed
  {								
    bitWrite(sampledData, sampleCounter, digitalRead(6));		//keep the sampled bit in the data by index(sampleCounter)
    sampleCounter++;	 										//sampledData size
    currentFUNC = millis();										//save current time
  }
  else															//time didnt pass yet, return 2 - the function didnt finish to sample
  	return 2;
  if(sampleCounter == SAMP_NUM + 2)								//function did finish to sample
  {
    sampledData = sampledData & mask;							//check curropted values
    sampleCounter = 0;											//nullify for the next sample
    if(sampledData == mask)										//equals 14 means 1
      return 1;   												
    else if(sampledData == 0)									//equals 0 means 0
      return 0;
    else														//there is an error
      return -1;    
  }
  return 2;														//any other case
}