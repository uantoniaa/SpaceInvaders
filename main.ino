#include "LedControl.h"  // Include the library for controlling LED matrices.
#include <LiquidCrystal.h>  // Include the library for LCD display control.
#include <EEPROM.h>  // Include the library for reading and writing to the Arduino's EEPROM.

// Information about the game
const char gameName[] = "Space Invaders";  // Name of the game.
const char authorName[] = "Ungureanu Antonia";  // Author's name.
const char githubLink[] = "github.com/uantoniaa/SpaceInvaders";  // GitHub link for the project.
const int settingsSize = 8;  // Number of settings available in the game.

// Pin definitions
const int buzzerPin = 13;  // Pin number connected to the buzzer.
const int dinPin = 12;  // 'Data In' pin for the LED matrix.
const int clockPin = 11;  // Clock pin for the LED matrix.
const int loadPin = 10;  // Load pin for the LED matrix.
int lastcurrentCursorMenuPos = 0;  // To keep track of the last cursor position in the menu.
const int playerBlinkInterval = 500;  // Time interval for player blink in milliseconds.
const int bombBlinkInterval = 100;  // Time interval for bomb blink in milliseconds.

struct cursorPosition {
  int lcdRow, lcdCol;  // Row and column for LCD cursor position.
  char* Name;  // Name of the cursor position (like menu items).
};

const int RS = 9;  // 'Register Select' pin for the LCD.
const int enable = 8;  // 'Enable' pin for the LCD.
const int d4 = 7, d5 = 6, d6 = 5, d7 = 4;  // Data pins for the LCD.
bool buttonPressed();  // Function declaration for checking button press.

const int xPin = A0;  // Analog pin for X-axis of joystick.
const int yPin = A1;  // Analog pin for Y-axis of joystick.
const int swPin = 2;  // Digital pin for joystick switch.

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // Initialize LED matrix control.
LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);  // Initialize LCD display.

const int ARROW_WIDTH = 3;  // Width of the arrow character.

const int menuPositionsSize = 4;  // Number of positions in the menu.
cursorPosition menuCursorPos[menuPositionsSize];  // Array to store menu cursor positions.
cursorPosition settingsCursorPos[settingsSize];  // Array to store settings cursor positions.
int currentCursorMenuPos;  // To track the current cursor position in the menu.

void handleGameOver();  // Function declaration for handling the game over scenario.

long long lastScroll = 0;  // Variable to store the last time the screen was scrolled.
int scrollInterval = 500;  // Time interval for scrolling text on the LCD.

int currentSettingsPos = 0, lastSettingsPos = 0;  // Variables to track the current and last positions in settings.
char currentUser[5] = "AAA\0";  // Default user name, with space for 3 characters and a null terminator.
int contrastValue = 3, brightnessValue = 3, currentLevel = 1, matrixBrightnessValue = 1;  // Default settings values.
int userCol = 6, userRow = 0;  // Variables for tracking the user's cursor position in settings.
const int contrastValues[] = {50, 75, 100, 120, 135, 150};  // Possible values for LCD contrast.
const int brightnessValues[] = {0, 50, 100, 150, 200, 250};  // Possible values for LCD brightness.

// Game menu variables
const char startGameMessage1[] = "Press to";  // Message part 1 displayed at game start.
const char startGameMessage2[] = "start level ";  // Message part 2 displayed at game start.
bool gameStarted = false;  // Flag to check if the game has started.
bool gameEnded = false;  // Flag to check if the game has ended.
const int maximumLevel = 5;  // Maximum level in the game.
int gameScore = 0;  // Current game score.
int startScore = 0;  // Score at the start of the game.
int totalScore = 0;  // Total score accumulated.
long long int lastLedBlink = 0;  // Time of the last LED blink.
const int blinkLedInterval = 200;  // Interval for LED blinking.

// Highscore menu variables
int highScore[3] = {0, 0, 0};  // Array to store top three high scores.
char highScoreNames[3][4] = {"UNK", "UNK", "UNK"};  // Array to store names of top scorers.
int currentScorePos = 0;  // Current position in the high score list.
int lastScorePos = 0;  // Last position in the high score list.
const int scoresSize = 4;  // Size of the scores array.
cursorPosition scoreCursorPos[scoresSize];  // Array to store score cursor positions.
bool newHighScore = false;  // Flag to check if there's a new high score.
bool scoreUpdated = false;  // Flag to check if the score was updated.

// Joystick control variables
bool switchState = LOW;  // Current state of the joystick switch.
bool lastState = HIGH;  // Last state of the joystick switch.
unsigned long lastEnemySpawnTime = 0;  // Last time an enemy was spawned.
const long enemySpawnInterval = 2500;  // Interval for spawning enemies.

const int minThreshold = 200;  // Minimum threshold for joystick movement.
const int maxThreshold = 600;  // Maximum threshold for joystick movement.

bool joyMoved = false;  // Flag to check if the joystick was moved.
bool joyMoved2 = false;  // Secondary flag for joystick movement.

const unsigned long alienMoveInterval = 1000;  // Interval for alien movement.

