#include <TFT_eSPI.h>
#include <Preferences.h>
#include <esp_adc_cal.h>
#include "back.h"
#include "gameOver.h"
#include "newGame.h"


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Create preferences object
Preferences preferences;

#define left 0
#define right 14
#define PIN_POWER_ON 15
#define PIN_BAT_VOLT 4

// define the time period in which the device should be put to deep sleep
#define SLEEP_PERIOD 60000 // 60 seconds

bool button_pressed = false; // flag to keep track of whether a button was pressed or not

int size=1;
int y[120]={0};
int x[120]={0};
int highscore=0;
unsigned long currentTime=0;
unsigned long readTime=0;
int period=200;
int deb,deb2=0;
int dirX=1;
int dirY=0;
bool taken=0;
unsigned short colors[2]={0x48ED,0x590F}; //terain colors
unsigned short snakeColor[2]={0x9FD3,0x38C9};
unsigned short textbg=0x7DFD;
unsigned short background=0x02D3;
bool chosen=0;
bool gOver=0;
int moves=0;
int playTime=0;
int foodX=0;
int foodY=0;
int howHard=0;
String diff[3]={"EASY","NORMAL","HARD"};
bool ready=1;
long readyTime=0;

int change=0;

void showHighscore() //Show the highscore
{
  // Blank the text area
  tft.fillRoundRect(80,6,65,21,1,textbg);
  tft.setTextDatum(0);
  tft.setTextSize(1);
  tft.drawString("Highscore:",83,8);
  if (size==highscore) {
    tft.setTextColor(TFT_RED,textbg);
  } else {
    tft.setTextColor(TFT_PURPLE,textbg);
  }
  tft.drawString((String)highscore,103,18);
}

void showBat() //Show the current battery %
{
  esp_adc_cal_characteristics_t adc_chars;

  // Get the internal calibration value of the chip
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half

  // Map millivolts to percentage
  uint32_t batPercent = map(v1, 3950, 2900, 100, 0);

  // Blank the text area
  tft.fillRoundRect(55,290,65,25,1,textbg);

  tft.setTextDatum(0);
  tft.setTextSize(1);
  tft.drawString("Battery:",65,295);
  if (batPercent<=10) {
    tft.setTextColor(TFT_RED,textbg);
  } else {
    tft.setTextColor(TFT_PURPLE,textbg);
  }
  if (batPercent>100){
    tft.drawString("Charging",65,305);
  } else {
    tft.drawString((String)batPercent+"%",75,305);
  }

  //tft.drawString((String)v1,75,305);

}

void checkGameOver()//..,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,check game over
{
if(x[0]<0 || x[0]>=17 || y[0]<0 || y[0]>=17 )
gOver=true;
for(int i=1;i<size;i++)
if(x[i]==x[0] && y[i]==y[0])
gOver=true;
}

void getFood()//.....................getFood -get new position of food
{
    foodX=random(0,17);
    foodY=random(0,17);
    taken=0;
    for(int i=0;i<size;i++)
    if(foodX==x[i] && foodY==y[i])
    taken=1;
    if(taken==1)
    getFood();
}

void run()//...............................run function
{

  for(int i=size;i>0;i--)
  {
    x[i]=x[i-1];    
    y[i]=y[i-1];     
  }    

  x[0]=x[0]+dirX;
  y[0]=y[0]+dirY;

  if(x[0]==foodX && y[0]==foodY)
  {
    size++;
    getFood();
    tft.setTextSize(3);
    tft.setTextDatum(4);
    tft.drawString(String(size),44,250);
    period=period-1;
    tft.drawString(String(500-period),124,250);
  }
    
  sprite.fillSprite(TFT_BLACK);
     /*
    for(int i=0;i<17;i++)
      for(int j=0;j<17;j++)
        {
        sprite.fillRect(j*10,i*10,10,10,colors[chosen]);
        chosen=!chosen;
        }
     chosen=0;*/
  checkGameOver();
  if(gOver==0)
  {
    sprite.drawRect(0,0,170,170,0x02F3);     
    for(int i=0;i<size;i++){
      sprite.fillRoundRect(x[i]*10,y[i]*10,10,10,2,snakeColor[0]); 
      sprite.fillRoundRect(2+x[i]*10,2+y[i]*10,6,6,2,snakeColor[1]); 
    }
    sprite.fillRoundRect(foodX*10+1,foodY*10+1,8,8,1,TFT_RED); 
    sprite.fillRoundRect(foodX*10+3,foodY*10+3,4,4,1,0xFE18); 
  }
  else    
  {
    sprite.pushImage(0,0,170,170,gameOver);
    sprite.pushSprite(0,30);
    if(size > highscore)
    {
      //Save the new highscore to flash
      highscore = size;
      preferences.putInt("highscore", highscore);
      //Show the highscore
      showHighscore();
    }
    delay(30000); //Show the score for 30 sec
    //Go into deep sleep        //Call new game instead?
    esp_sleep_enable_ext0_wakeup((GPIO_NUM_14), LOW);
    esp_deep_sleep_start();
  }    
  sprite.pushSprite(0,30);
}     

