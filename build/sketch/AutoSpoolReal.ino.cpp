#include <Arduino.h>
#line 1 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
#include <MobaTools.h>
#include <MoToButtons.h>
#include <MoToTimer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//Menu Options
#define NUMBER_OF_MENU_OPTIONS 5

//SCL pin 21;
//SCB pin 20;

//Pins for spool dir and steps
int dirPinSpool = 29;
int stepPinSpool = 31;

//Pins for head dir and steps
int dirPinHead = 43;
int stepPinHead = 41;

//Pins for microstepping
int ms1Head = 35;
int ms2Head = 37;
int ms3Head = 39;

//Gear ratio between gear on motor and gear on spool
const float gearRatioSpool = 28.0/22; 

//Step count for spool motor
const int steps = 6400; 

//Steps per 1 revolution of the spool (not spool motor)
float stepsPerRevolutionSpool = steps *gearRatioSpool;

//Scaling factor for calculations using potentiometer value read
const float potentiometerReadScale = 1.0/1000;

//Speed ratio between distribution head motor and spool motor
float speedRatio = 0.253*2; 

//Max sppol rpm allowed
const int maxRpmSpool = 20;

//Ratio to convert rawMotorValue to rpm
const int rawValueRpmRatio = 810; 

//Potentiometer value used in updateSpeed
int potentiometerRawValue = 0; 

//Previous potentiometer value used in updateSpeed
int lastPotentiometerRawValue = 0; 

//Analog range of potentiometer
float analogRange = 665.0; 

//Minimum motor output rpm
int motorOutputLowerLimit = 1; 

//Maximum motor output rpm
int motorOutputUpperLimit = 20; 

//Motor output range used for speed calculations in updateSpeed()
int motorOutputRange = motorOutputUpperLimit - motorOutputLowerLimit;

//Rounds motorRawValue to nearest integer
int motorOutputResolution = 1; 

//Previous motorRawValue used in updateSpeed
int lastMotorRawValue = 0; 

//Constantly updating time since last change used in updateSpeed()
int timeSinceChange = 0; 

//Set time delay before speed is updated after potentiometer change
int motorOutputDelayMillis = 800; 

//Boolean to describe whether or not speed has been updated within updateSpeed function
bool valueUpdated = false; 

//Value used to calcuate speed output in updateSpeed() if time delay is complete
int motorOutputValue = 0; 

//Raw motor value used in updateSpeed() calculations
int motorRawValue = float(potentiometerRawValue)  / analogRange * motorOutputRange + motorOutputLowerLimit; 

//Speed of spool
int speedSteps = motorRawValue*100; 

//Speed of head
int speedStepsHead = speedSteps*speedRatio; 

//Speed of calibration
const int calibrationSpeed = 500000; 

//pins for buttons
int button1 = 13;
int button2 = 12;
int button3 = 11;
int button4 = 10;

//Directon of spin
int spinDir = 1;

//Pin of limit switch
int limitSwitch = 34;

//Position from limit swtich in mm
float positionMm = 0;

//Position when spooling is paused
float interuptedMm = 0;

//Index for different menus
int menuIndex[4] = {0};

//Bool for if pause button was hit during head movement
bool interupted = false;

//Ratio of motor steps to mm length (horizontal distance of one motor rotation divided by the steps per rotation)
int stepsToMm = 16.1/3200;

//Pins for powering and reading potentiometer
int potentiometerRead = A7;

//Value for controlling output delay
bool value_updated = false;

//Bool for if button is pressed down
bool buttonPressed[4] = {false};

//Bool for state of button, updated when button pressed during updateMainMenu() function
int buttonState[4] = {0};

//Pins for button input
int buttonPins[4] = {button1, button2, button3, button4};

//Associated values for states used in menuFunction()
int idleState = 0;
int spoolingState = 1;
int calibratePromptState = 2;
int calibrateState = 3;

//Ramp length for motor acceleration
int rampLen = 1;