// Matrix display variables
const byte matrixSize = 8;  // Size of the LED matrix (8x8).
unsigned long long lastMoved = 0;  // Last time the LED matrix was updated.
const int moveInterval = 150;  // Interval for moving objects on the matrix.

bool matrixChanged = true;  // Flag to check if the matrix display has changed.

// Initial state of the LED matrix
bool matrix[matrixSize][matrixSize] = {
  {1, 0, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 1, 1},
  {1, 0, 1, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 0, 1, 0, 0, 1},
  {1, 0, 0, 0, 1, 0, 1, 1},
  {1, 1, 1, 1, 1, 0, 1, 1}
};


// General logic variables
bool inMenu = true;  // Flag to indicate if the game is currently showing the menu.
bool inGame = false;  // Flag to indicate if the game is currently being played.
int blinkInterval = 300;  // Time interval (in milliseconds) for blinking elements in the game.
long long int lastBlink = 0;  // Timestamp of the last blink event.

long long int lastBoost = 0;  // Timestamp of the last boost event.
long long int lastBoostBlink = 0;  // Timestamp of the last boost blink event.
int blinkBoostInterval = 100;  // Time interval (in milliseconds) for blinking during a boost.
const int boostTime[] = {1500, 1250, 1000, 750, 750};  // Array defining boost duration for different levels.
const int boostInterval = 1550;  // Time interval (in milliseconds) between boosts.
const int boostScore[] = {100, 200, 300, 400, 550};  // Scores awarded for boosts at different levels.

bool lockedIn = true;  // Flag to indicate if a selection in the menu is locked in.
bool blinkState = false;  // Current state of blinking (true for visible, false for not visible).

int EEPROMNameAddress = 6;  // Starting address in EEPROM for storing player names.
int EEPROMSettingsAddress = 16;  // Starting address in EEPROM for storing game settings.

// Sound frequencies and durations
const int endGameSound = 500;  // Frequency (in Hz) for the end game sound.
const int boostSound = 1000;  // Frequency (in Hz) for the boost sound.
const int endLevelSound = 500;  // Frequency (in Hz) for the end level sound.
const int soundBoostDuration = 150;  // Duration (in milliseconds) for the sound boost.
const int soundEndDuration = 1000;  // Duration (in milliseconds) for the end sound.
bool sound = true;  // Flag to enable or disable sound.
bool firstTime = true;

// Struct definitions for game elements
struct Alien {
  int x, y;  // X and Y position of the alien.
  bool alive;  // Indicates whether the alien is alive.
  unsigned long lastMoveTime;  // Timestamp of the alien's last movement.
};

struct Player {
  int x, y;  // X and Y position of the player.
};

struct Bullet {
  int x, y;  // X and Y position of the bullet.
  bool active;  // Indicates whether the bullet is active.
};

const int NUM_ALIENS = 10;  // Number of aliens in the game (adjust based on display size and game design).
const int MAX_BULLETS = 5;  // Maximum number of bullets allowed on the screen at once.

// Game element instances
Player player;  // Player instance.
Alien aliens[NUM_ALIENS];  // Array to hold alien instances.
Bullet bullets[MAX_BULLETS];  // Array to hold bullet instances.

int playerHits = 0;  // Counter for the number of times the player has been hit.

// Byte arrays representing custom characters for the LCD display
byte arrow[matrixSize] = {
  B11000,
  B01100,
  B00110,
  B00011,
  B00110,
  B01100,
  B11000,
};

byte down[matrixSize] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10001,
  B01010,
  B00100,
};

byte up[matrixSize] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte message[matrixSize] = {
  B00000100, 
  B00000000,
  B01010100,
  B01010100,
  B01110100,
  B01010100,
  B01010100,
  B00000000
};

void setupSpaceInvaders() {

  // Initialize aliens at the start of the game.
  for (int i = 0; i < NUM_ALIENS; i++) {
    aliens[i].x = i; 
    aliens[i].y = 1; 
    aliens[i].alive = true;
    aliens[i].lastMoveTime = millis();  // Record the initial time for each alien.
  }
  // Initialize the player's position in the middle at the bottom row of the matrix.
  player.x = matrixSize / 2;
  player.y = matrixSize - 1; 

  // Initialize bullets and set them as inactive at the start.
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }
}


void spawnEnemy() {
  // Check if it's time to spawn a new enemy.
  if (millis() - lastEnemySpawnTime >= enemySpawnInterval) {
    // Choose a random column to place the enemy.
    int randomColumn = random(0, matrixSize);
    // Place the enemy at the top of the matrix.
    aliens[randomColumn].x = randomColumn; 
    aliens[randomColumn].y = 0; 
    aliens[randomColumn].alive = true;
    
    lastEnemySpawnTime = millis();  // Update the time an enemy was last spawned.
  }

  // Move aliens down the matrix and check for collisions.
  for (int i = 0; i < NUM_ALIENS; i++) {
    if (aliens[i].alive && millis() - aliens[i].lastMoveTime > alienMoveInterval) {
      aliens[i].y++;  // Move the alien down the matrix.
      aliens[i].lastMoveTime = millis();  // Update the last move time of the alien.

      // Check if the alien has reached the player.
      if (aliens[i].y >= matrixSize - 1) {
        if (aliens[i].x == player.x) {
          handleGameOver(); // Game over if the player is hit.
        } else {
          aliens[i].alive = false; // Remove the alien from the matrix if it passes the player.
        }
      }
    }
  }
}

