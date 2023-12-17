
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "LedControl.h"
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
byte isTextScrolling = false;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrix[matrixSize][matrixSize] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};
byte isGamePlaying = false;
const int rs = 9, en = 8, d4 = 7, d5 = 3, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
char* menuOptions[] = { "Start Game", "Settings", "HowToPlay" };
char* howToPlay = "Crossy road is a game in which you have to cross a dangerous road by dodging cars and crossing rivers. Use the joystick to controll the blincking dot and your tenacity to get it safely acrros the road. Good luck!";
char * settingsMenuOptions[] = { "LCD Brightness", "Mtrx Brightness", "About", "Difficulty", "Show HighScores", "Reset Highscore", "Sounds", "Back" };
char* aboutOptions[] = { "Andrei Alexandru", "Crossy Roads", "https://github.com/andreialexandru02/IntroductionToRobotics " };
char* soundOptions[] = { "ON", "OFF" };
//char letters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
byte isInSettings = false;
byte isInLCDBrightness = false;
byte isInMatrixBrightness = false;
byte isGameEnded = false;
byte isInAboutSection = false;
byte isInDifficulty = false;
byte isGameWon = false;
byte isInHighScoreDisplay = false;
byte isInSoundMode = false;
byte isInHowToPlay = false;
char* setNameText = "Set Name (Max 6 letters)  ";
int numMenuOptions = 3;
int selectedMenuOption = 0;

//JoyStick
int xValueJoy = 0;
int yValueJoy = 0;
bool joyMoved = false;
int minThresholdJoy = 400;
int maxThresholdJoy = 600;
const int pinXJoy = A0;
const int pinYJoy = A1;
//Button
const int buttonPin = 2;
byte buttonState = LOW;
byte reading = HIGH;
byte lastReading = HIGH;
unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;
int LCD_Backlight = 6;

