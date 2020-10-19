# Python Files

## Auto-start pm2 (NodeJS Process Manager)
https://pm2.keymetrics.io/docs/usage/quick-start/

### Environment Checking and Setting up
1. Check NodeJS installation and version to be version 10 or above.  
    `node --version`

    Install NodeJS if needed: https://nodejs.org/en/download/package-manager/

1. `npm install -g pm2`

### Enabling Auto Start
1. `cd` to this folder  
    Examine `ecosystem.config.js`  
    Change `autorestart: false` to true for high availability (auto restart if it crashes) when desired.

1. `pm2 start ecosystem.config.js`  
    The script should be started

1. `pm2 startup`  
    Copy the generated command and execute it.  
    For example  
    `sudo env PATH=$PATH:/usr/bin /usr/lib/node_modules/pm2/bin/pm2 startup systemd -u pi --hp /home/pi`

### Disabling Auto Start
- `pm2 unstartup`
    Copy the generated command and execute it.  

### Killing it away
You can also stop the script from running anytime while still keeps autostart.  
- `pm2 kill`



## Auto-start SYSTEMD
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

### Deletion
1. Disable the service
`sudo systemctl disable dataGen.service`
2. Delete dataGen.service file from /usr/lib/systemd/system/
3. Reboot

## Instructions
- Strap the pi to your wrist
- **Ensure the pi's USB ports are facing your wrist, for consistent data collection.**
- Choose your activity using the joystick. r = running, w = walking, j = jumping, s = stationary
- Start the recording by clicking on the joystick. (LED grid should turn red)
- Proceed to do the activity
- End the recording by clicking on the joystick. (LED grid will be blank)
- Program will restart to allow you to start a new track
- Select "e" to exit
- Add and commit the auto generated file that is placed in the dataFiles folder

