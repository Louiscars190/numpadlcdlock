#include <Arduino.h> // Done in Platform.io
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Servo.h>
#include <Keypad.h>

// Function Prototypes
void padInput(); 
void SerialInputCheck();
void doorSync();

LiquidCrystal lcd(A2,A3,4,5,6,7); // Initializes the LCD Display Object

Servo doorLock; // Initiliazes a Servo object

String incomingWords; // Declares a C++ string 

boolean newData = false, timer; // Initializes/Declares 2 boolean datatypes values

int i,b,v; // 3 integer variables declared. 
char password[15]; // Makes an array of size 15 that stores and compares the password to input

// This section makes the matrix for the number pad.
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = 
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {8, 9, 10, 11};  // Pins 8,9,10,11 used for rows
byte colPins[COLS] = {12, 13, A0, A1};  // Pins 12,13, A0, A1 for columns (not enough digital pins)

Keypad pad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); // Initializes keypad object

void setup() {
  // Begins serial connection at 9600 baudrate for user input and debugging.
  // Security guard controlling the door would have access to this.
  Serial.begin(9600);
  Serial.println("open = Unlock the Door");
  Serial.println("close = Locks the Door");
  Serial.setTimeout(10); // Makes the serial communcation faster by reducing dead-time
  
  // Initializes the lcd, and sets up the standard/basic things on the display.
  lcd.begin(16,2);
  lcd.print("Door Code: ");
  lcd.setCursor(0,1);
  lcd.print("Locked");

  i = 11; // Sets the cursor to 11th column to input 4 digit password
  b = 0; // b is the number of values inputed as a password
  v = 0; // number of values that coresspond to the correct password.

  // Attaches a the servo 'doorLock' that attaches to pin 3 and sets position to closed (0 degrees)
  doorLock.attach(3); 
  doorLock.write(0); 
}

void loop() 
{
  SerialInputCheck(); // Calls a function that checks if there is user input from serial monitor and 
  // sets the inputs to the string 'incomingWords, prints in serial monitor for debugging.
  if (b != 4) // This is the base scenario, when not all digits of 4 digit password has been entered.
  {
    padInput(); // Calls a function that writes the pad input to the display and waits for b = 4.
  }
  if (b == 4) // When the 4 digit password has been completely inputed. 
  {
    for (int j = 11; j < 15; j++) // Goes through the password stored in the 1 dimension array to see if the corresponding values are correct (ie. 1st digit, 2nd digit, 3rd digit, etc) in a for loop.
    {
      Serial.print(password[j]); // Prints 4 digit password to serial monitor for debugging and monitoring for the security guard.
      // Can add more passowrds by putting '||' in the if statement, which would still set the device to v = 4, making it easy for each person to have thier own password.
      if (j == 11 && password[j] == '1') v++;
      if (j == 12 && password[j] == '9') v++;
      if (j == 13 && password[j] == '2') v++;
      if (j == 14 && password[j] == '0') v++;
    }
    Serial.println();
    Serial.println(v); // Debugging, shows the number of correct values, collected from for loop.
    if (v != 4) // Displays wrong, waits, and clears the display if the inputed password is incorrect (v != 4)
    {
      lcd.setCursor(9,1);
      lcd.print("Wrong!");
      delay(2000);
      lcd.setCursor(9,1);
      lcd.print("      ");
      v = 0;
    }
    i = 11; // resets the cursor for following pad input function that uses i as cursor position.
    b = 0;
    lcd.setCursor(11,0); // Clears the 4 digit password field for retry.
    lcd.print("     ");
  }
  doorSync();
}

void SerialInputCheck() // Checks if there has been input from user through Serial Monitor. If yes, it puts the data into a string and prints it back out to serial for debug. It also it sets newData to true, which allows the door synd function to know to update the display in the ncessary fields.
{
  if (Serial.available() > 0)
  {
    incomingWords = Serial.readString();
    Serial.println(incomingWords);
    newData = true;
  }
}

void padInput() // Most important function in the program. Monitors the actions from the user from the 4x4 matrix numberpad. If # is pressed the field is cleared and reset to try again. 
{
  char KeypadCharacter = pad.getKey(); // Puts the inputed character into a local variable.
  if(KeypadCharacter)
  {
    if (KeypadCharacter == '#') // Clears the field and resets all values to retry.
    {
      lcd.setCursor(11,0);
      lcd.print("     ");
      b = 0;
      i = 11;
      v = 0;
      return;
    }
    else // If # is not pressed it prints out the input to the display jumping cursor position everytime so data is not overrided. i is the cursor position, and is a global variable so it can be changed by other functions to reset the field. +1 to b every iteration to keep track of the number of digits inputed in the 4 digit field. the main loop takes care of the logic for this.
    {
      lcd.setCursor(i,0);
      lcd.print(KeypadCharacter);
      Serial.println(KeypadCharacter);

      password[i] = KeypadCharacter; // Puts the current character into the password array, to be verified with the correct code in the main function.
      b++;
      i++;
    }
  }
}

void doorSync() // Actual physical mechanism
{
  if (newData == true && incomingWords == "open") // if there is new data coming from serial monitor that says open (user input like security guard or GUI), then the door is unlocked by writing the servo posiition to 90 degrees which unlocks it. It also changes the display to say Unlocked. This lasts indefinitely because it is meant for the security guard to control and not the client.
  {
      doorLock.write(90);
      lcd.setCursor(0,1);
      lcd.print("                ");
      delay(300);
      lcd.setCursor(0,1);
      lcd.print("Unlocked");
      v = 0;
      b = 0; // Resets the code field regardless and correct key, to ensure security.
      newData = false; // Ends the otherwise indefinite cycle of new data, to make sure it is only activated when there is new information because there is no longer new information. 
  }
  else if (v == 4 || (newData == true && incomingWords == "1920")) // Else if statement that takes care of the numeric input from the number pad and gives the option to the security guard to input the actual passowrd. This shows it to the client, which could be useful if the security guard wants to give the client the password. The password stays visiible for as long as the door is unlocked, as it emulates a client inputting the correct password (5 seconds unlocked)
  {
    if (incomingWords == "1920") 
    {
      lcd.setCursor(11,0);
      lcd.print("1920");
    }
    doorLock.write(90); // Unlocks the door
    lcd.setCursor(0,1);
    lcd.print("                ");
    delay(300);
    lcd.setCursor(0,1);
    lcd.print("Unlocked");
    delay(5000); // Waits 5 seconds so the user can have enough time to open the door and enter before it relocks itself.
    v = 0;
    b = 0; // Resets all field values.
    timer = true; // Once the 5 seconds is up, timer is set to true, which is equivalent to the security guard giving the close commnad, which updates the display to lock, clears the display of the password.
    newData = false;
    incomingWords = ' '; // Clears the incoming words so code only shows when security guard inputs.
  }
  else if ((newData == true && incomingWords == "close") || timer == true) // If either the security guard decides to lock the door (maybe an emergency) or the client has entered and the timer has finished, the door will now lock, clearing the display of any previous password and setting the display to locked mode. This allows the next user to input the password 
  {
    lcd.setCursor(0,1);
    lcd.print("                "); // Clears the line then writes Locked 
    lcd.setCursor(11,0);
    lcd.print("     ");
    doorLock.write(0);
    delay(350); // Enough time for the display to update without corrupting the internal memory.

    lcd.setCursor(0,1);
    lcd.print("Locked");
    newData = false;
    i = 11; // Resets the cursor position for the next user
    timer = false; // resets timer for next person.
  }
}