//EEPROM
int LCDBrightnessMermorySlot = 0;
int MatrixBrightnessMermorySlot = 2;
int difficultyMemorySlot = 4;
int highScoreMemorySlots[] = { 6, 8, 10 };
int highScoreNamesMemorySlots[] = { 12, 18, 24 };
int soundMemorySlot = 30;
unsigned long timerStartTime = 0;
unsigned long startingScollingTime = 0;
int scrollingDelay = 200;
int timer;
int cursor;
byte fullBlock[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
byte heart[8] = {
  0b00000,
  0b00000,
  0b11011,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};
int car1, car2, bridge1, bridge2, car1Position, car2Position, bridge1Position, bridge2Position;
unsigned long carMovingTime;
unsigned long bridgeMovingTime;
int bridgeMovingDelay;
int carMovingDelay;
int playerX, playerY;
byte playerLedState = false;
unsigned long playerBlinkTime;
const int playerBlinkDelay = 200;
byte followBridge = false;
int lives = 3;
unsigned long loseLifeTime = 0;
int loseLifeDelay = 1000;

const int buzzerPin = 13;
int buzzerToneSwitch = 500;
int buzzerToneButton = 1000;
int buzzerToneLoseLife = 300;
int buzzerToneGameOver = 100;
byte sound = false;

void setup() {
  sound = EEPROM.get(soundMemorySlot, sound);
  lc.shutdown(0, false);
  int matrixBrightness = EEPROM.get(MatrixBrightnessMermorySlot, matrixBrightness);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
  pinMode(buttonPin, INPUT_PULLUP);
  int difficutlyLvl = EEPROM.get(difficultyMemorySlot, difficutlyLvl);
  setDifficultyLvl(difficutlyLvl);
  // pinMode(LCD_Backlight, OUTPUT);
  int brightnessLvl = EEPROM.get(LCDBrightnessMermorySlot, brightnessLvl);
  analogWrite(LCD_Backlight, brightnessLvl);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO");
  lcd.setCursor(0, 1);
  lcd.print("CROSSY ROADS");
  delay(1000);
  lcd.clear();
  lcd.print(menuOptions[selectedMenuOption]);
  lcd.createChar(0, fullBlock);
  lcd.createChar(1, heart);
}
void loop() {


  if (!isGamePlaying) {
    moveJoyStick();
    if (isTextScrolling) {
      scrollText(aboutOptions[selectedMenuOption], 1);
    }
    if(isInHowToPlay){
      scrollText(howToPlay, 1);
    }
  } else {
    moveJoyStick();
    displayGameScreen();
    playGame();
  }
  buttonDebounce();
}
void setDifficultyLvl(int difficutlyLvl) {
  if (difficutlyLvl == 1) {
    bridgeMovingDelay = 1000;
    carMovingDelay = 600;
  } else if (difficutlyLvl == 2) {
    bridgeMovingDelay = 700;
    carMovingDelay = 500;
  } else {
    bridgeMovingDelay = 600;
    carMovingDelay = 400;
  }
}
void moveJoyStick() {
  xValueJoy = analogRead(pinXJoy);
  yValueJoy = analogRead(pinYJoy);
  if (yValueJoy > maxThresholdJoy && joyMoved == false) {  //JOS
    Serial.println("JOS");
    if (sound) {
      tone(buzzerPin, buzzerToneSwitch, 100);
    }
    if (isGamePlaying) {
      lc.setLed(0, playerX, playerY, false);
      if (playerX != 7) {
        playerX++;
      }
    } else {
      Serial.println(numMenuOptions);
      Serial.println(selectedMenuOption);
      selectedMenuOption = (selectedMenuOption + 1) % numMenuOptions;
      displayMenu();
    }
    joyMoved = true;
  }
  if (yValueJoy < minThresholdJoy && joyMoved == false) {  //SUS
    Serial.println("SUS");
    if (sound) {
      tone(buzzerPin, buzzerToneSwitch, 100);
    }
    if (isGamePlaying) {
      Serial.println("asdad");
      lc.setLed(0, playerX, playerY, false);
      if (playerX != 0) {
        // winGame();
        playerX--;
      } else {
        winGame();
      }
    } else {
      Serial.println(numMenuOptions);
      Serial.println(selectedMenuOption);
      selectedMenuOption = (selectedMenuOption - 1 + numMenuOptions) % numMenuOptions;
      displayMenu();
    }
    joyMoved = true;
  }
  if (xValueJoy > maxThresholdJoy && joyMoved == false) {  //DREAPTA
    Serial.println("DREAPTA");
    if (sound) {
      tone(buzzerPin, buzzerToneSwitch, 100);
    }
    if (isGamePlaying) {
      lc.setLed(0, playerX, playerY, false);
      playerY++;
      playerY %= matrixSize;
    } else {
      if (isInLCDBrightness && selectedMenuOption < 4) {
        selectedMenuOption = selectedMenuOption + 1;
        analogWrite(LCD_Backlight, 20 + selectedMenuOption * 20);
        Serial.println(20 + selectedMenuOption * 20);
      } else if (isInMatrixBrightness && selectedMenuOption < 4) {
        selectedMenuOption = selectedMenuOption + 1;
        lc.setIntensity(0, (selectedMenuOption + 1) * 3);
      } else if (isInDifficulty && selectedMenuOption < 2) {
        selectedMenuOption = selectedMenuOption + 1;
      }
      displayMenu();
    }
    joyMoved = true;
  }
  if (xValueJoy < minThresholdJoy && joyMoved == false) {  //STANGA
    Serial.println("STANGA");
    if (sound) {
      tone(buzzerPin, buzzerToneSwitch, 100);
    }
    if (isGamePlaying) {
      lc.setLed(0, playerX, playerY, false);
      if (playerY == 0) {
        playerY = 7;
      } else {
        playerY--;
      }
    } else {
      if (isInLCDBrightness && selectedMenuOption > 0) {
        selectedMenuOption = selectedMenuOption - 1;
        analogWrite(LCD_Backlight, 20 + selectedMenuOption * 20);
        Serial.println(20 + selectedMenuOption * 20);
      } else if (isInMatrixBrightness && selectedMenuOption > 0) {
        selectedMenuOption = selectedMenuOption - 1;
        lc.setIntensity(0, (selectedMenuOption + 1) * 3);
      } else if (isInDifficulty && selectedMenuOption > 0) {
        selectedMenuOption = selectedMenuOption - 1;
      }
      displayMenu();
    }
    joyMoved = true;
  }

  if (xValueJoy >= minThresholdJoy && xValueJoy <= maxThresholdJoy && yValueJoy >= minThresholdJoy && yValueJoy <= maxThresholdJoy) {
    joyMoved = false;
  }
}
void buttonDebounce() {
  reading = digitalRead(buttonPin);
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        if (sound) {
          tone(buzzerPin, buzzerToneButton, 100);
        }
        if (!isInSoundMode && !isInHighScoreDisplay && !isGameWon && !isInDifficulty && !isInAboutSection && !isGameEnded && !isGamePlaying && !isInSettings && !isInLCDBrightness && !isInMatrixBrightness && selectedMenuOption == 0) {
          isGamePlaying = true;
          timerStartTime = millis();
          timer = 0;
          gameSetup();
        } else if (isGameEnded) {
          isGameEnded = false;
          displayMenu();
        } else if (isGameWon) {
          isGameWon = false;
          displayMenu();
        } else if (!isInSoundMode && !isInHighScoreDisplay && !isInAboutSection && !isInDifficulty && !isInLCDBrightness && !isInMatrixBrightness && !isInSettings && selectedMenuOption == 1) {
          isInSettings = true;
          selectedMenuOption = 0;
          numMenuOptions = 8;
          displayMenu();
        } else if (!isInHowToPlay && !isInSoundMode && !isInHighScoreDisplay && !isInAboutSection && !isInDifficulty && !isInLCDBrightness && !isInMatrixBrightness && !isInSettings && selectedMenuOption == 2) {
          isInHowToPlay = true;
          displayMenu();
        } else if(isInHowToPlay) {
          selectedMenuOption = 0;
          isInHowToPlay = false;
          displayMenu();
        } 
        else if (isInSettings && selectedMenuOption == 5) {  //resetHighScore
          resetHighScores();
        } else if (isInSettings && selectedMenuOption == 7) {  //Back
          isInSettings = false;
          selectedMenuOption = 0;
          numMenuOptions = 3;
          displayMenu();
        } else if (isInSettings && !isInLCDBrightness && selectedMenuOption == 0) {
          isInLCDBrightness = true;
          isInSettings = false;
          int brightnessLvl = EEPROM.get(LCDBrightnessMermorySlot, brightnessLvl);
          selectedMenuOption = (brightnessLvl - 20) / 20;
          displayMenu();
        } else if (isInSettings && !isInAboutSection && selectedMenuOption == 2) {
          isInAboutSection = true;
          isInSettings = false;
          numMenuOptions = 3;
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInAboutSection) {
          isInAboutSection = false;
          isTextScrolling = false;
          isInSettings = true;
          selectedMenuOption = 0;
          numMenuOptions = 8;
          displayMenu();
        } else if (isInLCDBrightness) {
          isInLCDBrightness = false;
          isInSettings = true;
          EEPROM.put(LCDBrightnessMermorySlot, 20 + selectedMenuOption * 20);
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInSettings && !isInMatrixBrightness && selectedMenuOption == 1) {
          isInMatrixBrightness = true;
          isInSettings = false;
          selectedMenuOption = EEPROM.get(MatrixBrightnessMermorySlot, selectedMenuOption) / 3 - 1;
          turnOnMatrix();
          displayMenu();
        } else if (isInMatrixBrightness) {
          isInMatrixBrightness = false;
          isInSettings = true;
          EEPROM.put(MatrixBrightnessMermorySlot, (selectedMenuOption + 1) * 3);
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInSettings && !isInDifficulty && selectedMenuOption == 3) {
          isInDifficulty = true;
          isInSettings = false;
          selectedMenuOption = EEPROM.get(difficultyMemorySlot, selectedMenuOption) - 1;
          displayMenu();
        } else if (isInDifficulty) {
          isInDifficulty = false;
          isInSettings = true;
          Serial.println(selectedMenuOption);
          EEPROM.put(difficultyMemorySlot, selectedMenuOption + 1);
          setDifficultyLvl(selectedMenuOption + 1);
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInSettings && !isInHighScoreDisplay && selectedMenuOption == 4) {
          isInHighScoreDisplay = true;
          isInSettings = false;
          displayMenu();
        } else if (isInHighScoreDisplay) {
          isInHighScoreDisplay = false;
          isInSettings = true;
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInSettings && !isInSoundMode && selectedMenuOption == 6) {  //Sounds
          isInSoundMode = true;
          isInSettings = false;
          numMenuOptions = 2;
          selectedMenuOption = 0;
          displayMenu();
        } else if (isInSoundMode) {
          if (selectedMenuOption == 0) {
            EEPROM.put(soundMemorySlot, true);
            sound = true;
          } else {
            EEPROM.put(soundMemorySlot, false);
            sound = false;
          }
          isInSoundMode = false;
          isInSettings = true;
          selectedMenuOption = 0;
          numMenuOptions = 8;
          displayMenu();
        }
      }
    }
  }
  lastReading = reading;
}
void displayMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isInSettings) {
    lcd.print(settingsMenuOptions[selectedMenuOption]);
  } else if (isInLCDBrightness || isInMatrixBrightness) {
    lcd.clear();
    showBrightnessLvl(selectedMenuOption + 1);
  } else if (isGameEnded) {
    if (sound) {
      tone(buzzerPin, buzzerToneGameOver, 500);
    }
    lcd.print("GameOver!");
  } else if (isInSoundMode) {
    lcd.clear();
    lcd.print(soundOptions[selectedMenuOption]);
  } else if (isGameWon) {

    int highScore1 = EEPROM.get(highScoreMemorySlots[0], highScore1);
    int highScore2 = EEPROM.get(highScoreMemorySlots[1], highScore2);
    int highScore3 = EEPROM.get(highScoreMemorySlots[2], highScore3);

    Serial.println(highScore1);
    Serial.println(highScore2);
    Serial.println(highScore3);

    if (timer < highScore3) {
      lcd.clear();
      lcd.print("New HighScore");
      lcd.setCursor(0, 1);
      lcd.print(timer);
      // lcd.print("SetName");
      // // lcd.setCursor(6,1);
      // // isInSetName = true;
      // // setName();
      if (timer < highScore1) {
        EEPROM.put(highScoreMemorySlots[0], timer);
      } else if (timer < highScore2) {
        EEPROM.put(highScoreMemorySlots[1], timer);
      } else {
        EEPROM.put(highScoreMemorySlots[2], timer);
      }
    } else {
      lcd.clear();
      lcd.print("Winner!");
      lcd.setCursor(0, 1);
      int text = "Score:";
      lcd.print("Score:");
      lcd.setCursor(strlen(text), 1);
      lcd.print(timer);
    }
  } else if (isInHighScoreDisplay) {
    int highScore1 = EEPROM.get(highScoreMemorySlots[0], highScore1);
    int highScore2 = EEPROM.get(highScoreMemorySlots[1], highScore2);
    int highScore3 = EEPROM.get(highScoreMemorySlots[2], highScore3);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("#1:");
    lcd.setCursor(3, 0);
    if (highScore1 != 10000) {
      lcd.print(highScore1);
    } else {
      lcd.print("NaN");
    }
    lcd.setCursor(8, 0);
    lcd.print("#2:");
    lcd.setCursor(11, 0);
    if (highScore2 != 10000) {
      lcd.print(highScore2);
    } else {
      lcd.print("NaN");
    }
    lcd.setCursor(0, 1);
    lcd.print("#3:");
    lcd.setCursor(3, 1);
    if (highScore3 != 10000) {
      lcd.print(highScore3);
    } else {
      lcd.print("NaN");
    }
  } else if (isInAboutSection) {
    if (strlen(aboutOptions[selectedMenuOption]) > 16) {
      cursor = 0;
      startingScollingTime = millis();
      isTextScrolling = true;
    } else {
      isTextScrolling = false;
      lcd.clear();
      lcd.print(aboutOptions[selectedMenuOption]);
    }
  } else if (isInDifficulty) {
    lcd.clear();
    ShowDifficultyLvl(selectedMenuOption + 1);
  } else {
    lcd.print(menuOptions[selectedMenuOption]);
  }
}
void showBrightnessLvl(int level) {
  lcd.clear();
  for (int i = 0; i < level; i++) {
    lcd.setCursor(i, 0);
    lcd.write(byte(0));
  }
}
void ShowDifficultyLvl(int level) {
  lcd.clear();
  for (int i = 0; i < level; i++) {
    lcd.setCursor(i, 0);
    lcd.write(byte(0));
  }
}
void turnOnMatrix() {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      lc.setLed(0, i, j, true);
}
void turnOffMatrix() {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      lc.setLed(0, i, j, false);
}
void displayGameScreen() {
  if (!isGameWon) {
    if (millis() - timerStartTime > 1000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      timer++;
      lcd.print(timer);
      timerStartTime = millis();
    }
    for (int i = 0; i < lives; i++) {
      lcd.setCursor(i, 1);
      lcd.write(byte(1));
    }
  }
}
void endGame() {

  isGamePlaying = false;
  isGameEnded = true;
  turnOffMatrix();
  displayMenu();
}
void winGame() {

  isGamePlaying = false;
  isGameWon = true;
  turnOffMatrix();
  displayMenu();
}
void scrollText(char* text, int line) {
  int textLength = strlen(text);

  if (millis() - startingScollingTime > scrollingDelay) {


    if (cursor == (textLength - 1)) {
      cursor = 0;
    }
    lcd.setCursor(0, line - 1);
    if (cursor < textLength - 16) {
      for (int i = cursor; i < cursor + 16; i++) {
        lcd.print(text[i]);
      }
    } else {
      for (int i = cursor; i < (textLength); i++) {
        lcd.print(text[i]);
      }
      for (int i = 0; i <= 16 - (textLength - cursor); i++) {
        lcd.print(text[i]);
      }
    }
    cursor++;
    startingScollingTime = millis();
  }
}
void playGame() {
  blinkPlayer();
  // Serial.println(bridgePosition);
  turnOffMatrix();
  for (int i = 0; i < matrixSize; i++) {
    if (i == car1Position) {
      lc.setLed(0, car1, i, true);
    }
    if (i == car2Position) {
      lc.setLed(0, car2, i, true);
    }
    if (i != bridge1Position) {
      lc.setLed(0, bridge1, i, true);
    }
    if (i != bridge2Position) {
      lc.setLed(0, bridge2, i, true);
    }
  }
  if (millis() - bridgeMovingTime > bridgeMovingDelay) {
    bridge1Position++;
    bridge1Position %= matrixSize;
    bridge2Position++;
    bridge2Position %= matrixSize;
    bridgeMovingTime = millis();
  }
  if (millis() - carMovingTime > carMovingDelay) {
    car1Position++;
    car1Position %= matrixSize;
    car2Position++;
    car2Position %= matrixSize;
    carMovingTime = millis();
  }
  if ((playerX == car1 && playerY == car1Position) || (playerX == car2 && playerY == car2Position) || (playerX == bridge1 && playerY != bridge1Position) || (playerX == bridge2 && playerY != bridge2Position)) {
    if (millis() - loseLifeTime >= loseLifeDelay) {
      if (sound) {
        tone(buzzerPin, buzzerToneLoseLife, 100);
      }
      lives--;
      playerX = 7;
      loseLifeTime = millis();
    }
  }
  if (lives == 0) {
    endGame();
  }
}
void blinkPlayer() {
  lc.setLed(0, playerX, playerY, playerLedState);
  if (millis() - playerBlinkTime > playerBlinkDelay) {
    playerLedState = !playerLedState;
    playerBlinkTime = millis();
  }
}
void gameSetup() {
  lives = 3;
  turnOffMatrix();
  randomSeed(analogRead(0));
  car1 = 0;
  car2 = 0;
  bridge1 = 0;
  bridge2 = 0;
  while (car1 == 0 || car1 == 7) {
    car1 = random(matrixSize);
  }
  while (car2 == 0 || car2 == 7 || car1 == car2) {
    car2 = random(matrixSize);
  }
  while (bridge1 == 0 || bridge1 == 7 || bridge1 == car1 || bridge1 == car2) {
    bridge1 = random(matrixSize);
  }
  while (bridge2 == 0 || bridge2 == 7 || bridge2 == car1 || bridge2 == car2 || abs(bridge2 - bridge1) <= 1) {
    bridge2 = random(matrixSize);
  }
  randomSeed(analogRead(0));
  car1Position = random(matrixSize);
  car2Position = random(matrixSize);
  bridge1Position = random(matrixSize);
  bridge2Position = random(matrixSize);
  playerX = 7;
  playerY = random(matrixSize);
  carMovingTime = millis();
  bridgeMovingTime = millis();
  playerBlinkTime = millis();
}
void resetHighScores() {
  EEPROM.put(highScoreMemorySlots[0], 10000);
  EEPROM.put(highScoreMemorySlots[1], 10000);
  EEPROM.put(highScoreMemorySlots[2], 10000);
}
