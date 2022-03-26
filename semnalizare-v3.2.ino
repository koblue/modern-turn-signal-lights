//Modern turn lights v.3.2.0 for scooters and motorcycles with arduino and ws2812b - by Fedaceag Ionut ( Youtube - Think small, build big! )
#include <FastLED.h>                        //FastLed library version 3.2.1 - https://github.com/FastLED/FastLED/wiki/Overview or http://fastled.io/ with NEOPIXEL or WS2812B
#define NUM_STRIPS 4                        // number of small led strips

#define NUM_LEDS_PART_A 50                  // number of leds for the big strip - should be the at least the number of leds from the small strips multiply by tree (in this case the minimum should be 10 x 3 = 30 leds)
#define NUM_LEDS_PART_B 10                  // number of leds for the small strips
#define NUM_LEDS_PART_C 3                   // number of leds for dashboard

CRGB ledsA[NUM_LEDS_PART_A];
CRGB ledsB[NUM_STRIPS][NUM_LEDS_PART_B];
CRGB ledsC[NUM_LEDS_PART_C];

//input pins
const int buttonPinL = 2;                   // turn left
const int buttonPinR = 3;                   // turn right
const int buttonPinEng = 4;                 // engine on to start day lights
const int buttonPinKnightRider = 5;         // knight rider lights
const int buttonPinBrake = 6;               // brake lights
const int buttonPinTailLights = 7;          // tail lights

int halfOfLeds = (NUM_LEDS_PART_A/2);
int buttonStateL = 0;
int buttonStateR = 0;
int engineOn = 0;
int maxBrtN = 0;
int KnightRiderState = 0;
int KnightRiderToOff = 0;
int Brake = 0;
int TailLights = 0;
int leftLeds = 0;
int rightLeds = 0;
int tailLeft = halfOfLeds-1;
int tailLeftSmall = 0;
int tailRight = halfOfLeds;
int tailRightSmall = 0;
int brakeToOffL = 0;
int brakeToOffR = 0;

int stateLT = 0;
int stateRT = 0;
uint8_t gBrtL = 0;
uint8_t gBrtR = 0;
int TailOn = 0;

// The RGB color codes can be found here https://github.com/FastLED/FastLED/wiki/Pixel-reference
unsigned long turnColor = 0xff6a00;                            // turn signal color
unsigned long dashTurnColor = 0x008000;                        // dashboard turn signal color
unsigned long dashHazardColor = 0xff6a00;                      // dashboard hazard light color
unsigned long dashDayColor = 0xFFFFFF;                         // dashboard day light color
unsigned long dashKnightRiderColor = 0xFF0000;                 // dashboard knight rider color
unsigned long dashTailColor = 0x0000FF;                        // dashboard tail light color

int showSignalBigStrip = 1;                                    // show the signal on the big led strip 0 = no | 1 = yes
int showTailBrakeSmalStrip = 1;                                // show tail and brake lights on small strips 0 = no | 1 = yes
int maxBrt = 254;                                              // maxim brightness day lights - from 0 to 254
int maxBrtNight = 100;                                         // maxim brightness day lights with tail lights on - from 0 to 254
int maxBrtBrake = 254;                                         // maxim brightness Brake lights - from 0 to 254
int maxBrtTailLights = 70;                                     // maxim brightness tail lights - from 0 to 254
uint8_t gHue = 0;                                              // knight rider color
int knightRiderRainbow = 0;                                    // knight rider rainbow effect 0 = no | 1 = yes
int knightRiderOvrDayL = 1;                                    // knight rider override day lights 0 = no | 1 = yes
int knightRiderSpeed = 45;                                     // speed for knight rider lights
int knightRiderTail = 12;                                      // tail effect for knight rider lights

int delayTurnLedAnim = 8;                                      //delay of each led in turn light animation
int delayTurnLedOff = 250;                                     //delay from animation to black (is used twice)
int delayLedToDayLight = 500;                                  //delay from animation to day light on
int tailDelay = 6;                                             //delay for animation tail lights on or off
int nrAnimAfterOff = 3;                                        //number of animations for a single impulse

