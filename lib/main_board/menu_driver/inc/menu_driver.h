#ifndef MENU_DRIVER_H
#define MENU_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef unsigned char menu_input;

#define INPUT_DIRECTION_LEFT_FLAG 0
#define INPUT_DIRECTION_RIGHT_FLAG 1
#define INPUT_DIRECTION_TOP_FLAG 2
#define INPUT_DIRECTION_BOTTOM_FLAG 3
#define INPUT_BUTTON_PRESS_A_FLAG 4

#define IS_INPUT_DIRECTION_LEFT(x)                                             \
  (((x) & (1 << INPUT_DIRECTION_LEFT_FLAG)) != 0)

#define IS_INPUT_DIRECTION_RIGHT(x)                                            \
  (((x) & (1 << INPUT_DIRECTION_RIGHT_FLAG)) != 0)

#define IS_INPUT_DIRECTION_TOP(x) (((x) & (1 << INPUT_DIRECTION_TOP_FLAG)) != 0)

#define IS_INPUT_DIRECTION_BOTTOM(x)                                           \
  (((x) & (1 << INPUT_DIRECTION_BOTTOM_FLAG)) != 0)

#define IS_INPUT_BUTTON_PRESS_A(x)                                             \
  (((x) & (1 << INPUT_BUTTON_PRESS_A_FLAG)) != 0)

// --- Set Macros (SET_...) ---
// Sets a specific bit to 1.

#define SET_INPUT_DIRECTION_LEFT(x) ((x) |= (1 << INPUT_DIRECTION_LEFT_FLAG))

#define SET_INPUT_DIRECTION_RIGHT(x) ((x) |= (1 << INPUT_DIRECTION_RIGHT_FLAG))

#define SET_INPUT_DIRECTION_TOP(x) ((x) |= (1 << INPUT_DIRECTION_TOP_FLAG))

#define SET_INPUT_DIRECTION_BOTTOM(x)                                          \
  ((x) |= (1 << INPUT_DIRECTION_BOTTOM_FLAG))

#define SET_INPUT_BUTTON_PRESS_A(x) ((x) |= (1 << INPUT_BUTTON_PRESS_A_FLAG))

// --- Clear Macros (CLEAR_...) ---
// Sets a specific bit to 0.

#define CLEAR_INPUT_DIRECTION_LEFT(x) ((x) &= ~(1 << INPUT_DIRECTION_LEFT_FLAG))

#define CLEAR_INPUT_DIRECTION_RIGHT(x)                                         \
  ((x) &= ~(1 << INPUT_DIRECTION_RIGHT_FLAG))

#define CLEAR_INPUT_DIRECTION_TOP(x) ((x) &= ~(1 << INPUT_DIRECTION_TOP_FLAG))

#define CLEAR_INPUT_DIRECTION_BOTTOM(x)                                        \
  ((x) &= ~(1 << INPUT_DIRECTION_BOTTOM_FLAG))

#define CLEAR_INPUT_BUTTON_PRESS_A(x) ((x) &= ~(1 << INPUT_BUTTON_PRESS_A_FLAG))

#define MAX_CHILDREN 8
#define MAX_NAME_LEN 32
struct menu_component;
struct menu_component_manager;

// A single struct to rule them all
typedef struct menu_component {
  // --- Tree Structure (The "Node" part) ---
  struct menu_component *parent;
  struct menu_component **children;
  uint8_t num_children;
  char *name;
  bool needs_render;

  // --- Component Logic (The "Component" part) ---
  void *state; // Your custom data (e.g., button_state_t)

  // Function pointers for behavior
  void (*init)(struct menu_component_manager *self);
  void (*destroy)(struct menu_component_manager *self);
  void (*update)(struct menu_component_manager *self);
  void (*render)(struct menu_component_manager *self);
} menu_component;

typedef struct menu_component_manager {
  menu_component *active_component;
  menu_input (*get_input)(void);
} menu_component_manager;

menu_component get_simple_list_of_elements(char *name, char **elements,
                                           uint8_t num_elems);
void tick_menu_component_manager(menu_component_manager *manager);
#endif // !MENU_DRIVER_H