void updateSpaceInvaders() {
    static unsigned long lastMoveTime = 0; // Keep track of the last move time for debounce.
    const unsigned long moveDelay = 200;   // Delay between moves, adjust as needed.

    int xValue = analogRead(A0);
    int yValue = analogRead(A1);
    // Move the player based on joystick input.
    if (millis() - lastMoveTime > moveDelay) {
        if (xValue < minThreshold) {
            if (player.x > 0) { // Ensure the player does not move out of bounds.
                player.x--;
                lastMoveTime = millis(); // Update last move time.
            }
        } else if (xValue > maxThreshold) {
            if (player.x < matrixSize - 1) { // Ensure the player does not move out of bounds.
                player.x++;
                lastMoveTime = millis(); // Update last move time.
            }
        }
    }

  // Manage bullet movement and collision detection.
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].y--; // Move the bullet up the matrix.
      if (bullets[i].y < 0) {
        bullets[i].active = false; // Deactivate the bullet if it leaves the matrix.
      } else {
        // Check for collision with aliens.
        for (int j = 0; j < NUM_ALIENS; j++) {
          if (aliens[j].alive && bullets[i].x == aliens[j].x && bullets[i].y == aliens[j].y) {
            aliens[j].alive = false; // Eliminate the alien on collision.
            bullets[i].active = false; // Deactivate the bullet after collision.
            gameScore += 10; // Update score for destroying an alien.
            break;
          }
        }
      }
    }
  }

  // Player firing a bullet logic.
  if (buttonPressed()) {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = player.x;
        bullets[i].y = player.y - 1;  // Start the bullet just above the player.
        bullets[i].active = true;
        break; // Fire one bullet at a time.
      }
    }
  }
}

void checkPlayerHit() {
  // Logic to check if the player is hit by an enemy.
  if (playerHits >= 3) {
    // End the game if the player is hit three times.
    handleGameOver();
  }
}

void drawSpaceInvaders() {
  static unsigned long lastPlayerBlinkTime = 0;
  static unsigned long lastBombBlinkTime = 0;
  static bool playerBlinkState = false;
  static bool bombBlinkState = false;

  unsigned long currentTime = millis();

  // Blinking logic for the player.
  if (currentTime - lastPlayerBlinkTime > playerBlinkInterval) {
    playerBlinkState = !playerBlinkState;
    lastPlayerBlinkTime = currentTime;
  }

  // Blinking logic for bombs.
  if (currentTime - lastBombBlinkTime > bombBlinkInterval) {
    bombBlinkState = !bombBlinkState;
    lastBombBlinkTime = currentTime;
  }
  lc.clearDisplay(0); // Clear the LED matrix before drawing.

  // Draw the player as a single point if in a blinking state.
  if (playerBlinkState) {
    lc.setLed(0, player.y, player.x, true);
  }

  // Draw aliens on the matrix.
  for (int i = 0; i < NUM_ALIENS; i++) {
    if (aliens[i].alive) {
      lc.setLed(0, aliens[i].y, aliens[i].x, true);
    }
  }

  // Draw bullets if in a blinking state.
  if (bombBlinkState) { 
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        lc.setLed(0, bullets[i].y, bullets[i].x, true);
      }
    }
  }
}

void setup() {
  // Initialize pins
  pinMode(swPin, INPUT_PULLUP);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  randomSeed(analogRead(0));

  // Initialize Serial communication
  Serial.begin(9600);

  // Load data from EEPROM
  loadDataEEPROM();

  // Update settings based on EEPROM data
  updateSettings();

  // Initialize the LCD and Matrix display
  lcdSetup();
  matrixSetup();

  // Setup cursors for menu navigation
  cursorsSetup();

  // Initialize Space Invaders game environment
  setupSpaceInvaders();
}


void lcdSetup()
{
  lcd.begin(16, 2);
  lcd.createChar(0, arrow);
  lcd.createChar(1, down);
  lcd.createChar(2, up);
  lcd.setCursor(3, 0);
  lcd.print("Welcome to");
  lcd.setCursor(3, 1);
  lcd.print(gameName);
  delay(2000);
  lcd.clear();
}

void matrixSetup()
{
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.clearDisplay(0);// clear screen
}

