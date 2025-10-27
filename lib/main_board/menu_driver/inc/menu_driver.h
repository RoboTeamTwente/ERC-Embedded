#ifndef MENU_DRIVER_H
#define MENU_DRIVER_H

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
struct menu_component;
struct menu_component_manager;

// A single struct to rule them all
typedef struct menu_component {
  // --- Tree Structure (The "Node" part) ---
  struct menu_component *parent;
  struct menu_component *children[MAX_CHILDREN]; // Pointer to first child
  uint8_t num_children;

  // --- Component Logic (The "Component" part) ---
  void *state; // Your custom data (e.g., button_state_t)

  // Function pointers for behavior
  void (*init)(struct menu_component *self);
  void (*destroy)(struct menu_component *self);
  void (*update)(struct menu_component *self, menu_input input);
  void (*render)(struct menu_component *self);
} menu_component;

typedef struct menu_component_manager {
  menu_component *active_component;
  char (*get_input)(void);
} menu_component_manager;

/**
 * Recursively destroys a component and all of its children.
 * This funciton performs a post-order traversal:
 * 1. It recursively calls itself on all children.
 * 2. After all children are destroyed, it calls the ocmponent's specific
 * destroy Function
 *
 * @Note It does *NOT* free any data, that needs to be handled through
 * comp->destroy
 */
void menu_component_destroy_recursive(menu_component *comp) {
  if (comp == NULL) {
    return;
  }
  for (int i = 0; i < comp->num_children; i++) {
    menu_component_destroy_recursive(comp->children[i]);
  }
  if (comp->destroy != NULL) {
    comp->destroy(comp);
  }
}

void menu_manager_switch_active_component(
    menu_component_manager *manager, menu_component *new_active_component) {
  if (manager == NULL || new_active_component == NULL) {
    return;
  }
  if (manager->active_component != NULL) {
    manager->active_component->destroy(manager->active_component);
  }
  manager->active_component = new_active_component;
  manager->active_component->init(manager->active_component);
}
void menu_component_manager_go_to_parent(menu_component_manager *manager) {
  if (manager == NULL || manager->active_component == NULL ||
      manager->active_component->parent == NULL) {
    return;
  }
  menu_manager_switch_active_component(manager,
                                       manager->active_component->parent);
}

void menu_component_manager_go_to_child(menu_component_manager *manager,
                                        uint8_t child_index) {
  if (manager == NULL || manager->active_component == NULL) {
    return;
  }
  if (child_index >= manager->active_component->num_children) {
    return;
  }
  menu_manager_switch_active_component(
      manager, manager->active_component->children[child_index]);
}
#endif // !MENU_DRIVER_H