unsigned long currentMillis = 0; 
unsigned long previousMillis = 0;

int delayForHazard = 500;                                      // delay in milliseconds for hazard light combination
unsigned long currentMillisSC = 0; 
unsigned long previousMillisSC = 0;

void setup() { 
  Serial.begin(115200);
  pinMode(buttonPinL, INPUT);
  pinMode(buttonPinR, INPUT);
  pinMode(buttonPinEng, INPUT);
  pinMode(buttonPinKnightRider, INPUT);
  pinMode(buttonPinBrake, INPUT);
  pinMode(buttonPinTailLights, INPUT);
        
  FastLED.addLeds<NEOPIXEL, 13>(ledsA, NUM_LEDS_PART_A);    //big led strip rear turn signals, brake light, tail lights
  FastLED.addLeds<NEOPIXEL, 12>(ledsB[0], NUM_LEDS_PART_B); //led strip rear left  turn signals
  FastLED.addLeds<NEOPIXEL, 11>(ledsB[1], NUM_LEDS_PART_B); //led strip rear right turn signals
  FastLED.addLeds<NEOPIXEL, 10>(ledsB[2], NUM_LEDS_PART_B); //led strip front left  turn signals
  FastLED.addLeds<NEOPIXEL, 9>(ledsB[3], NUM_LEDS_PART_B);  //led strip front right turn signals
  FastLED.addLeds<NEOPIXEL, 8>(ledsC, NUM_LEDS_PART_C);     //led strip dashboard
  
  attachInterrupt(digitalPinToInterrupt(buttonPinL),btnPressL,RISING); // we use interrupt for instant reaction of turn lights
  attachInterrupt(digitalPinToInterrupt(buttonPinR),btnPressR,RISING); // we use interrupt for instant reaction of turn lights

  fill_solid(ledsA, NUM_LEDS_PART_A, CRGB::Black);    // some led strips are all on at power on, so let's power them off at boot
  fill_solid(ledsB[0], NUM_LEDS_PART_B, CRGB::Black); // some led strips are all on at power on, so let's power them off at boot
  fill_solid(ledsB[1], NUM_LEDS_PART_B, CRGB::Black); // some led strips are all on at power on, so let's power them off at boot
  fill_solid(ledsB[2], NUM_LEDS_PART_B, CRGB::Black); // some led strips are all on at power on, so let's power them off at boot
  fill_solid(ledsB[3], NUM_LEDS_PART_B, CRGB::Black); // some led strips are all on at power on, so let's power them off at boot
  fill_solid(ledsC, NUM_LEDS_PART_C, CRGB::Black);    // some led strips are all on at power on, so let's power them off at boot
  
  FastLED.show();
}

