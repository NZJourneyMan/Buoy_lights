#!/usr/bin/env python
"""
Sample program to demonstrate how marine lights could be simmulated. 
It is written with no object orientation, threading or async code to make it easier to port to an Arduino.
Polling is used as an Arduino does not have signals.
"""

import json
from random import random
from time import time, sleep
from colorama import Fore, Back, Style, init as initColorama
from argparse import ArgumentParser

# Globals
lights = [
    # Channel Pair 1
    {
        "name": "Channel Port 1",
        "colour": "red",
        "sequence": [0.7, 0.9, 0.7, 2.7],
        "seq_num": -1,
    },
    {
        "name": "Channel Starboard 1",
        "colour": "green",
        "sequence": [0.7, 0.9, 0.7, 2.7],
        "seq_num": -1,
    },
    # Channel Pair 2
    {
        "name": "Channel Port 2",
        "colour": "red",
        "sequence": [1.0, 1.2, 1.0, 6.8],
        "seq_num": -1,
    },
    {
        "name": "Channel Starboard 2",
        "colour": "green",
        "sequence": [1.0, 1.2, 1.0, 6.8],
        "seq_num": -1,
    },
    # Pier 1
    {
        "name": "Pier 1 Port",
        "colour": "red",
        "sequence": [
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.3,
        ],
        "seq_num": -1,
    },
    {
        "name": "Pier 1 Starboard",
        "colour": "green",
        "sequence": [
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.06,
            0.04, 0.3,
        ],
        "seq_num": -1,
    },
    # Cardinals
    {
        "name": "Northern Cardinal 1",
        "colour": "white",
        "sequence": [0.3, 0.5],
        "seq_num": -1,
    },
]
colour = {
    "red": Fore.RED,
    "green": Fore.GREEN,
    "white": Fore.WHITE,
    "black": Fore.BLACK
}
event_stack = []
max_event_stack = 10
debug = False

def mkEvent(light_id, event_time):
    """Creates an event structure"""
    return {
         "light_id": light_id,
         "time": event_time,
    }

def insertIntoEventStack(light_id, event_time):
    """This is a chronological stack, not a FIFO or a LIFO stack. For this reason each event has to be 
    inserted into the correct location in the stack, and not at the top/bottom."""
    doDebug(f"Scheduling light {light_id} at {event_time}.")
    if len(event_stack) >= max_event_stack:
        raise RuntimeError("Event stack full. Event not added")
    
    # Check for an empty stack
    if len(event_stack) == 0:
        event_stack.append(mkEvent(light_id, event_time))
        return
    # Find a place to insert event
    for i, event in enumerate(event_stack):
        if event_time < event["time"]:
            event_stack.insert(i, mkEvent(light_id, event_time))
            return

    # No place found to insert, so append event to the end of the stack
    event_stack.append(mkEvent(light_id, event_time))

def getLightState(sequence_id):
     return 1 - sequence_id % 2

def peekEventStack():
    return event_stack[0]

def popEventStack():
    event = event_stack[0]
    del event_stack[0]
    return event

def initialiseLights():
    now = time()
    for i, light in enumerate(lights):
        start_time = now + sum(light["sequence"]) * random()
        insertIntoEventStack(i, start_time)
    doDebug(f"Event Stack:\n{json.dumps(event_stack, indent=2)}")
    doDebug(f"Time is {now}")

def updateLight(id, state):
    doDebug(f"Changing light {id} to {state}.")
    pass

def printLights():
    BLOCK_GRAPHIC="\u2588"  # Unicode for block graphic
    print(f"\r{Back.BLACK}            ", end="")  # Print on the same line by using \r to return the cursor to line beginning
    for light in lights:
        state = getLightState(light["seq_num"])
        use_colour = colour[light["colour"]] if state == 1 else colour["black"]
        print(f"{use_colour}{BLOCK_GRAPHIC}{Fore.BLACK}              ", end="", flush=True)

def doDebug(msg):
    if debug:
        print(f"{Style.RESET_ALL}\n{msg}", flush=True)

def main():
    parser = ArgumentParser()
    parser.add_argument("-d", dest="debug", help="Show debug output", action="store_true")
    parser.add_argument("-s", dest="show", help="Show lights description", action="store_true")
    args = parser.parse_args()
    global debug 
    debug = args.debug
    if args.show:
        for light in lights:
            print(f"Name: {light['name']}\nColour: {light['colour']}\nSequence: {light['sequence']}\n")
        return

    initColorama()

    print("Press <Ctrl-C> to exit:\n")
    initialiseLights()
    next_event_time = peekEventStack()["time"]
    while True:
        if time() >= next_event_time:
            event = popEventStack()
            light = lights[event["light_id"]]
            if light["seq_num"] < len(light["sequence"]) - 1:
                light["seq_num"] += 1
            else:
                light["seq_num"] = 0
            doDebug(f"Seq num {light['seq_num']}")
            updateLight(event["light_id"], getLightState(light["seq_num"]))
            new_time = event["time"] + light["sequence"][light["seq_num"]]
            insertIntoEventStack(event["light_id"], new_time)
            next_event_time = peekEventStack()["time"]
            printLights()

        sleep(0.005)  # Loop every 5ms to simulate polling on an arduino due to an Arduino's lack of signals/alarms

         
         

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
         print(Style.RESET_ALL)