//Bool conditions for spoolFilament()
bool calibrated = false;
bool idleBreakCalibrate = false;
bool idleBreakSpool = false;

//Assign steps and step direction to motors
MoToStepper stepperSpool(steps, STEPDIR); 
MoToStepper stepperHead(steps, STEPDIR);

//Run calibration
#line 158 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void calibrate();
#line 200 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void idle();
#line 225 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void rightPauseInterupted();
#line 235 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void rightPause();
#line 244 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void leftPauseInterupted();
#line 254 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void leftPause();
#line 263 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void moveHeadRight();
#line 300 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void moveHeadLeft();
#line 334 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void moveHeadRightInterupted();
#line 366 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void moveHeadLeftInterupted();
#line 399 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void spoolFilament();
#line 441 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void runCalibration();
#line 452 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void clearCalibration();
#line 463 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void promptCalibration();
#line 478 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void manualHeadControl();
#line 493 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void updateSpeed();
#line 523 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void updateMainMenu();
#line 571 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void menuFunction();
#line 642 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void setup();
#line 696 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void loop();
#line 158 "C:\\Users\\Joshua\\OneDrive - University of Waterloo\\Documents\\School\\Co-op\\ALT TEX\\Projects\\AutoSpoolReal\\AutoSpoolReal.ino"
void calibrate()
{
    lcd.setCursor(0,0);
    lcd.println("Calibrating");
    //while limit switch is not pressed, head motor will run head towards switch
    while(digitalRead(limitSwitch) == 0)
    {
        Serial.println(digitalRead(limitSwitch));
        stepperHead.setSpeedSteps(calibrationSpeed);
        stepperHead.rotate(-1);
    }
    //when limit switch is hit, motor stops and position is zero-ed
    stepperHead.setZero(0);
    stepperHead.setZero();
    stepperHead.moveTo(0);
    positionMm = 0;

    //Moves to 5mm to right of switch
    while(positionMm < 5)
    {
        Serial.println(digitalRead(limitSwitch));
        stepperHead.setSpeedSteps(calibrationSpeed);
        positionMm = stepperHead.readSteps() * 8.0/3200.0;
        stepperHead.rotate(1);
    }
    //motors stop, position is zero-ed
    stepperHead.setZero();
    stepperHead.moveTo(0);
    positionMm = 0;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.println("Calibrated");
    delay(2000);

    //booleans set for potential state changes
    menuIndex[idleState] == true;
    spinDir = -1;
    calibrated = true;
}

//Runs idle loop until button inputs exit state
void idle()
{
    while(digitalRead(buttonPins[1]) == false)
    {

        if(digitalRead(buttonPins[1]) == true || digitalRead(buttonPins[0]) == true)
        {
            updateMainMenu();
            break;
        }
    }
    if(buttonState[0] == true)
    {
        //Sets boolean for idle to go into calibration state
        idleBreakCalibrate = true;
    }
    
    if(buttonState[1] == true)
    {
        //Sets boolean for idle to go into spooling state
        idleBreakSpool = true;
    }
}

//Stops motors and saves position values
void rightPauseInterupted()
{
    stepperHead.rotate(0);
    stepperSpool.rotate(0);
    spinDir = -1;
    interupted = true;
    interuptedMm = positionMm;
}

//Stops motors, assigns direction switch
void rightPause()
{
    stepperHead.rotate(0);
    stepperSpool.rotate(0);
    spinDir = 1;
    interupted = false;
}

//Stops motors and saves position values
void leftPauseInterupted()
{
    stepperHead.rotate(0);
    stepperSpool.rotate(0);
    spinDir = 1;
    interupted = true;
    interuptedMm = positionMm;
}

//Stops motors, assigns direction switch
void leftPause()
{
    stepperHead.rotate(0);
    stepperSpool.rotate(0);
    spinDir = -1;
    interupted = false;
}

