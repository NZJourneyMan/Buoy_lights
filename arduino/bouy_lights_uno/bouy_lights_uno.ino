#include <FastLED.h>

// FastLED Configuration
#define NUM_LEDS 7
#define DATA_PIN 13
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// Light structure
struct Light {
  const char* name;
  CRGB colour;
  float sequence[16];  // Max sequence length
  int sequence_length;
  int seq_num;
};

// Event structure
struct Event {
  int light_id;
  unsigned long time;
};

// Globals
Light lights[7] = {
  // Channel Pair 1
  {
    "Channel Port 1",
    CRGB::Red,
    {0.75, 0.95, 0.75, 2.75},
    4,
    -1
  },
  {
    "Channel Starboard 1", 
    CRGB::Green,
    {0.7, 0.9, 0.7, 2.7},
    4,
    -1
  },
  // Channel Pair 2
  {
    "Channel Port 2",
    CRGB::Red,
    {1.0, 1.2, 1.0, 1.2, 1.0, 6.8},
    6,
    -1
  },
  {
    "Channel Starboard 2",
    CRGB::Green, 
    {0.95, 1.15, 0.95, 1.15, 0.95, 6.8},
    6,
    -1
  },
  // Pier 1
  {
    "Pier 1 Port",
    CRGB::Red,
    {0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.3},
    14,
    -1
  },
  {
    "Pier 1 Starboard",
    CRGB::Green,
    {0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.06, 0.04, 0.3},
    14,
    -1
  },
  // Cardinals
  {
    "Northern Cardinal 1",
    CRGB::White,
    {0.3, 0.5},
    2,
    -1
  }
};

Event event_stack[10];
int event_stack_size = 0;
const int max_event_stack = 10;
bool debug_mode = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(32);
  
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  Serial.println("Marine Lights Simulator Starting...");
  Serial.println("Send 'd' for debug mode, 's' to show light info");
  
  initialiseLights();
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'd') {
      debug_mode = !debug_mode;
      Serial.print("Debug mode: ");
      Serial.println(debug_mode ? "ON" : "OFF");
    } else if (cmd == 's') {
      showLights();
    }
  }
  
  unsigned long max_millis;
  if (event_stack_size > 0) {
    
    // Handle millis() rolling over - it takes about 50 days!
    // unsigned long current_millis = millis();

    // if current_millis > max_millis {
    //   max_millis = current_millis;
    // } else {
      
    // }
    
    unsigned long next_event_time = peekEventStack().time;
    if (millis() >= next_event_time) {
      Event event = popEventStack();
      Light* light = &lights[event.light_id];
      
      if (light->seq_num < light->sequence_length - 1) {
        light->seq_num++;
      } else {
        light->seq_num = 0;
      }
      
      doDebug("Light " + String(event.light_id) + " seq_num: " + String(light->seq_num));
      
      updateLight(event.light_id, getLightState(light->seq_num));
      
      unsigned long new_time = event.time + (unsigned long)(light->sequence[light->seq_num] * 1000);
      insertIntoEventStack(event.light_id, new_time);
      
      updateLEDs();
    }
  }
  
  delay(5);  // 5ms polling interval
}

Event mkEvent(int light_id, unsigned long event_time) {
  Event event;
  event.light_id = light_id;
  event.time = event_time;
  return event;
}

void insertIntoEventStack(int light_id, unsigned long event_time) {
  doDebug("Scheduling light " + String(light_id) + " at " + String(event_time));
  
  if (event_stack_size >= max_event_stack) {
    Serial.println("ERROR: Event stack full!");
    return;
  }
  
  // Check for empty stack
  if (event_stack_size == 0) {
    event_stack[0] = mkEvent(light_id, event_time);
    event_stack_size = 1;
    return;
  }
  
  // Find place to insert event (chronological order)
  for (int i = 0; i < event_stack_size; i++) {
    if (event_time < event_stack[i].time) {
      // Shift events to make room
      for (int j = event_stack_size; j > i; j--) {
        event_stack[j] = event_stack[j-1];
      }
      event_stack[i] = mkEvent(light_id, event_time);
      event_stack_size++;
      return;
    }
  }
  
  // No place found, append to end
  event_stack[event_stack_size] = mkEvent(light_id, event_time);
  event_stack_size++;
}

int getLightState(int sequence_id) {
  return 1 - (sequence_id % 2);
}

Event peekEventStack() {
  return event_stack[0];
}

Event popEventStack() {
  Event event = event_stack[0];
  
  // Shift remaining events
  for (int i = 0; i < event_stack_size - 1; i++) {
    event_stack[i] = event_stack[i + 1];
  }
  event_stack_size--;
  
  return event;
}

void initialiseLights() {
  unsigned long now = millis();
  
  randomSeed(analogRead(0));
  for (int i = 0; i < 7; i++) {
    // Calculate total sequence time
    float total_time = 0;
    for (int j = 0; j < lights[i].sequence_length; j++) {
      total_time += lights[i].sequence[j];
    }
    
    // Random start time within sequence period
    unsigned long start_time = now + (unsigned long)((float)random(1000) / 1000.0 * total_time * 1000);
    insertIntoEventStack(i, start_time);
  }
  
  doDebug("Lights initialized at time: " + String(now));
}

void updateLight(int id, int state) {
  doDebug("Changing light " + String(id) + " to " + String(state));
  // This function is called but actual LED update happens in updateLEDs()
}

void updateLEDs() {
  // Update each LED based on light state
  for (int i = 0; i < NUM_LEDS && i < 7; i++) {
    int state = getLightState(lights[i].seq_num);
    if (state == 1) {
      leds[i] = lights[i].colour;
    } else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}

void showLights() {
  Serial.println("\n=== Light Configuration ===");
  for (int i = 0; i < 7; i++) {
    Serial.print("Name: ");
    Serial.println(lights[i].name);
    Serial.print("Colour: ");
    if (lights[i].colour == CRGB::Red) Serial.println("Red");
    else if (lights[i].colour == CRGB::Green) Serial.println("Green"); 
    else if (lights[i].colour == CRGB::White) Serial.println("White");
    else Serial.println("Unknown");
    Serial.print("Sequence: ");
    for (int j = 0; j < lights[i].sequence_length; j++) {
      Serial.print(lights[i].sequence[j]);
      if (j < lights[i].sequence_length - 1) Serial.print(", ");
    }
    Serial.println("\n");
  }
}

void doDebug(String msg) {
  if (debug_mode) {
    Serial.println("[DEBUG] " + msg);
  }
}