void loop() {
  // read the input state
  buttonStateL = digitalRead(buttonPinL); 
  buttonStateR = digitalRead(buttonPinR);
  engineOn = digitalRead(buttonPinEng);
  KnightRiderState = digitalRead(buttonPinKnightRider);

  currentMillisSC = millis();
  
  if(buttonStateL == HIGH){
    stateLT = 1;
    previousMillisSC = currentMillisSC;
  }else{
    if(currentMillisSC-previousMillisSC >= delayForHazard && stateLT >= nrAnimAfterOff && stateRT == LOW){
      stateLT = 0;  
    }
  }

  if(buttonStateR == HIGH){
    stateRT = 1;
    previousMillisSC = currentMillisSC;
  }else{
    if(currentMillisSC-previousMillisSC >= delayForHazard && stateRT >= nrAnimAfterOff && stateLT == LOW){
      stateRT = 0;  
    }
  }
  
  //function for hazard lights
  if(stateLT != 0 && stateRT != 0){
    
    leftLeds = NUM_LEDS_PART_B-1;
    rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;

    for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {

        if(showSignalBigStrip == 1){
          ledsA[leftLeds] = turnColor;
          ledsA[rightLeds] = turnColor;          
        }
      
        ledsB[0][dot] = turnColor;
        ledsB[1][dot] = turnColor;
        ledsB[2][dot] = turnColor; 
        ledsB[3][dot] = turnColor;

        ledsC[0] = dashHazardColor;
        ledsC[2] = dashHazardColor;
      
        FastLED.show();
        //delay(delayTurnLedAnim);
        currentMillis = previousMillis = millis();
        while(previousMillis + delayTurnLedAnim >= currentMillis){
          TailAndBrake();
          FastLED.show();
          currentMillis = millis();
        }
        leftLeds--;
        rightLeds++;
    }
    
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }

    if(showSignalBigStrip == 1){
      
      leftLeds = NUM_LEDS_PART_B-1;
      for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {
          ledsA[leftLeds] = 0x000000;          
          leftLeds--;
      }      
      rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
      for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {
          ledsA[rightLeds] = 0x000000;       
          rightLeds++;
      }
      
    }
    fill_solid(ledsB[0], NUM_LEDS_PART_B, CRGB::Black);
    fill_solid(ledsB[1], NUM_LEDS_PART_B, CRGB::Black);
    fill_solid(ledsB[2], NUM_LEDS_PART_B, CRGB::Black);
    fill_solid(ledsB[3], NUM_LEDS_PART_B, CRGB::Black);
    ledsC[0] = CRGB::Black;
    ledsC[2] = CRGB::Black;
    FastLED.show();
    
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }

    buttonStateL = digitalRead(buttonPinL); 
    buttonStateR = digitalRead(buttonPinR);
    stateLT++;
    stateRT++;
    if(buttonStateL != HIGH && buttonStateR != HIGH && stateLT >= nrAnimAfterOff && stateRT >= nrAnimAfterOff){
      
      if(buttonStateL == HIGH){
        stateLT = 1;
      }else{
        stateLT = 0;
        gBrtL = 0;
      }
      
      if(buttonStateR == HIGH){
        stateRT = 1;
      }else{
        stateRT = 0;
        gBrtR = 0;
      }
      
      if(buttonStateL != HIGH && buttonStateR != HIGH){
        //delay(delayLedToDayLight);           
        currentMillis = previousMillis = millis();
        while(previousMillis + delayLedToDayLight >= currentMillis){
          TailAndBrake();
          FastLED.show();
          currentMillis = millis();
        }       
        
        leftLeds = NUM_LEDS_PART_B-1;
        rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
        
        if(showSignalBigStrip == 1){
          tailLeft = NUM_LEDS_PART_B;
          tailRight = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
        }else{
          tailLeft = halfOfLeds-1;
          tailRight = halfOfLeds;
        }
      }
    }    

  //function for left turn lights
  }else if(stateLT != 0){
    
    if(KnightRiderState == HIGH){
      fill_solid(ledsB[1], NUM_LEDS_PART_B, CRGB::Black);
      fill_solid(ledsB[3], NUM_LEDS_PART_B, CRGB::Black);
    }
    
    leftLeds = NUM_LEDS_PART_B-1;
    for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {

        if(showSignalBigStrip == 1){
          ledsA[leftLeds] = turnColor;          
        }

        ledsB[0][dot] = turnColor;
        ledsB[2][dot] = turnColor;
        ledsC[0] = dashTurnColor;
        
        FastLED.show();
        //delay(delayTurnLedAnim);
        currentMillis = previousMillis = millis();
        while(previousMillis + delayTurnLedAnim >= currentMillis){
          TailAndBrake();
          FastLED.show();
          currentMillis = millis();
        }
        leftLeds--;
    }
    
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }

    leftLeds = NUM_LEDS_PART_B-1;
    for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {
        if(showSignalBigStrip == 1){
          ledsA[leftLeds] = 0x000000;          
        }
        ledsB[0][dot] = 0x000000;
        ledsB[2][dot] = 0x000000;
        ledsC[0] = 0x000000;
        
        leftLeds--;
    }

    FastLED.show();
    
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }

    stateLT++;
    if(stateLT >= nrAnimAfterOff && buttonStateL != HIGH){
      stateLT = 0;
      gBrtL = 0;
      //delay(delayLedToDayLight);           
      currentMillis = previousMillis = millis();
      while(previousMillis + delayLedToDayLight >= currentMillis){
        TailAndBrake();
        FastLED.show();
        currentMillis = millis();
      }      
      leftLeds = NUM_LEDS_PART_B-1;
      if(showSignalBigStrip == 1){
        tailLeft = NUM_LEDS_PART_B;
        tailRight = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
      }else{
        tailLeft = halfOfLeds-1;
        tailRight = halfOfLeds;
      }
    }

  //function for right turn lights
  }else if(stateRT != 0){
       
    if(KnightRiderState == HIGH){
        fill_solid(ledsB[0], NUM_LEDS_PART_B, CRGB::Black);
        fill_solid(ledsB[2], NUM_LEDS_PART_B, CRGB::Black);
    }
    
    rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
    for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {
        if(showSignalBigStrip == 1){
          ledsA[rightLeds] = turnColor;          
        }

        ledsB[1][dot] = turnColor;
        ledsB[3][dot] = turnColor;
        ledsC[2] = dashTurnColor;
        
        FastLED.show();
        //delay(delayTurnLedAnim);
        currentMillis = previousMillis = millis();
        while(previousMillis + delayTurnLedAnim >= currentMillis){
          TailAndBrake();
          FastLED.show();
          currentMillis = millis();
        }
        rightLeds++;
    }
    
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }
    
    rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
    for(int dot = 0; dot < NUM_LEDS_PART_B; dot++) {
        if(showSignalBigStrip == 1){
          ledsA[rightLeds] = 0x000000;          
        }
        
        ledsB[1][dot] = 0x000000;
        ledsB[3][dot] = 0x000000;
        ledsC[2] = 0x000000;
        
        rightLeds++;
    }
    
    FastLED.show();
    //delay(delayTurnLedOff);       
    currentMillis = previousMillis = millis();
    while(previousMillis + delayTurnLedOff >= currentMillis){
      TailAndBrake();
      FastLED.show();
      currentMillis = millis();
    }

    stateRT++;
    if(stateRT >= nrAnimAfterOff && buttonStateR != HIGH){
      stateRT = 0;
      gBrtR = 0;
      //delay(delayLedToDayLight);
      currentMillis = previousMillis = millis();
      while(previousMillis + delayLedToDayLight >= currentMillis){
        TailAndBrake();
        FastLED.show();
        currentMillis = millis();
      }
      rightLeds = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
      if(showSignalBigStrip == 1){
        tailLeft = NUM_LEDS_PART_B;
        tailRight = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
      }else{
        tailLeft = halfOfLeds-1;
        tailRight = halfOfLeds;          
      }
    }

  //function for kight rider and day lights
  }else{

      if(knightRiderOvrDayL == 1 && KnightRiderState == HIGH && engineOn == HIGH){
        engineOn = LOW;
      }
      
      if(TailLights == HIGH && engineOn == HIGH){
        maxBrtN = maxBrtNight;
      }else{
        maxBrtN = maxBrt;
      }

      if(stateLT == 0 && engineOn == HIGH){
        if(gBrtL <= maxBrtN){
          EVERY_N_MILLISECONDS( 1 ) { gBrtL++; }
          fill_solid( ledsB[2], NUM_LEDS_PART_B, CHSV(0,0,gBrtL));
        }else if(gBrtL >= maxBrtN){
          EVERY_N_MILLISECONDS( 1 ) { gBrtL--; }
          fill_solid( ledsB[2], NUM_LEDS_PART_B, CHSV(0,0,gBrtL));
        }
      }else{
        if(gBrtL > 0){
          EVERY_N_MILLISECONDS( 1 ) { gBrtL--; }
          fill_solid( ledsB[2], NUM_LEDS_PART_B, CHSV(0,0,gBrtL));
        }
      }
      
      if(stateRT == 0 && engineOn == HIGH){
        if(gBrtR <= maxBrtN){
          EVERY_N_MILLISECONDS( 1 ) { gBrtR++; }
          fill_solid( ledsB[3], NUM_LEDS_PART_B, CHSV(0,0,gBrtR));
        }else if(gBrtR >= maxBrtN){
          EVERY_N_MILLISECONDS( 1 ) { gBrtR--; }
          fill_solid( ledsB[3], NUM_LEDS_PART_B, CHSV(0,0,gBrtR));
        }
      }else{
        if(gBrtR > 0){
          EVERY_N_MILLISECONDS( 1 ) { gBrtR--; }
          fill_solid( ledsB[3], NUM_LEDS_PART_B, CHSV(0,0,gBrtR));
        }
      }
        
      if(KnightRiderState == HIGH && engineOn == LOW){
        if(knightRiderRainbow == 1){
          EVERY_N_MILLISECONDS( 20 ) { gHue++; }
        }
        fadeToBlackBy( ledsB[0], NUM_LEDS_PART_B, knightRiderTail);
        fadeToBlackBy( ledsB[1], NUM_LEDS_PART_B, knightRiderTail);
        fadeToBlackBy( ledsB[2], NUM_LEDS_PART_B, knightRiderTail);
        fadeToBlackBy( ledsB[3], NUM_LEDS_PART_B, knightRiderTail);
        int pos = beatsin16( knightRiderSpeed, 0, NUM_LEDS_PART_B-1 );
        ledsB[0][pos] += CHSV( gHue, 255, 192);
        ledsB[1][pos] += CHSV( gHue, 255, 192);
        ledsB[2][pos] += CHSV( gHue, 255, 192);
        ledsB[3][pos] += CHSV( gHue, 255, 192);
        KnightRiderToOff = 1;
      }else{
        if(KnightRiderToOff == 1){
          fill_solid(ledsB[0], NUM_LEDS_PART_B, CRGB::Black);
          fill_solid(ledsB[1], NUM_LEDS_PART_B, CRGB::Black);
          fill_solid(ledsB[2], NUM_LEDS_PART_B, CRGB::Black);
          fill_solid(ledsB[3], NUM_LEDS_PART_B, CRGB::Black);
          if(showSignalBigStrip == 1){
            tailLeft = NUM_LEDS_PART_B;
            tailRight = NUM_LEDS_PART_A-NUM_LEDS_PART_B;
          }else{
            tailLeft = halfOfLeds-1;
            tailRight = halfOfLeds;          
          }
          KnightRiderToOff = 0;
        }
      }

    TailAndBrake();
    FastLED.show();
  }
}