void setup() {  //.......................setup
  pinMode(left,INPUT_PULLUP);
  pinMode(right,INPUT_PULLUP);
  pinMode(PIN_POWER_ON, OUTPUT);

  digitalWrite(PIN_POWER_ON, HIGH);  	//triggers the LCD backlight, and enables battery power

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0,0,170,320,back);
  tft.pushImage(0,30,170,170,newGame);

  // Create rectangle for highscore
  tft.fillRect(80,6,81,23,background);

  // Create rectangle for battery state
  tft.fillRect(45,284,75,35,background);

  tft.setTextColor(TFT_PURPLE,textbg);
  tft.fillSmoothCircle(28,102+(howHard*24),5,TFT_RED,TFT_BLACK); 
  tft.drawString("DIFFICULTY:  "+ diff[howHard]+"   ",26,267); 

  sprite.createSprite(170,170);
  sprite.setSwapBytes(true);

  //Use preferences to save config
  preferences.begin("snake", false); 

  //Retreive highscore
  highscore = preferences.getInt("highscore", 0); //if the key does not exist, return a default value of 0
  //highscore = 2; //testing
  //Show the highscore
  showHighscore();
  
  //Wait for right button to start game
  while(digitalRead(right)==1)
  {
    unsigned long time_now = millis(); // current time          
    if(digitalRead(left)==0)
    {
      if(deb2==0)
        {
        button_pressed = true; // set the flag to true if a button is pressed
        deb2=1; 
        tft.fillCircle(28,102+(howHard*24),6,TFT_BLACK);   
        howHard++;   if(howHard==3) howHard=0;  
        tft.fillSmoothCircle(28,102+(howHard*24),5,TFT_RED,TFT_BLACK);
        tft.drawString("DIFFICULTY:  "+ diff[howHard]+"   ",26,267);  
        period=200-howHard*20;  
        delay(200); 
        }             

    } else deb2=0;

    // check if it's time to put the device to sleep
    if (time_now - currentTime >= SLEEP_PERIOD && !button_pressed) {
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW);
      esp_deep_sleep_start();
    }
    // if a button is pressed, reset the flag and update the current time
    if (button_pressed) {
      button_pressed = false;
      currentTime = time_now;
    }

    // Check if it's time to read battery voltage
    if (time_now - readTime >= 1000) {
      //Show the battery voltage
      showBat();
      readTime = time_now;
    }
  }

  y[0]=random(5,15);
  getFood();
  tft.setTextSize(3);
  tft.setTextDatum(4);
  tft.drawString(String(size),44,250); 
  tft.drawString(String(500-period),124,250);
  delay(400);
  dirX=1;
  dirY=0;
}



void loop() { //...............................................................loop
  
  if(millis()>currentTime+period) 
  {run(); currentTime=millis();}

  if(millis()>readTime+1000) 
  {showBat(); readTime=millis();}

  if(millis()>readyTime+100 && ready==0) 
  {ready=1;} 

  if(ready==1){
  if(digitalRead(left)==0){

    if(deb==0)
    {deb=1;
      if(dirX==1 && change==0) {dirY=dirX*-1; dirX=0; change=1;}
      if(dirX==-1 && change==0) {dirY=dirX*-1; dirX=0;change=1; }
      if(dirY==1 && change==0) {dirX=dirY*1; dirY=0; change=1;}
      if(dirY==-1 && change==0) {dirX=dirY*1; dirY=0; change=1;}
      change=0;
      ready=0;
      readyTime=millis();
    }
  }else{ deb=0;}}

  if(ready==1){
  if(digitalRead(right)==0)
  {
    
    if(deb2==0)
    {
      deb2=1;
      if(dirX==1 && change==0) {dirY=dirX*1; dirX=0; change=1;}
      if(dirX==-1 && change==0) {dirY=dirX*1; dirX=0;change=1; }
      if(dirY==1 && change==0) {dirX=dirY*-1; dirY=0; change=1;}
      if(dirY==-1 && change==0) {dirX=dirY*-1; dirY=0; change=1;}
      change=0;
      ready=0;
      readyTime=millis();
    }
  }else {deb2=0;}}

}
