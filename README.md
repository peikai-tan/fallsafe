# LabProj-G18 - Team Whole Garden
## FallSafe

### Abstract
The elderly are prone to having poor balance, muscle weaknesses, slow reaction time and  joint problems due to old age. Additionally, medication for age-related illnesses may cause deterioration in cognitive and motor skills.[2] This puts them at a high risk of incurring serious injuries or a loss of life by a fall.
This project explores the possibility of lowering the chances of serious injuries and or a loss of life by detecting falls through a wearable.

### Equipment and Reasoning:
#### Controller:
- 1x Raspberry Pi 3 Model B+  
This is required by the project. Ideally, we would be using a Raspberry Pi Zero instead, as it is much smaller, and easier to convert into a wearable. 

#### Output:
- 1x Buzzer (I2C)  
This provides audio feedback to the user for when the device detects a hard fall. 

#### Input(s):
- 1x SenseHat (Comes with Gyroscope, Accelerometer and Joystick)  


This is what we currently possess, and suits our needs. However, a standalone gyroscope, accelerometer and button would make the entire package smaller, and use less electricity, which would be better suited for an actual product.  

We will be using a combination of the gyroscope and accelerometer data to detect if a fall has occurred through a neural network and defined thresholds that will be calculated using algorithms.The SenseHat module will be used to display the current status of the wearer.  

The joystick will allow the user to disable the buzzer when false-positive occurs.


| Team Members | ID | GitHub Name |
|--------------|----|-------------|
| Chen Yu    | 1901843 | Cerlancism |
| Ng Cheuk Fung | 16ADG049Y | chalkfung |
| Edwin Tang Qin Kang | 1901858 | TanglingTreats |
| Tan PeiKai | 1901853  | catkane-doodles |
| Tan Qi Guang    | 1901844  | qg25|
