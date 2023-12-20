# Space Invaders - Arduino Project

## Overview
Space Invaders is a classic arcade game recreated using Arduino. This project demonstrates the use of LED matrices and LCD displays to create an interactive game experience. 

## Author
- **Name:** Ungureanu Antonia
- **GitHub:** [uantoniaa/SpaceInvaders](https://github.com/uantoniaa/SpaceInvaders)

## Hardware Requirements
- Arduino Board
- LED Matrix
- LCD Display
- Joystick Module
- Buzzer
- EEPROM for data storage

## Libraries Used
- `LedControl.h` for controlling LED matrices.
- `LiquidCrystal.h` for LCD display control.
- `EEPROM.h` for reading and writing to Arduino's EEPROM.

## Game Features
- Multiple levels with increasing difficulty.
- High score tracking and saving.
- Customizable player settings.
- Sound effects for an immersive experience.

## Pin Configurations
Refer to the source code for detailed pin configurations including LED matrix pins, LCD display pins, and joystick pins.

## How to Play
- Use the joystick to move the player left or right.
- Press the joystick button to fire at incoming aliens.
- Avoid getting hit by the aliens and try to shoot them down.
- Advance through levels and beat your high score!

## Custom Characters
Custom byte arrays are used to represent various characters on the LCD, including arrows and symbols.

## EEPROM Usage
The game uses EEPROM to store high scores and player settings, ensuring that your data persists between sessions.

## Image with the Arduino Setup
![WhatsApp Image 2023-12-20 at 16 14 13_7fb633a7](https://github.com/uantoniaa/SpaceInvaders/assets/93488180/1eb76e2b-32b0-4af5-a320-b283bcbd50f6)

## Video 
[!Space Invaders - Video ](https://youtu.be/jYCBzDWW0-E)

## Code 


Enjoy playing Space Invaders on Arduino!