void cursorsSetup()
{
  menuCursorPos[0].lcdRow = 0;
  menuCursorPos[0].lcdCol = 0;
  menuCursorPos[0].Name = new char[strlen("Start") + 1];
  strcpy(menuCursorPos[0].Name, "Start");
  menuCursorPos[1].lcdRow = 0;
  menuCursorPos[1].lcdCol = 7;
  menuCursorPos[1].Name = new char[strlen("Settings") + 1];
  strcpy(menuCursorPos[1].Name, "Settings");
  menuCursorPos[2].lcdRow = 1;
  menuCursorPos[2].lcdCol = 7;
  menuCursorPos[2].Name = new char[strlen("Info") + 1];
  strcpy(menuCursorPos[2].Name, "Info");
  menuCursorPos[3].lcdRow = 1;
  menuCursorPos[3].lcdCol = 0;
  menuCursorPos[3].Name = new char[strlen("Score") + 1];
  strcpy(menuCursorPos[3].Name, "Score");
  settingsCursorPos[0].lcdRow = 0;
  settingsCursorPos[0].lcdCol = 0;
  settingsCursorPos[0].Name = new char[strlen("Name: ") + 1];
  strcpy(settingsCursorPos[0].Name, "Name: ");
  settingsCursorPos[1].lcdRow = 1;
  settingsCursorPos[1].lcdCol = 0;
  settingsCursorPos[1].Name = new char[strlen("Start level: ") + 1];
  strcpy(settingsCursorPos[1].Name, "Start level: ");
  settingsCursorPos[2].lcdRow = 0;
  settingsCursorPos[2].lcdCol = 0;
  settingsCursorPos[2].Name = new char[strlen("Contrast: ") + 1];
  strcpy(settingsCursorPos[2].Name, "Contrast: ");
  settingsCursorPos[3].lcdRow = 1;
  settingsCursorPos[3].lcdCol = 0;
  settingsCursorPos[3].Name = new char[strlen("Brightness: ") + 1];
  strcpy(settingsCursorPos[3].Name, "Brightness: ");
  settingsCursorPos[4].lcdRow = 0;
  settingsCursorPos[4].lcdCol = 0;
  settingsCursorPos[4].Name = new char[strlen("Matrix Bness: ") + 1];
  strcpy(settingsCursorPos[4].Name, "Matrix Bness:");
  settingsCursorPos[5].lcdRow = 1;
  settingsCursorPos[5].lcdCol = 0;
  settingsCursorPos[5].Name = new char[strlen("Clear score: ") + 1];
  strcpy(settingsCursorPos[5].Name, "Clear Score");
  settingsCursorPos[6].lcdRow = 0;
  settingsCursorPos[6].lcdCol = 0;
  settingsCursorPos[6].Name = new char[strlen("Volume: ") + 1];
  strcpy(settingsCursorPos[6].Name, "Volume: ");
  settingsCursorPos[7].lcdRow = 1;
  settingsCursorPos[7].lcdCol = 0;
  settingsCursorPos[7].Name = new char[strlen("Back: ") + 1];
  strcpy(settingsCursorPos[7].Name, "Back");
  scoreCursorPos[0].lcdRow = 0;
  scoreCursorPos[0].lcdCol = 0;
  scoreCursorPos[1].lcdRow = 1;
  scoreCursorPos[1].lcdCol = 0;
  scoreCursorPos[2].lcdRow = 0;
  scoreCursorPos[2].lcdCol = 0;
  scoreCursorPos[3].lcdRow = 1;
  scoreCursorPos[3].lcdCol = 0;
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}

void writeStringToEEPROM(int addrOffset, char* strToWrite)
{
  for (int i = 0; i < scoresSize - 1; i++)
  {
    EEPROM.update(addrOffset + i, strToWrite[i]);
  }
}