//Moves head to the right until limit reached, then sets boolean to switch direction
void moveHeadRight()
{
    stepperHead.setZero(0);
    //Serial.println(stepperHead.readSteps());
    while(positionMm <= 103)
    {
        updateMainMenu();
        updateSpeed();
        stepperSpool.rotate(1);
        stepperSpool.setSpeedSteps(speedSteps);
        stepperHead.rotate(1);
        stepperHead.setSpeedSteps(speedStepsHead);
        positionMm = stepperHead.readSteps() * 8.0/3200.0;
        lcd.setCursor(10,1);
        lcd.println(motorRawValue);
        //if pause is pressed, head stops, boolean set to enter moveHeadRightInterupted when unpaused, position value saved
        if(buttonState[1] == true)
        {
            interupted == true;
            interuptedMm = stepperHead.readSteps();
            break;
        }
    
    }
 
    if(buttonState[1] == true)
    {
        rightPauseInterupted();
    }

    if(buttonState[1] == false)
    {
        rightPause();
    }
}

//Moves head to the left until limit reached, then sets boolean to switch direction
void moveHeadLeft()
{
    while(positionMm > 0)
    {
        updateMainMenu();
        updateSpeed();
        stepperSpool.rotate(1);
        stepperSpool.setSpeedSteps(speedSteps);
        stepperHead.rotate(-1);
        stepperHead.setSpeedSteps(speedStepsHead);
        positionMm = stepperHead.readSteps() * 8.0/3200.0;
        lcd.setCursor(10,1);
        lcd.println(motorRawValue);
        //if pause pressed, motor stops
        if(buttonState[1] == true)
        {
            break;
        }
    
    }
    
    if(buttonState[1] == true)
    {
        leftPauseInterupted();
    }

    if(buttonState[1] == false)
    {
        leftPause();
    }

}

//Moves head to the right remaining distance after being paused
void moveHeadRightInterupted()
{
    while(positionMm <= 103)
    {
        updateMainMenu();
        updateSpeed();
        stepperSpool.rotate(1);
        stepperSpool.setSpeedSteps(speedSteps);
        stepperHead.rotate(1);
        stepperHead.setSpeedSteps(speedStepsHead);
        positionMm = stepperHead.readSteps() * 8.0/3200.0;
        lcd.setCursor(10,1);
        lcd.println(motorRawValue);
        //if pause pressed, motor stops
        if(buttonState[1] == true)
        {
            break;
        }
    }

    if(buttonState[1] == true)
    {
        rightPauseInterupted();
    }

    if(buttonState[1] == false)
    {
        rightPause();
    }
}

//Moves head to the left remaining distance after being paused
void moveHeadLeftInterupted()
{
    while(positionMm >= 0)
    {
        updateMainMenu();
        updateSpeed();
        stepperSpool.rotate(1);
        stepperSpool.setSpeedSteps(speedSteps);
        stepperHead.rotate(-1);
        
        positionMm = stepperHead.readSteps() * 8.0/3200.0;
        lcd.setCursor(10,1);
        lcd.println(motorRawValue);
        //if pause pressed, motor stops
        if(buttonState[1] == true)
        {
            break;
        }
    }

    if(buttonState[1] == true)
    {
        leftPauseInterupted();
    }

    if(buttonState[1] == false)
    {
        leftPause();
    }
        
}

//Function for spooling state. Can be interupted by pressing pause
void spoolFilament()
{
    calibrated = false;

    while(buttonState[1] == false)
    {
        if(interupted == false)
        {
            if(spinDir == -1)
            {
                Serial.println("moveRight");
                moveHeadRight();
            }

            else if(spinDir == 1)
            {
                Serial.println("moveLeft");
                moveHeadLeft();
            }

        }
        else if(interupted == true)
        {
            interupted = false;
            if(spinDir == -1)
            {
                Serial.println("moveRightInt");
                moveHeadRightInterupted();
            }

            else if(spinDir == 1)
            {
                Serial.println("moveLeftInt");
                moveHeadLeftInterupted();
            }
            
            }
    }

}

