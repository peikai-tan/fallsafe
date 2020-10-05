# Python Files

## Auto-start
https://www.dexterindustries.com/howto/run-a-program-on-your-raspberry-pi-at-startup/ (Option 4)
1. Place dataGen.py at a any convenient location. (e.g ~/Documents/LabProj-G18/pythonFiles/dataGen.py)
2. Create dataGen.service in /usr/lib/systemd/system with the following contents  
```
[Unit]  
Description=Activity Tracker  
WantedBy=multi-user.target  
  
[Service]  
Type=simple  
WorkingDirectory=<absolute/path/to/>LabProj-G18/
ExecStart=/usr/bin/python3 <absolute/path/to>/dataGen.py  

[Install]
WantedBy=multi-user.target
```
3. Enable the service  
`sudo systemctl enable dataGen.service`
4. Reboot

## Instructions
- Strap the pi to your wrist
- Choose your activity using the joystick. r = running, w = walking, j = jumping, s = stationary
- Start the recording by clicking on the joystick. (LED grid should turn red)
- Proceed to do the activity
- End the recording by clicking on the joystick. (LED grid will be blank)
- Program will restart to allow you to start a new track
- Select "e" to exit
- Add and commit the auto generated file that is placed in the dataFiles folder

## Deletion
1. Disable the service
`sudo systemctl disable dataGen.service`
2. Delete dataGen.service file from /usr/lib/systemd/system/
3. Reboot
