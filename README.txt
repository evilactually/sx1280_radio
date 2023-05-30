LCD NOT WORKING:
- Moved original IOC file
- Verifyied pins with schematic
- Added defines for screen type
- Changed brightness pin from PWM to Output to be solid on, screen lit up for a moment
- Added into while loop HAL_GPIO_WritePin(E3_GPIO_Port,GPIO_PIN_10,GPIO_PIN_RESET);
- Seems the pin is active low
- There's issue with the timer TIM1
