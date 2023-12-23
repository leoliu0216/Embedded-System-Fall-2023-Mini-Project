This repository is created for the Embedded Systems course project in the autumn of 2023 (grades: 20 / 20). The system developed is a simple game system based on the LCD1602 display. Programming is done using embedded C for the ATmega328P microcontroller. The project includes the following components:

1. Basic Input/Output
2. Interrupt Design
3. Game Content Design
4. Ultrasonic Distance Measurement
5. Serial Communication
6. PWM control for servos

Notably, this project does not utilize the Arduino library. A comprehensive report is available for reference.

The game has the following features:
1. There’re three main character with different appearance for you to choose from.
2. The three characters have different features.
3. The game with the first character will be in the normal mode, where you avoid being hit by enemies appearing in random rows of the screen.
4. The second character is able to shoot bullets; If you kill one of the enemies, your score will be added a certain value.
5. The third character is invincible. The game is in debug mode and the player will not be killed.
6. There’re three levels of difficulty. The enemy appearance, enemy health will change accordingly; for the final level, enemy will be able to shoot bullets.
7. There are bonus round after you finish a certain round, where there’s a bonus for you to catch to gain scores.
8. If the player survived the final round, they will be in the ultimate level, where a bonus will appearing randomly on the screen. If they manage to catch it, they won. Or they will be challenging level 3 again.
9. You can use the ultrasonic sensor to pause the game by decreasing the distance; while in pause mode, you can freely change any characters that you like.
10. Scores will be displayed on the screen all the way till the end of the game. Once the game ends, your score will be automatically sent to the upper computer.
11. There’s a welcome page at the beginning of the game. There will be page to indicate which level are you in once you’re about to level up. Also, there will be an ending page to tell your total scores.
