import processing.serial.*;
import cc.arduino.*;

Arduino arduino;

color off = color(4, 79, 111);
color on = color(84, 145, 158);
  
int CLKpin = 4;    // <-- Arduino pin delivering the clock pulses to pin 3 (CLK) of the TSL1402R
int SIpin = 5;     // <-- Arduino pin delivering the SI (serial-input) pulse to pin 2 of the TSL1402R
int AOpin1 = 1;    // <-- Arduino pin connected to pin 4 (analog output 1)of the TSL1402R
int AOpin2 = 2;    // <-- Arduino pin connected to pin 8 (analog output 2)of the TSL1402R
int[] IntArray = new int[256]; // <-- the array where the readout of the photodiodes is stored, as integers

int count = 0;
String measurement = "";

void setup() {
  size(470, 200);
  println("------------------ SETUP -------------------------");

  // Prints out the available serial ports.
  for ( int i = 0; i < Arduino.list().length; i++ )
  {
    println(Arduino.list()[i]);
  }
  
  // Modify this line, by changing the "0" to the index of the serial
  // port corresponding to your Arduino board (as it appears in the list
  // printed by the line above).
  arduino = new Arduino(this, Arduino.list()[0], 57600);

  // Alternatively, use the name of the serial port corresponding to your
  // Arduino (in double-quotes), as in the following line.
  //arduino = new Arduino(this, "/dev/tty.usbmodem621", 57600);

  // Set the Arduino digital pins as outputs.
  //for (int i = 0; i <= 13; i++)
    //arduino.pinMode(i, Arduino.OUTPUT);
    
  // Initialize two Arduino pins as digital output:
  arduino.pinMode(CLKpin, Arduino.OUTPUT);
  arduino.pinMode(SIpin, Arduino.OUTPUT);
  
  // Set all IO pins low:
  for ( int i = 0; i < 14; i++ )
  {
    arduino.digitalWrite(i, Arduino.LOW);
  }
  
  // Clock out any existing SI pulse through the ccd register:
  for (int i = 0; i < 260; i++)
  {
    ClockPulse();
  }
  
  // Create a new SI pulse and clock out that same SI pulse through the sensor register:
  arduino.digitalWrite(SIpin, Arduino.HIGH);
  ClockPulse();
  arduino.digitalWrite(SIpin, Arduino.LOW);
  for (int i = 0; i < 260; i++)
  {
    ClockPulse();
  }
}

void draw() {
  background(off);
  stroke(on);
  println("------------------ DRAW -------------------------");
  // Stop the ongoing integration of light quanta from each photodiode by clocking in a SI pulse
  // into the sensors register:
  arduino.digitalWrite(SIpin, Arduino.HIGH);
  ClockPulse();
  arduino.digitalWrite(SIpin, Arduino.LOW);
  
  // Next, read all 256 pixels in parallell. Store the result in the array. Each clock pulse
  // causes a new pixel to expose its value on the two outputs:
  for (int i = 0; i < 128; i++)
  {
    //delay(20);// <-- We add a delay to stabilize the AO output from the sensor
    IntArray[i] = arduino.analogRead(AOpin1);
    IntArray[i + 128] = arduino.analogRead(AOpin2);
    ClockPulse();
  }
  for (int i = 0; i < 128; i++)
    println(i + " " + IntArray[i] + " - " + IntArray[i + 128]);
  
  // Next, stop the ongoing integration of light quanta from each photodiode by clocking in a
  // SI pulse:
  arduino.digitalWrite(SIpin, Arduino.HIGH);
  ClockPulse();
  arduino.digitalWrite(SIpin, Arduino.LOW);
  
  // Next, send the measurement stored in the array to host computer using serial (rs-232).
  // communication. This takes ~80 ms during whick time no clock pulses reaches the sensor.
  // No integration is taking place during this time from the photodiodes as the integration
  // begins first after the 18th clock pulse after a SI pulse is inserted:
  for (int i = 0; i < 256; i++)
  {
    measurement = measurement + IntArray[i] + ";";
  }
  
  //println(measurement);
  
  // Next, a new measuring cycle is starting once 18 clock pulses have passed. At
  // that time, the photodiodes are once again active. We clock out the SI pulse through
  // the 256 bit register in order to be ready to halt the ongoing measurement at our will
  // (by clocking in a new SI pulse):
  for (int i = 0; i < 260; i++)
  {
    if (i == 18)
    {
      // Now the photodiodes goes active..
      // An external trigger can be placed here
    }
    ClockPulse();
  }
  
  // The integration time of the current program / measurement cycle is ~3ms. If a larger time
  // of integration is wanted, uncomment the next line:
  // delay(15);// <-- Add 15 ms integration time

}

// This function generates an outgoing clock pulse from the Arduino digital pin 'CLKpin'. This clock
// pulse is fed into pin 3 of the linear sensor:
void ClockPulse()
{
  //delay(1);
  arduino.digitalWrite(CLKpin, Arduino.HIGH);
  arduino.digitalWrite(CLKpin, Arduino.LOW);
}
