module.exports = {
  apps: [{
    name: "SenseHat DataGen",
    cwd: "..", 
    script: "./pythonFiles/dataGen.py",
    exec_interpreter: "python3",
    autorestart: false,
  }]
};