//Runs calibration function
void runCalibration()
{
    lcd.clear();
    //sets booleans for state transition in menuFunction()
    menuIndex[calibrateState] = true;
    menuIndex[calibratePromptState] = false;
    calibrate();
    buttonState[1] = false;
}

//Function that cancels calibration request and returns state to main menu
void clearCalibration()
{
    lcd.clear();
    //sets booleans for state transition in menuFunction()
    menuIndex[calibrateState] = false;
    menuIndex[calibratePromptState] = false;
    menuIndex[spoolingState] = true;
    buttonState[0] = false;
}

//Function that prompts user on whether or not they would like to calibrate
void promptCalibration()
{
    lcd.clear();
    //sets boolean for state transition in menuFunction()
    menuIndex[calibratePromptState] = true;
    lcd.setCursor(0,0);
    lcd.print("Calibrate?");
    lcd.setCursor(0,1);
    lcd.print("Manual Cont < >");
    //clears button states for input
    buttonState[0] = false;
    buttonState[1] = false;
}

//Function for manually controlling head movement left and right. Holding right button moves right, holding left button moves left
void manualHeadControl()
{
    while(digitalRead(buttonPins[2]) == true)
    {
        stepperHead.rotate(-1);
    }
    stepperHead.rotate(0);
    stepperSpool.rotate(0);
    while(digitalRead(buttonPins[3]) == true)
    {
        stepperHead.rotate(1);
    }
}

//Function that checks potentiometer readings and updates the speed of the spool and head accordingly
void updateSpeed()
{
    //Takes potentiometer reading and scales it to associated motor values
    potentiometerRawValue = analogRead(potentiometerRead);
    motorRawValue = (potentiometerRawValue)/ analogRange * motorOutputRange + motorOutputLowerLimit;
    motorRawValue = motorRawValue / motorOutputResolution * motorOutputResolution;

    //If potentiometer value doesn't change, time since the last change is saved and boolean proclaiming that the value has not been changed is set
    if(lastMotorRawValue != motorRawValue)
    {
        timeSinceChange = millis();
        valueUpdated = false;
    }

    //If potentiometer value changes and a sufficent amount of time since the last change has passed, motor value will update
    if(millis() - timeSinceChange > motorOutputDelayMillis && valueUpdated == false)
    {
        motorOutputValue = motorRawValue;
        speedSteps = motorOutputValue * rawValueRpmRatio;
        speedStepsHead = speedSteps*speedRatio;
        valueUpdated = true;
    }

    //Current values set to previous values for next loop
    lastMotorRawValue = motorRawValue;
    lastPotentiometerRawValue = potentiometerRawValue;
    Serial.println(analogRead(potentiometerRawValue));
}

//Main function that checks button inputs and updates screen and runs functions accordingly
void updateMainMenu()
{
    if(digitalRead(buttonPins[0]) == 1 || digitalRead(buttonPins[1]) == 1 || digitalRead(buttonPins[2]) == 1 || digitalRead(buttonPins[3]) == 1)
    {
        // Get button states
        for (int i = 0; i < 4; i++) {
            buttonState[i] = digitalRead(buttonPins[i]);
        }
        
        if (buttonState[0] == 1 && buttonPressed[0] != true) {
            buttonPressed[0] = true;
            
        }
        if (buttonState[0] == 0) {
            buttonPressed[0] = false;
        }

        if (buttonState[1] == 1 && buttonPressed[1] != true) {
            buttonPressed[1] = true;
        
        }
        if (buttonState[1] == 0) {
            buttonPressed[1] = false;
        }

        if (buttonState[2] == 1 && buttonPressed[2] != true) {
            buttonPressed[2] = true;
            
        }
        if (buttonState[2] == 0) {
            buttonPressed[2] = false;
        }

        if (buttonState[3] == 1 && buttonPressed[3] != true) {
            buttonPressed[3] = true;
            
        }
        if (buttonState[3] == 0) {
            buttonPressed[3] = false;
        }
        delay(200);
    }
    
    else{}

}

