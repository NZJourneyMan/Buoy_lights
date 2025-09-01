#include <FastLED.h>

// FastLED Configuration for Trinket
#define NUM_LEDS 7
#define DATA_PIN 1          // Pin 1 on Trinket (GPIO #1)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// Light structure (optimized for Trinket's limited memory)
struct Light {
  uint8_t colour_r, colour_g, colour_b;  // Store RGB values instead of CRGB objects
  uint16_t sequence[8];     // Reduced max sequence length, store as milliseconds
  uint8_t sequence_length;
  int8_t seq_num;
};

// Event structure (optimized for memory)
struct Event {
  uint8_t light_id;
  uint32_t time;
};

// Globals (reduced for Trinket memory constraints)
Light lights[7] = {
  // Channel Pair 1
  {255, 0, 0, {700, 900, 700, 2700}, 4, -1},  // Red
  {0, 255, 0, {700, 900, 700, 2700}, 4, -1},  // Green
  
  // Channel Pair 2  
  {255, 0, 0, {1000, 1200, 1000, 6800}, 4, -1},  // Red
  {0, 255, 0, {1000, 1200, 1000, 6800}, 4, -1},  // Green
  
  // Pier 1 (simplified sequence to fit memory)
  {255, 0, 0, {40, 60, 40, 60, 40, 60, 40, 300}, 8, -1},  // Red
  {0, 255, 0, {40, 60, 40, 60, 40, 60, 40, 300}, 8, -1},  // Green
  
  // Cardinal
  {255, 255, 255, {300, 500}, 2, -1}  // White
};

Event event_stack[7];  // Reduced stack size for Trinket
uint8_t event_stack_size = 0;
const uint8_t max_event_stack = 7;

void setup() {
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(64);  // Reduced brightness for Trinket power limits
  
  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  // Small delay for startup
  delay(100);
  
  initialiseLights();
}

void loop() {
  if (event_stack_size > 0) {
    uint32_t next_event_time = peekEventStack().time;
    
    if (millis() >= next_event_time) {
      Event event = popEventStack();
      Light* light = &lights[event.light_id];
      
      if (light->seq_num < light->sequence_length - 1) {
        light->seq_num++;
      } else {
        light->seq_num = 0;
      }
      
      updateLight(event.light_id, getLightState(light->seq_num));
      
      uint32_t new_time = event.time + light->sequence[light->seq_num];
      insertIntoEventStack(event.light_id, new_time);
      
      updateLEDs();
    }
  }
  
  delay(10);  // 10ms polling - slightly slower to reduce CPU load
}

Event mkEvent(uint8_t light_id, uint32_t event_time) {
  Event event;
  event.light_id = light_id;
  event.time = event_time;
  return event;
}

void insertIntoEventStack(uint8_t light_id, uint32_t event_time) {
  if (event_stack_size >= max_event_stack) {
    return;  // Silent fail to save memory
  }
  
  // Check for empty stack
  if (event_stack_size == 0) {
    event_stack[0] = mkEvent(light_id, event_time);
    event_stack_size = 1;
    return;
  }
  
  // Find place to insert event (chronological order)
  for (uint8_t i = 0; i < event_stack_size; i++) {
    if (event_time < event_stack[i].time) {
      // Shift events to make room
      for (uint8_t j = event_stack_size; j > i; j--) {
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

uint8_t getLightState(int8_t sequence_id) {
  return 1 - (sequence_id % 2);
}

Event peekEventStack() {
  return event_stack[0];
}

Event popEventStack() {
  Event event = event_stack[0];
  
  // Shift remaining events
  for (uint8_t i = 0; i < event_stack_size - 1; i++) {
    event_stack[i] = event_stack[i + 1];
  }
  event_stack_size--;
  
  return event;
}

void initialiseLights() {
  uint32_t now = millis();
  
  for (uint8_t i = 0; i < 7; i++) {
    // Calculate total sequence time
    uint32_t total_time = 0;
    for (uint8_t j = 0; j < lights[i].sequence_length; j++) {
      total_time += lights[i].sequence[j];
    }
    
    // Simple pseudo-random start time (no random() to save memory)
    uint32_t start_time = now + (((uint32_t)i * 1000 + now) % total_time);
    insertIntoEventStack(i, start_time);
  }
}

void updateLight(uint8_t id, uint8_t state) {
  // Function exists for compatibility but actual update is in updateLEDs()
}

void updateLEDs() {
  // Update each LED based on light state
  for (uint8_t i = 0; i < NUM_LEDS && i < 7; i++) {
    uint8_t state = getLightState(lights[i].seq_num);
    if (state == 1) {
      leds[i] = CRGB(lights[i].colour_r, lights[i].colour_g, lights[i].colour_b);
    } else {
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}