//function for Brake lights, tail lights
void TailAndBrake(){
  
  Brake = digitalRead(buttonPinBrake);
  TailLights = digitalRead(buttonPinTailLights);
    
    if(TailLights == HIGH && Brake == LOW){
      
      if(stateLT == 0 && stateRT == 0){
        
        if(tailLeft >= 0){
          EVERY_N_MILLISECONDS( tailDelay ) {
            ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
            if(showTailBrakeSmalStrip == 1 && tailLeft < NUM_LEDS_PART_B){
              tailLeftSmall = map(tailLeft,NUM_LEDS_PART_B-1,0,0,NUM_LEDS_PART_B-1);
              ledsB[0][tailLeftSmall]=CHSV(0,255,maxBrtTailLights);              
            }
            tailLeft--;
          }
        }
        
        if(tailRight < NUM_LEDS_PART_A){
          EVERY_N_MILLISECONDS( tailDelay ) {
            ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
            if(showTailBrakeSmalStrip == 1 && tailRight > NUM_LEDS_PART_A-NUM_LEDS_PART_B-1){
              tailRightSmall = map(tailRight,NUM_LEDS_PART_A-NUM_LEDS_PART_B-1,NUM_LEDS_PART_A-1,0,NUM_LEDS_PART_B-1);
              ledsB[1][tailRightSmall]=CHSV(0,255,maxBrtTailLights);              
            }
            tailRight++;
          }
        }
        
      }else if(stateLT != 0 && stateRT != 0){

        if(showSignalBigStrip == 1){
          if(tailLeft >= NUM_LEDS_PART_B){
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
              tailLeft--;
            }
          }
          
          if(tailRight < NUM_LEDS_PART_A-NUM_LEDS_PART_B){
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
              tailRight++;
            }
          }
                          
        }else{
          if(tailLeft >= 0){
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
              tailLeft--;
            }
          }
          
          if(tailRight < NUM_LEDS_PART_A){
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
              tailRight++;
            }
          }
        }
       
      }else{
        
        if(stateLT != 0){

        if(showSignalBigStrip == 1){
          if(tailLeft >= NUM_LEDS_PART_B){   
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
              tailLeft--;
            }
          }          
        }else{
          if(tailLeft >= 0){   
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
              tailLeft--;
            }
          }
        }

          if(tailRight < NUM_LEDS_PART_A){ 
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
              if(showTailBrakeSmalStrip == 1 && tailRight > NUM_LEDS_PART_A-NUM_LEDS_PART_B-1){
                tailRightSmall = map(tailRight,NUM_LEDS_PART_A-NUM_LEDS_PART_B-1,NUM_LEDS_PART_A-1,0,NUM_LEDS_PART_B-1);
                ledsB[1][tailRightSmall]=CHSV(0,255,maxBrtTailLights);              
              }
              tailRight++;
            }
          }
        }
        
        if(stateRT != 0){

          if(tailLeft >= 0){   
            EVERY_N_MILLISECONDS( tailDelay ) {
              ledsA[tailLeft]=CHSV(0,255,maxBrtTailLights);
              if(showTailBrakeSmalStrip == 1 && tailLeft < NUM_LEDS_PART_B){
                tailLeftSmall = map(tailLeft,NUM_LEDS_PART_B-1,0,0,NUM_LEDS_PART_B-1);
                ledsB[0][tailLeftSmall]=CHSV(0,255,maxBrtTailLights);              
              }
              tailLeft--;
            }
          }

          if(showSignalBigStrip == 1){
            if(tailRight < NUM_LEDS_PART_A-NUM_LEDS_PART_B){ 
              EVERY_N_MILLISECONDS( tailDelay ) {
                ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
                tailRight++;
              }
            }            
          }else{
            if(tailRight < NUM_LEDS_PART_A){ 
              EVERY_N_MILLISECONDS( tailDelay ) {
                ledsA[tailRight]=CHSV(0,255,maxBrtTailLights);
                tailRight++;
              }
            }
          }
        }
      }
           
    }else if(Brake == HIGH){
      
      if(stateLT == 0 && stateRT == 0){
        fill_solid( ledsA, NUM_LEDS_PART_A, CHSV(0,255,maxBrtBrake));
        if(showTailBrakeSmalStrip == 1){
          fill_solid( ledsB[0], NUM_LEDS_PART_B, CHSV(0,255,maxBrtBrake));
          fill_solid( ledsB[1], NUM_LEDS_PART_B, CHSV(0,255,maxBrtBrake));   
        }
      
      }else if(stateLT != 0 && stateRT != 0){

        if(showSignalBigStrip == 1){
          for(int i =  NUM_LEDS_PART_B; i < halfOfLeds; i++){
            ledsA[i] = CHSV(0,255,maxBrtBrake);
          }      
          for(int i = halfOfLeds; i < NUM_LEDS_PART_A-NUM_LEDS_PART_B; i++){
            ledsA[i] = CHSV(0,255,maxBrtBrake);
          }          
        }else{
          for(int i = 0; i < halfOfLeds; i++){
            ledsA[i] = CHSV(0,255,maxBrtBrake);
          }      
          for(int i = halfOfLeds; i < NUM_LEDS_PART_A; i++){
            ledsA[i] = CHSV(0,255,maxBrtBrake);
          }
        }
            
      }else{
        
        if(stateLT != 0){
          
            if(showSignalBigStrip == 1){
              for(int i =  NUM_LEDS_PART_B; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }              
            }else{
              for(int i =  0; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
            }

            if(showTailBrakeSmalStrip == 1){
              fill_solid( ledsB[1], NUM_LEDS_PART_B, CHSV(0,255,maxBrtBrake));   
            }
        }
        if(stateRT != 0){
          
            if(showSignalBigStrip == 1){
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A-NUM_LEDS_PART_B; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
              for(int i = 0; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
            }else{
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }
              for(int i = 0; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,255,maxBrtBrake);
              }

            }
            if(showTailBrakeSmalStrip == 1){
              fill_solid( ledsB[0], NUM_LEDS_PART_B, CHSV(0,255,maxBrtBrake));
            }
        }
        
      }
      
      tailLeft = halfOfLeds-1;
      tailRight = halfOfLeds;
      brakeToOffL = 1;
      brakeToOffR = 1;
      
    }else{
      
      if(tailLeft < halfOfLeds-1){
        EVERY_N_MILLISECONDS( tailDelay ) {
          tailLeft++;
          ledsA[tailLeft]=CHSV(0,0,0);
          if(showTailBrakeSmalStrip == 1 && tailLeft < NUM_LEDS_PART_B){
            tailLeftSmall = map(tailLeft,NUM_LEDS_PART_B-1,0,0,NUM_LEDS_PART_B-1);
            ledsB[0][tailLeftSmall]=CHSV(0,0,0);
          }
        }
      }else{
        if(stateLT != 0){
          if(brakeToOffL == 1){
            if(showSignalBigStrip == 1){
              for(int i = NUM_LEDS_PART_B; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,0,0);
              }              
            }else{
              for(int i = 0; i < halfOfLeds; i++){
                ledsA[i] = CHSV(0,0,0);
              }
            }
            brakeToOffL = 0;
          }
        }else{
          for(int i = 0; i < halfOfLeds; i++){
              ledsA[i] = CHSV(0,0,0);
          }
          if(KnightRiderState == LOW){
            fill_solid( ledsB[0], NUM_LEDS_PART_B, CHSV(0,0,0));
          }
        }
      }

      if(tailRight > halfOfLeds){
        EVERY_N_MILLISECONDS( tailDelay ) {
          tailRight--;
          ledsA[tailRight]=CHSV(0,0,0);
          if(showTailBrakeSmalStrip == 1 && tailRight > NUM_LEDS_PART_A-NUM_LEDS_PART_B-1){
            tailRightSmall = map(tailRight,NUM_LEDS_PART_A-NUM_LEDS_PART_B-1,NUM_LEDS_PART_A-1,0,NUM_LEDS_PART_B-1);
            ledsB[1][tailRightSmall]=CHSV(0,0,0);
          }
        }

      }else{
        if(stateRT != 0){
          if(brakeToOffR == 1){
            if(showSignalBigStrip == 1){
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A-NUM_LEDS_PART_B; i++){
                ledsA[i] = CHSV(0,0,0);
              }              
            }else{
              for(int i = halfOfLeds; i < NUM_LEDS_PART_A; i++){
                ledsA[i] = CHSV(0,0,0);
              }
            }
            brakeToOffR = 0;
          }
        }else{
            for(int i = halfOfLeds; i < halfOfLeds*2; i++){
              ledsA[i] = CHSV(0,0,0);
            }
          if(KnightRiderState == LOW){
            fill_solid( ledsB[1], NUM_LEDS_PART_B, CHSV(0,0,0));            
          }
        }
      }
    } 
    if(engineOn == HIGH && TailLights == LOW){
      ledsC[1] = dashDayColor;      
    }else if(KnightRiderState == HIGH){
      ledsC[1] = dashKnightRiderColor;
    }else if(TailLights == HIGH){
      ledsC[1] = dashTailColor;
    }else{
      ledsC[1] = 0x000000;
    }
}

//function for left signal interrupt
void btnPressL(){
  stateLT = 1;
  for(int nr = 0; nr < 1500; nr++) { 
    buttonStateR = digitalRead(buttonPinR);
    if(buttonStateR == 1){
      stateRT = 1;     
    }
  }  
}

//function for right signal interrupt
void btnPressR(){
  stateRT = 1;
  for(int nr = 0; nr < 1500; nr++) { 
   buttonStateL = digitalRead(buttonPinL); 
   if(buttonStateL == 1){
      stateLT = 1;
    }
  }
}
