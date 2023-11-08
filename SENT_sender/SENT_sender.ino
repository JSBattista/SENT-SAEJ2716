/*
SENT demo and notes.     11-07-2023

Useful links:
  https://www.arduino.cc/reference/en/language/functions/time/micros/
  https://www.youtube.com/watch?v=qPz0Iu9Pzs8
  https://www.youtube.com/watch?v=LskY76oKdHY
  https://www.arduino.cc/reference/en/language/functions/time/delaymicroseconds/
  https://docs.arduino.cc/hacking/software/PortManipulation

  Specification determines 56 ticks for sync
            t_sync           840 uS      
  t_tick = ____________ = _______________ = 15 uS as one tick
            56                 56

Additional Notes:
 - Best to shut off interrupts as this makes things faster. 
 - The Sync interval set here might not be the actual time that ends up at the listener. For example, 840uS here, but coming out to 848uS at the other end. 
   This is due to the slow chip. Floating point values for the Tick can be possible so it's 15 here but can be 15.NN on the other end. 
 - NOTE THIS IS NOT A CAUSE OF DATA CORRUPTION in most cases apparently the decoder can compensate for "lag" of sorts. So using the same Sync pulse value at the decoder will not be a big problem.
 - using array access for the SENT encoded data (that data in an array) adds a few uS to each operation. 
 - The decoder end "knows" the length of the Sync pulse and uses it as such. 
 - A Pause pulse at the end of data and after CRC value is passed is optional and there does not appear to be much restriction regarding the length of time.
*/


double SENTencode (uint8_t v, double t)    
{
  v = v + 12; // Add 12.
  return t * v; // Multipl by tick so that the length of the pulse represents the data + 12
}

// Any value + 12 to encode ensures that "someting" is sent even for 0 values. Decode is number of ticks - 12 to get the value 0 - 15 so highest value is 27 which would be 15. 0 is 12.
// Time variuance itself IS the value. So what follows is a hardcoded set. 11-06-2023
// Tick is 840 / 56. Any sync time can be used, but divide it by 56 to get that tick time to determine nibbles.

uint8_t data[6]   = {0xF, 0x05,0x0A,0x02,0x08,0x00};   // Actual values in hex 0-F This is the original value
double  t_data[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};   // These are the SENT time-encoded results. 12 is added to the value then multiplied by tick time to determine the length of the pulse that represents the data value.

double tick;                // Actual tick time in uS which would be the length of the Sync pulse in uS divided by 56 (according to standard)
double t_sync;              // Calculated length of the sync pulse, which is tick multiplied by 56
double t_stat;              // A status register nibble 0-F. Can be any value needed that conveys information to the decoder.
double t_tcheck;            // The CRC check value, which also gets encoded on the way out. Algorithm for calculating this value is in the setup() function.

void setup() {
  // put your setup code here, to run once:
  noInterrupts();  // Highly necessary for a proper SENT output or it "swims" a little and wrecks CRC checking.
  Serial.begin(9600);
  tick = 15;    // value in uS
  t_sync    = 56 * tick; //840 uS     The receiving end takes the length of the sync pulse and divides it by 56.
  t_stat    = SENTencode(0, tick);//12 * tick; //180 uS   
  t_data[0]   = SENTencode(data[0], tick); //27 * tick; //405 uS       
  t_data[1]   = SENTencode(data[1], tick);//17 * tick; //255 uS
  t_data[2]   = SENTencode(data[2], tick);//22 * tick; //330 uS
  t_data[3]   = SENTencode(data[3], tick);//14 * tick; //210 uS
  t_data[4]   = SENTencode(data[4], tick);//20 * tick; //300 uS
  t_data[5]   = SENTencode(data[5], tick);//12 * tick; //180 uS
  uint8_t calculatedCRC; 
  uint8_t i;
  const uint8_t CrcLookup[16] = { 0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5 };
  calculatedCRC = 5; // initialize checksum with seed "0101"
  for (i = 0; i < 6; i++)
  {
      calculatedCRC = CrcLookup[calculatedCRC];
      calculatedCRC = (calculatedCRC ^ data[i]) & 0x0F;       // NOTE original data values are used to generate the CRC value.
  }
  // One more round with 0 as input
  calculatedCRC = CrcLookup[calculatedCRC];
  t_tcheck  = SENTencode(calculatedCRC, tick);//21 * tick; //315 uS    // Now the CRV value itself is SENT-encoded

      // Some information display routines.
  for (int incr = 0; incr < 6; incr++){
    Serial.print(data[incr], HEX);
    Serial.print(",");
  }  // for
  Serial.println();
  for (int incr = 0; incr < 6; incr++){
    Serial.print(t_data[incr]);
    Serial.print(",");
  }  // for
  Serial.flush();  // get the message out because the port writing thing is harsh.
  Serial.end();   // With no interrupts and the heavy hitting of the port writing scheme serial communication is fairly borked beyond this point.
  DDRB = B010000;   // Set up pins 8 - 13. 13 is output. Note this could also be the LED pin that will appear constantly on with fast signalling.  Using the LED pin does not appear to have any side-effects.
                    // Note the endian order: the last value is Pin 8, the first (on the left) is pin 13.
}
  // In order to have a low state that is detectable at the decoder, and long enough to overcome circuit cap/ind that denies enough low state to be detected as "below threshold". Situations can vary.
  // This can effect pulse length. It appears that an absolute minimum helps. A 1uS downstate was not eneough for the scope to detect a low, but 4 - 6seems reliable. THIS WILL AFFECT TICK TIME.
uint8_t downstate = 4;      
void loop() {
  PORTB = B010000;
  delayMicroseconds(t_sync);
  PORTB = B000000;
  delayMicroseconds(downstate-1); // Long pulses do not appear to require as-long down states.

  PORTB = B010000;
  delayMicroseconds(t_stat);
  PORTB = B000000;
  delayMicroseconds(downstate);

  PORTB = B010000;
  delayMicroseconds(t_data[0]);
  PORTB = B000000;
  delayMicroseconds(downstate);


  PORTB = B010000;
  delayMicroseconds(t_data[1]);
  PORTB = B000000;
  delayMicroseconds(downstate);


  PORTB = B010000;
  delayMicroseconds(t_data[2]);
  PORTB = B000000;
  delayMicroseconds(downstate);


  PORTB = B010000;
  delayMicroseconds(t_data[3]);
  PORTB = B000000;
  delayMicroseconds(downstate);



  PORTB = B010000;
  delayMicroseconds(t_data[4]);
  PORTB = B000000;
  delayMicroseconds(downstate);



  PORTB = B010000;
  delayMicroseconds(t_data[5]);
  PORTB = B000000;
  delayMicroseconds(downstate);

  PORTB = B010000;
  delayMicroseconds(t_tcheck);
  PORTB = B000000;
  delayMicroseconds(downstate);

// something of a pause pulse?
  PORTB = B010000;
  delayMicroseconds(10000);
  PORTB = B000000;
  delayMicroseconds(downstate-2);
  //digitalWrite(sigSENT, LOW);

 
   

}