//Takes button inputs and executes actions based on if statement tree. Button inputs and previous states are considered in if statements
void menuFunction()
{
    //IDLE after calibration state
    if(buttonState[0,1,2,3] == false && menuIndex[idleState] == true && idleBreakSpool == false || calibrated == true)
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.println("Press Play");
        lcd.setCursor(0,1);
        lcd.println("to Begin");
        idle();
    }

    //Initiate spooling state
    if(buttonState[1] == true && menuIndex[idleState] == true || idleBreakSpool == true)
    {
        idleBreakSpool = false;
        lcd.clear();
        menuIndex[spoolingState] = true;
        buttonState[1] = false;
        lcd.setCursor(0,0);
        lcd.print("Spooling");
        lcd.setCursor(0,1);
        lcd.print("Spool rpm");
        spoolFilament();
    }

    //if "X" is pressed and not already in menu, calibration prompt
    if(buttonState[0] == true && menuIndex[idleState] == true || idleBreakCalibrate == true) 
    {
        idleBreakCalibrate = 0;
        promptCalibration();
        while(buttonState[1] == false)
        {
            manualHeadControl();
            if(digitalRead(buttonPins[1]) == true || digitalRead(buttonPins[0]))
            {
                break;
            }
        }
    }

    if(menuIndex[calibratePromptState] == true) 
    {
        //Play selected when prompting for calibration, run calibration
        if(buttonState[0] == true)
        {
            runCalibration();
        }

        //Back selected when prompting for calibration, back to idle
        if(buttonState[1] == true)
        {
            clearCalibration();
        }
    }

    if(buttonState[1] == true && menuIndex[spoolingState] == true)
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Paused...");
        lcd.setCursor(0,1);
        lcd.print("Play to cont'");
        idle();
    }
    

}

//Setup function
void setup()
{
    //initialize serial communication
    Serial.begin(9600); 

    //initialize motor output pins
    pinMode(stepPinHead, OUTPUT);
    pinMode(stepPinSpool, OUTPUT);
    pinMode(dirPinHead, OUTPUT);
    pinMode(stepPinSpool, OUTPUT);
    pinMode(potentiometerRead, INPUT);

    //set microsteps
    digitalWrite(ms1Head, HIGH);
    digitalWrite(ms2Head, HIGH);
    digitalWrite(ms3Head, HIGH);

    //initialize the lcd
    lcd.init();  
    //open the backlight
    lcd.backlight();  

    //Set Pin Modes
    pinMode(limitSwitch, INPUT);
    pinMode(potentiometerRead, INPUT);

    //initialize motors
    stepperSpool.attach(stepPinSpool, dirPinSpool);
    stepperSpool.setSpeedSteps(speedSteps);
    stepperHead.attach(stepPinHead, dirPinHead);
    stepperHead.setSpeedSteps(speedStepsHead);
    stepperHead.setRampLen(rampLen);
    stepperSpool.setRampLen(rampLen);

    //setup buttons
    pinMode(button1, INPUT);
    pinMode(button2, INPUT);
    pinMode(button3, INPUT);
    pinMode(button4, INPUT);

    //initiate calibration
    calibrate();

    //set pinMode for buttons
    for (int i = 0; i < 4; i++) 
    {       
        pinMode(buttonPins[i], INPUT);
    } 

    delay(20);

}

//Main loop
void loop()
{

    //Updates button states based on user input
    updateMainMenu();

    while(digitalRead(buttonPins[0]) == false && digitalRead(buttonPins[1]) == false && digitalRead(buttonPins[2]) == false && digitalRead(buttonPins[3]) == false)
    {
        // Takes button inputs and executes actions based on if statement tree. Button inputs and previous states are considered in if statements
        menuFunction(); 
    }
    
}
