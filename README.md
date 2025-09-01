# Buoy Lights Code for Arduino

## Introdution

This repo contains Arduino code to enable a builder to make a physical model of a harbour complete with IALA flashing lights on harbour structures like buoys, bridges, piers, etc. There is Arduino code for both Uno and AdaFruit Trinket boards.

The repo also contains a python demo program that was used to develop the logic that was used to create the Arduino code. Full disclosure, Claude AI did the conversion from the python code to the Uno and Trinket code.

The python code was written in a way to make it easy to port to C, so can be used in an Arduino. So there are no use of threads, or async, and the timing event loop has been implemented via polling the clock, rather than using alarms and signals.

## Installation

This was tested on WSL, so should be identical for Linux. Windows should not be super different

Ensure that git exists on your system. 

```shell
# Create a repository directory and change to it
mkdir mydir
cd mydir

# Get reposigory
git clone https://github.com/NZJourneyMan/buoy_lights.git
```

## Run on Python

Ensure python exists on your system.

```shell
# Create virtual environment
cd mydir
python -m venv venv
pip install --upgrade pip
pip install -r requirements.txt

# Start the python demo program program
cd python
source venv/bin/activate
./bouy_lights.py

# Enjoy!
```

## Arduino

There are two files. One for a Uno, and one for an AdaFruit Trinket. The trinket code has been modified to simplify the data structures and to allow for timing differences. More comments in the code. You will need to install the Arduino FastLED library. For people new to Arduino go to https://www.arduino.cc/en/software/ and install the Arduino IDE and follow the Getting Started guide: https://www.arduino.cc/en/Guide/

### ToDo

The trinket lights run much slower than the Uno, so there is likely some CPU scaling work that needs to be done on the trinket to get it to run to actual seconds.

## Changing the lights

The python code is fairly self explanatory. The sequence is measured in seconds, and the first item of the squence is how long the light is on, while the second is how long the light is off, and so on. 

In the Arduino code there is an extra field, and this is the length of the sequence. In the below code the sequence `{0.75, 0.95, 0.75, 2.75}` are the on and off timings, while the `4` signifies the length of the sequence. If the length of the sequence is changed, the length number must also be changed for the sequence to be shown correctly. 

```c
  // Channel Pair 1
  {
    "Channel Port 1",
    CRGB::Red,
    {0.75, 0.95, 0.75, 2.75},  // Sequence: On Time, Off Time, On Time, Off Time, ...
    4,  // <-- Sequence length
    -1
  },
```

**Notes:** 
1. The light sequence must be made up of **off** and **on** pairs. Anything else will cause erratic, undefined behaviour, and may cause the arduino to crash. 
1. The Trinket sequence numbers are in milliseconds, not seconds.
