# Bouy Lights Simmulation program

## Introdution

This program is a basic simmulation of IALA bouy lights.

It is intended as a test bench for the logic to implement a physical map using an Arduino, that has the actual lights flashing as they would in real life to be used as a training tool.

The python code has been written in a way that should make it easy to port to C, so can be used in an Arduino. So there are no use of threads, or async, and the timing event loop has been implemented via pooling the clock, rather than using alarms and signals.

## Roadmap
Next step is to port this to an Arduino, and use programmable LEDs for the bouy lights

## Installation

Ensure that git and python exist on your system. This _should_ be Windows friendly, but has not been tested.

```shell
# Create a repository directory and change to it
mkdir mydir
cd mydir

# Get reposigory
git clone https://github.com/NZJourneyMan/Bouy_lights.git

# Create virtual environment
python -m venv venv
pip install --upgrade pip
pip install -r requirements.txt

# Start program
source venv/bin/activate
./bouy_lights.py

# Enjoy!
```