char* readStringFromEEPROM(int addrOffset)
{
  char* data = new char[4];
  for (int i = 0; i < scoresSize - 1; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[3] = '\0';
  return data;
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

bool verifName(char *str)
{
  /*if(strlen(str) != 3)
    return false;*/
  for (int i = 0 ; i < strlen(str); i++)
    if (str[i] < 'A' || str[i] > 'Z')
      return false;
  return true;
}

void loadDataEEPROM()
{
  int EEPROMvalue;
  for (int i = 0; i < EEPROMNameAddress; i += 2)
  {
    if (readIntFromEEPROM(i) > 0)
      highScore[i / 2] = readIntFromEEPROM(i);
    else
      highScore[i / 2] = 0;
  }
  for (int i = EEPROMNameAddress; i < EEPROMNameAddress + 7; i = i + 3)
  {
    if (verifName(readStringFromEEPROM(i)) == true)
      strcpy(highScoreNames[(i - EEPROMNameAddress) / 3], readStringFromEEPROM(i));
    else
      strcpy(highScoreNames[(i - EEPROMNameAddress) / 3], "UNK");
  }
  if (verifName(readStringFromEEPROM(EEPROMSettingsAddress)) == true)
    strcpy(currentUser, readStringFromEEPROM(EEPROMSettingsAddress));
  else
    strcpy(currentUser, "AAA");
  for (int i = 19; i < 25; i = i + 2) // addresses for the contrast, brightness and matrix brightness
  {
    EEPROMvalue = readIntFromEEPROM(i);
    if (i == 19)
    {
      if (EEPROMvalue < 0 || EEPROMvalue > 5)
        contrastValue = 3;
      else
        contrastValue = EEPROMvalue;
    }
    else if (i == 21)
    {
      if (EEPROMvalue < 0 || EEPROMvalue > 5)
        brightnessValue = 3;
      else
        brightnessValue = EEPROMvalue;
    }
    else if (i == 23)
    {
      if (EEPROMvalue < 0 || EEPROMvalue > 5)
        matrixBrightnessValue = 3;
      else
        matrixBrightnessValue = EEPROMvalue;
    }
  }
}

// function that will print the menu data
void printMenu()
{
  for (int i = 0 ; i < menuPositionsSize; i++)
  {
    lcd.setCursor(menuCursorPos[i].lcdCol + 1, menuCursorPos[i].lcdRow);
    lcd.print(menuCursorPos[i].Name);
  }
  lcd.setCursor(menuCursorPos[currentCursorMenuPos].lcdCol, menuCursorPos[currentCursorMenuPos].lcdRow);
  lcd.write(byte(0));
}

// function that will help us to navigate through the menu
void moveMenuCursor()
{
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  int lastPos = currentCursorMenuPos;

  if (xValue < minThreshold)
  {
    if (currentCursorMenuPos == 3)
      currentCursorMenuPos = 0;
    if (currentCursorMenuPos == 2)
      currentCursorMenuPos = 1;
  }

  if (xValue > maxThreshold)
  {
    if (currentCursorMenuPos == 0)
      currentCursorMenuPos = 3;
    if (currentCursorMenuPos == 1)
      currentCursorMenuPos = 2;
  }

  if (lastPos == currentCursorMenuPos)
  {
    if (yValue > maxThreshold)
    {
      if (currentCursorMenuPos == 1)
        currentCursorMenuPos = 0;
      if (currentCursorMenuPos == 2)
        currentCursorMenuPos = 3;
    }

    if (yValue < minThreshold)
    {
      if (currentCursorMenuPos == 0)
        currentCursorMenuPos = 1;
      if (currentCursorMenuPos == 3)
        currentCursorMenuPos = 2;
    }
  }

  if (lastPos != currentCursorMenuPos)
  {
    lcd.clear();
  }

}

bool buttonPressed() {
  static unsigned long lastDebounceTime = 0;
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(swPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > 50) {
    if (currentButtonState == LOW) {
      lastButtonState = currentButtonState;
      return true;
    }
  }

  lastButtonState = currentButtonState;
  return false;
}


/// here we will generate all the data which will be shown on the lcd
void displayMenu() {
  /// if we are on the main menu and the button is pressed then we have to enter in the current selection
  if (buttonPressed() == true && currentCursorMenuPos != -1)
  {
    lcd.clear();
    lastcurrentCursorMenuPos = currentCursorMenuPos;
    currentCursorMenuPos = -1;
    /// for every selectuib from the menu we have to to some intializations
    if (lastcurrentCursorMenuPos == 0)
    {
      lcd.clear();
      gameEnded = false;
      gameStarted = false;
      inGame = false;
      gameScore = 0;
      totalScore = (currentLevel - 1) * 2000;
      scoreUpdated = false;
      newHighScore = false;
      // waitAnimationDisplay
    }
    if (lastcurrentCursorMenuPos == 1)
    {
      lockedIn = true;
      joyMoved = false;
      joyMoved2 = false;
      currentSettingsPos = 0;
      lastSettingsPos = 0;
    }
    if (lastcurrentCursorMenuPos == 3)
    {
      joyMoved = false;
      joyMoved2 = false;
      currentScorePos = 0;
      lastScorePos = 0;
    }
  }
  /// we print the main menu if we did not select any field
  if (currentCursorMenuPos != -1)
  {
    printMenu();
    moveMenuCursor();
  }
  else if (currentCursorMenuPos == -1)
  {
    /// otherwise, we see what field we chose and develop the logic from that field
    if (lastcurrentCursorMenuPos == 0)
    {
      inGame = true;
      displayGame();
    }
    if (lastcurrentCursorMenuPos == 1)
    {
      displaySettings();
      // lockedIn will tell us if we chose a field from settings or not
      if (buttonPressed() == true)
      {
        if (lockedIn == true)
        {
          if (currentSettingsPos == settingsSize - 1)
          {
            lcd.clear();
            lockedIn = true;
            currentCursorMenuPos = lastcurrentCursorMenuPos;
            return;
          }
          else if (currentSettingsPos == settingsSize - 3)
          {
            clearHighScore();
          }
          else if (currentSettingsPos == settingsSize - 2)
          {
            lcd.clear();
            sound = !sound;
          }
          else
          {
            lockedIn = false;
            lastBlink = millis();
          }
        }
        else if (lockedIn == false)
        {
          lockedIn = true;
          joyMoved = false;
          joyMoved2 = false;
          userRow = 0;
          userCol = 6;
          updateSettings();
          clearDisplay();
          firstTime = true;
        }
      }
      if (lockedIn == false)
      {
        modifySettings();
      }
    }
    if (lastcurrentCursorMenuPos == 2)
    {
      // we press the button to exit info
      displayInfo();
      if (buttonPressed() == true)
      {
        lcd.clear();
        currentCursorMenuPos = lastcurrentCursorMenuPos;
      }
    }
    if (lastcurrentCursorMenuPos == 3)
    {
      displayScore();
      if (buttonPressed() == true)
      {
        if (currentScorePos == scoresSize - 1)
        {
          lcd.clear();
          currentCursorMenuPos = lastcurrentCursorMenuPos;
          return;
        }
      }
    }
  }
}

// function which will clear our highscores
void clearHighScore()
{
  for (int i = 0; i < EEPROMNameAddress + 9; i++)
    EEPROM.update(i, 0);
  for (int i = 0; i < scoresSize - 1; i++)
  {
    highScore[i] = 0;
    strcpy(highScoreNames[i], "UNK");
  }
}

// function that will help us to navigate in the score section
void moveScoreCursor()
{
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  int lastPos = currentScorePos;

  if (xValue < minThreshold && joyMoved == false)
  {
    if (currentScorePos > 0)
    {
      currentScorePos--;
      joyMoved = true;
    }
  }

  if (xValue > maxThreshold && joyMoved == false)
  {
    if (currentScorePos < scoresSize - 1)
    {
      currentScorePos++;
      joyMoved = true;
    }
  }

  if (xValue > minThreshold && xValue < maxThreshold)
    joyMoved = false;

  if (lastPos != currentScorePos)
  {
    lcd.clear();
  }
  if (lastPos % 2 == 1 && currentScorePos == lastPos + 1) // in jos
  {
    lcd.clear();
    lastScorePos = currentScorePos;
  }
  if (lastPos % 2 == 0 && currentScorePos == lastPos - 1) // in sus
  {
    lcd.clear();
    lastScorePos = currentScorePos - 1;
  }
}

// function that will print the score section on the lcd

void printScore()
{

  lcd.setCursor(scoreCursorPos[currentScorePos].lcdCol, scoreCursorPos[currentScorePos].lcdRow);
  lcd.write(byte(0));
  for (int i = lastScorePos ; i < min(lastScorePos + 2, scoresSize); i++)
  {
    lcd.setCursor(scoreCursorPos[i].lcdCol + 1, scoreCursorPos[i].lcdRow);
    if (i < 3)
    {
      lcd.print(i + 1);
      lcd.print(".");
      lcd.print(highScoreNames[i]);
      lcd.print(": ");
      lcd.print(highScore[i]);
    }
    else if (i == 3)
      lcd.print("Back");
  }
  if (lastScorePos < scoresSize - 2)
  {
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }
  if (lastScorePos > 1)
  {
    lcd.setCursor(15, 0);
    lcd.write(byte(2));
  }
}
void displayScore()
{
  printScore();
  moveScoreCursor();
}

// here we update the scores
void updateScore()
{
  for (int i = 0 ; i < scoresSize - 1; i++)
  {
    // if final score is lower that the score from highScore or it is not updated at all, we move all the scores down, and put on the current position the final score
    if (totalScore < highScore[i] or highScore[i] == 0)
    {
      for (int j = scoresSize - 2; j > i; j--)
      {
        highScore[j] = highScore[j - 1];
        strcpy(highScoreNames[j], highScoreNames[j - 1]);
      }
      highScore[i] = totalScore;
      strcpy(highScoreNames[i], currentUser);
      if (i == 0)
        newHighScore = true;
      break;
    }
  }
  for (int i = 0 ; i < scoresSize - 1; i++)
  {
    writeIntIntoEEPROM(i * 2, highScore[i]);
  }
  for (int i = 0 ; i < scoresSize - 1; i++)
  {
    writeStringToEEPROM(i * 3 + EEPROMNameAddress, highScoreNames[i]);
  }
}

inline void copyMatrix(bool levelStartMatrixAux[8][8])
{
  for (int i = 0 ; i < matrixSize; i++)
    for (int j = 0 ; j < matrixSize; j++)
      matrix[i][j] = levelStartMatrixAux[i][j];
}





// here we print the infos at the start of the game/level
void printBeforeGame()
{
  lcd.setCursor(2, 0);
  lcd.print(startGameMessage1);
  lcd.setCursor(2, 1);
  lcd.print(startGameMessage2);
  lcd.print(currentLevel);
}

// here we print the infos at the end of the game
void printEndGame()
{
  if (newHighScore == false)
  {
    lcd.setCursor(0, 0);
    lcd.print("You scored:");
    lcd.print(totalScore);
    lcd.setCursor(0, 1);
    lcd.print("Press to go back");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("You scored:");
    lcd.print(totalScore);
    lcd.print(" New Best Score");
    if (millis() - lastScroll > scrollInterval)
    {
      lcd.scrollDisplayLeft();
      lastScroll = millis();
    }
    lcd.setCursor(0, 1);
    lcd.print("Press to go back");
  }
}

// game menu logic
void displayGame()
{
  if (gameEnded == true)
  {
    if ( scoreUpdated == false)
    {
      updateScore();
      scoreUpdated = true;
    }
    printEndGame();
    if (buttonPressed() == true)
    {
      lcd.clear();
      currentCursorMenuPos = lastcurrentCursorMenuPos;
      firstTime = true;
      gameEnded = false;
      gameStarted = false;
      inGame = false;
      gameScore = 0;
      totalScore = 0;
      clearDisplay();
    }
    return;
  }
  if (gameStarted == false)
  {
    printBeforeGame();
    if (buttonPressed() == true)
    {
      lcd.clear();
      gameStarted = true;
      gameEnded = false;
      // gameScore = 0;
      startScore = millis() / 10;
      lastLedBlink = millis();
      lastBoost = millis();
    }
  }
  else
  {
    lcd.setCursor(0, 0); 
    lcd.print("Scor: ");
    lcd.print(gameScore); 

  }
}

// here we update the settings
void updateSettings()
{
  lc.setIntensity(0, matrixBrightnessValue * 3);
  writeIntIntoEEPROM(19, contrastValue);
  writeIntIntoEEPROM(21, brightnessValue);
  writeIntIntoEEPROM(23, matrixBrightnessValue);
}

// here we change the settings values
void modifySettings()
{
  int auxValue;
  char auxName[4];
  int Size = 5;
  // if we change the name
  if (currentSettingsPos == 0)
  {
    strcpy(auxName, currentUser);
    int xValue = analogRead(xPin);
    int yValue = analogRead(yPin);

    int lastCol = userCol;

    if (yValue < minThreshold && joyMoved2 == false)
    {
      userCol++;
      joyMoved2 = true;
    }

    if (yValue > maxThreshold && joyMoved2 == false)
    {
      userCol--;
      joyMoved2 = true;
    }

    // we know we have the name from 6th to 8th position in the first row
    if (userCol < 6)
      userCol = 8;
    if (userCol > 8)
      userCol = 6;

    if (yValue > minThreshold && yValue < maxThreshold)
      joyMoved2 = false;

    if (userCol == lastCol)
    {
      if (xValue < minThreshold && joyMoved == false)
      {
        auxName[userCol - 6]++;
        joyMoved = true;
      }

      if (xValue > maxThreshold && joyMoved == false)
      {
        auxName[userCol - 6]--;
        joyMoved = true;
      }

      if (auxName[userCol - 6] > 'Z')
        auxName[userCol - 6] = 'A';
      if (auxName[userCol - 6] < 'A')
        auxName[userCol - 6] = 'Z';

      if (xValue > minThreshold && xValue < maxThreshold)
        joyMoved = false;

      if (strcmp(auxName, currentUser))
      {
        Serial.println(auxName);
        strcpy(currentUser, auxName);
        writeStringToEEPROM(EEPROMSettingsAddress, auxName);
        joyMoved2 = false;
      }
    }
  }
  // otherwise we have to change and integer value
  else
  {
    if (currentSettingsPos == 1)
      auxValue = currentLevel;
    if (currentSettingsPos == 2)
      auxValue = contrastValue;
    if (currentSettingsPos == 3)
      auxValue = brightnessValue;
    if (currentSettingsPos == 4)
      auxValue = matrixBrightnessValue;

    int xValue = analogRead(xPin);
    int yValue = analogRead(yPin);

    int lastValue = auxValue;
    if (xValue < minThreshold && joyMoved == false) {
      auxValue++;
      joyMoved = true;
    }

    if (xValue > maxThreshold && joyMoved == false) {
      auxValue--;
      joyMoved = true;
    }

    // for the starting level we have other boundaries
    if (currentSettingsPos == 1)
    {
      if (auxValue > Size)
        auxValue = 1;
      if (auxValue < 1)
        auxValue = Size;
    }
    else
    {
      if (auxValue > Size)
        auxValue = 0;
      if (auxValue < 0)
        auxValue = Size;
    }
    if (xValue > minThreshold && xValue < maxThreshold)
      joyMoved = false;

    if (auxValue != lastValue)
    {
      if (currentSettingsPos == 1)
        currentLevel = auxValue;
      if (currentSettingsPos == 2)
        contrastValue = auxValue;
      if (currentSettingsPos == 3)
        brightnessValue = auxValue;
      if (currentSettingsPos == 4)
      {
        matrixBrightnessValue = auxValue;
        lightDisplay();
      }
      // if something has changed we update the settings
      updateSettings();
    }
  }
}

void printSettings()
{
  // here we have the blink on the current field from settings
  if (lockedIn == false)
  {
    if (millis() - lastBlink > blinkInterval)
    {
      lastBlink = millis();
      blinkState = !blinkState;
      lcd.setCursor(settingsCursorPos[currentSettingsPos].lcdCol, settingsCursorPos[currentSettingsPos].lcdRow);
      if (blinkState == true)
        lcd.write(byte(0));
      else
        lcd.clear();
    }
  }
  else
  {
    lcd.setCursor(settingsCursorPos[currentSettingsPos].lcdCol, settingsCursorPos[currentSettingsPos].lcdRow);
    lcd.write(byte(0));
  }
  // here we print the sections from settings
  for (int i = lastSettingsPos ; i < min(lastSettingsPos + 2, settingsSize); i++)
  {
    lcd.setCursor(settingsCursorPos[i].lcdCol + 1, settingsCursorPos[i].lcdRow);
    lcd.print(settingsCursorPos[i].Name);
    if (i == 0)
      lcd.print(currentUser);
    else if (i == 1)
      lcd.print(currentLevel);
    else if (i == 2)
      lcd.print(contrastValue);
    else if (i == 3)
      lcd.print(brightnessValue);
    else if (i == 4)
      lcd.print(matrixBrightnessValue);
    else if (i == 6)
    {
      if (sound == true)
        lcd.print("ON");
      else
        lcd.print("OFF");
    }
  }
  if (lastSettingsPos < settingsSize - 2)
  {
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }
  if (lastSettingsPos > 1)
  {
    lcd.setCursor(15, 0);
    lcd.write(byte(2));
  }
}

// here we move through the settings fields
void moveSettingsCursor()
{
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  int lastPos = currentSettingsPos;
  if (xValue < minThreshold && joyMoved == false)
  {
    if (currentSettingsPos > 0)
    {
      currentSettingsPos--;
      joyMoved = true;
    }
  }

  if (xValue > maxThreshold && joyMoved == false)
  {
    if (currentSettingsPos < settingsSize - 1)
    {
      currentSettingsPos++;
      joyMoved = true;
    }
  }

  if (xValue > minThreshold && xValue < maxThreshold)
    joyMoved = false;

  if (lastPos != currentSettingsPos)
  {
    lcd.clear();
  }
  if (lastPos % 2 == 1 && currentSettingsPos == lastPos + 1) // in jos
  {
    lcd.clear();
    lastSettingsPos = currentSettingsPos;
  }
  if (lastPos % 2 == 0 && currentSettingsPos == lastPos - 1) // in sus
  {
    lcd.clear();
    lastSettingsPos = currentSettingsPos - 1;
  }
}

void displaySettings()
{
  printSettings();
  // just if we are lockedIn we can change the value of the settings
  if (lockedIn == true)
  {
    moveSettingsCursor();
  }
}

void displayInfo()
{
  lcd.setCursor(4, 0);
  lcd.print(gameName);
  lcd.print(" by ");
  lcd.print(authorName);
  lcd.setCursor(4, 1);
  lcd.print(githubLink);
  if (millis() - lastScroll > scrollInterval)
  {
    lcd.scrollDisplayLeft();
    lastScroll = millis();
  }
}

void loop() {
  // Read the joystick values
  // int xValue = analogRead(xPin); // Read the X-axis of the joystick
  // int yValue = analogRead(yPin); // Read the Y-axis of the joystick
  bool buttonState = digitalRead(swPin); // Read the joystick button state
  if (inGame == false && firstTime == true)
  {
    firstTime = false;
    printByte(message);
  }
  // Handle menu display and navigation if in menu mode
  if (inMenu) {
    displayMenu();
  }
  if (inGame) {
    
    // if we are in game, then we update the score
    if (millis() % 10 == 0 && gameStarted == true && gameEnded == false)
    {
      gameScore = millis() / 10 - startScore;
    }

    // Spawn enemies at intervals
    spawnEnemy();
    if (gameStarted) {
      updateSpaceInvaders();
      
     
      if (millis() - lastMoved > moveInterval)
      {
      
        lastMoved = millis();
      }
    
    // Check if the player has been hit by an enemy
    checkPlayerHit();

    // Update the display with the new game state
    drawSpaceInvaders(); }
    else {
      
      // Handle the game start logic
      printBeforeGame();
      if (buttonPressed()) {
        gameStarted = true;
      }
    }
    
    // Check if the game has ended and handle game over scenario
    if (gameEnded) {
      handleGameOver();
    }
  }

  // Small delay to debounce the button press and to control the frame rate
  delay(100);
}
void handleGameOver() {
    // Display Game Over Message
    lcd.clear();
    lcd.setCursor(0, 0); // Adjust position based on your LCD size
    lcd.print("GAME OVER");

    // Display Final Score
    lcd.setCursor(0, 1); // Adjust for next line
    lcd.print("Score: ");
    lcd.print(totalScore);

    // Optionally, play a game over sound if your hardware supports it
    if (sound) {
        tone(buzzerPin, endGameSound, soundEndDuration);
    }

    // Reset game variables for a new game
    gameEnded = false;
    gameStarted = false;
    inGame = false;
    // gameScore = 0;
    // totalScore += gameScore;
    currentLevel = 1; // Reset to initial level or to the last saved level
    // Reset player, aliens, and bullets positions and states here

    // Wait for player input to restart or go back to menu
    while (true) {
        if (buttonPressed()) {
            // Check if button is pressed to restart or go back to menu
            // Implement the logic to either restart the game or go back to the main menu
            // For example:
            inMenu = true;
            break; // Exit the loop to return to the main menu
        }
        delay(100); // Add a small delay to prevent rapid button press
    }
}


void updateDisplay() {
  for (int row = 0; row < matrixSize; row++)
  {
    for (int col = 0; col < matrixSize; col++)
    {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void lightDisplay()
{
  for (int row = 0; row < matrixSize; row++)
  {
    for (int col = 0; col < matrixSize; col++)
    {
      lc.setLed(0, row, col, 1);
    }
  }
}

void clearDisplay()
{
  for (int row = 0; row < matrixSize; row++)
  {
    for (int col = 0; col < matrixSize; col++)
    {
      lc.setLed(0, row, col, 0);
    }
  }
}


void printByte(byte character [])
{
  int i = 0;
  for (i = 0; i < matrixSize; i++)
  {
    lc.setRow(0, i, character[i]);
  }
}
