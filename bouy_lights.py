#!/usr/bin/env python
"""
Sample program to demonstrate how marine lights could be simmulated. 
It is written with no object orientation to make it easier for someone with not a lot of
programming experience to port this to an Arduino. For this reason the clock is polled 
instead of using alarms and signals.
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
]
colour = {
    "red": Fore.RED,
    "green": Fore.GREEN,
    "white": Fore.WHITE,
    "black": Fore.BLACK
}
event_stack = []
max_event_stack = 5
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
    doDebug(json.dumps(event_stack, indent=2))
    doDebug(f"Time is {now}")

def updateLight(id, state):
    doDebug(f"Changing light {id} to {state}.")
    pass

def printLights():
    print(f"\r{Back.BLACK}            ", end="")
    for light in lights:
        state = getLightState(light["seq_num"])
        use_colour = colour[light["colour"]] if state == 1 else colour["black"]
        print(f"{use_colour}\u2588{Fore.BLACK}              ", end="", flush=True)

def doDebug(msg):
    if debug:
        print(f"{Style.RESET_ALL}\n{msg}", flush=True)

def main():
    parser = ArgumentParser()
    parser.add_argument("-d", dest="debug", help="Show debug output", action="store_true")
    args = parser.parse_args()
    global debug 
    debug = args.debug

    initColorama()
    initialiseLights()
    next_event_time = peekEventStack()["time"]
    print()
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

        sleep(0.01)  # Loop every 10ms to simulate polling on an arduino's due to lack of signals/alarms

         
         

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
         print(Style.RESET_ALL)