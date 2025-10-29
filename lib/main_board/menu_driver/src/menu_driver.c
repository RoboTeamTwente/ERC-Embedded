#include "menu_driver.h"
#include "logging.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types.h>

static char *TAG = "MENU DRIVER";

// Declarations
void menu_component_manager_go_to_parent(menu_component_manager *);
void menu_component_destroy_recursive(menu_component *);
void menu_component_manager_switch_active_component(menu_component_manager *,
                                                    menu_component *);
void menu_component_manager_go_to_child(menu_component_manager *, uint8_t);

typedef struct menu_component_list_state {
  unsigned char current_index;
  const SSD1306_Font_t *title_font;
  const SSD1306_Font_t *list_font;
  char selected_entry_prefix;
  bool is_light; // defines wether to use white for background and black for
  // text;
} menu_component_list_state;

void init(menu_component_manager *self) { self->active_component->state = 0; }

void menu_list_update(menu_component_manager *self) {

  menu_component_list_state *list_state =
      (menu_component_list_state *)self->active_component->state;
  menu_input input = self->get_input();

  if (IS_INPUT_DIRECTION_LEFT(input)) {
    menu_component_manager_go_to_parent(self);
    self->active_component->needs_render = true;
  } else if (IS_INPUT_BUTTON_PRESS_A(input) ||
             IS_INPUT_DIRECTION_RIGHT(input)) {
    menu_component_manager_go_to_child(self, list_state->current_index);
    self->active_component->needs_render = true;
  } else if (IS_INPUT_DIRECTION_BOTTOM(input)) {
    list_state->current_index =
        list_state->current_index < self->active_component->num_children - 1
            ? list_state->current_index + 1
            : 0;
    LOGI(TAG, "Updating input...%d", list_state->current_index);
    self->active_component->needs_render = true;
  } else if (IS_INPUT_DIRECTION_TOP(input)) {
    list_state->current_index = list_state->current_index == 0
                                    ? self->active_component->num_children - 1
                                    : list_state->current_index - 1;
    self->active_component->needs_render = true;
  }
}

/**
 * Calculates the starting render index for a centered, clamped viewport.
 *
 * @param list_length         The total number of items in your list.
 * @param selected_index   The index (0-based) of the item to center.
 * @param max_visible_size [in/out] A pointer to an uint8_t.
 * On input, this is the maximum number of items
 * that can fit on screen.
 * On output, this value is modified to be the
 * *actual* number of items that will be rendered
 * (i.e., min(list_len, original_max_visible_size)).
 * @return                 The calculated 0-based start index for rendering.
 */
uint8_t get_list_viewport(uint8_t list_length, uint8_t selected_index,
                          uint8_t *max_visible_size) {
  uint8_t visible_size = *max_visible_size;
  if (list_length <= visible_size) {
    // Trivial case where length can fit on screen
    *max_visible_size = list_length;
    return 0;
  }
  uint8_t padding = visible_size / 2;

  uint8_t ideal_start = padding > selected_index ? 0 : selected_index - padding;
  // Last possible index we can start from
  uint8_t max_start = list_length - *max_visible_size;
  uint8_t actual_start = max_start < ideal_start ? max_start : ideal_start;
  LOGI(TAG, "Ideal start: %d, Max start: %d, Actual Start: %d", ideal_start,
       max_start, actual_start);
  return actual_start;
}

void menu_list_render(menu_component_manager *self) {
  uint8_t starting_x = 8, starting_y = 8, offset_x = 16, offset_y,
          num_of_visible_elements, text_color, height_of_title;

  menu_component_list_state *list_state =
      (menu_component_list_state *)self->active_component->state;

  LOGI(TAG, "Current index: %d of list %s", list_state->current_index,
       self->active_component->name);

  ssd1306_Fill(list_state->is_light ? White : Black);
  // Render Title:
  text_color = list_state->is_light ? Black : White;
  ssd1306_SetCursor(starting_x, starting_y);
  ssd1306_WriteStringUnderlinede(self->active_component->name,
                                 *list_state->title_font, text_color, 1,
                                 starting_x);
  height_of_title = starting_y + list_state->title_font->height + 2 +
                    3; // 2 for gap between underline and 3 for underline size;
  num_of_visible_elements =
      (SSD1306_HEIGHT - height_of_title) / list_state->list_font->height - 1;
  offset_y = starting_y + 3;
  uint8_t list_index_start =
      get_list_viewport(self->active_component->num_children,
                        list_state->current_index, &num_of_visible_elements);
  LOGI(TAG, "Max Elems: %d, Viewport start:%d", num_of_visible_elements,
       list_index_start);

  uint8_t current_line = height_of_title;

  for (uint8_t i = 0; i < num_of_visible_elements; i++) {
    uint8_t child_index = i + list_index_start;
    menu_component *child = self->active_component->children[child_index];

    ssd1306_SetCursor(offset_x, current_line);
    if (child_index == list_state->current_index) {
      ssd1306_WriteStringUnderlined(child->name, *list_state->list_font,
                                    text_color, 1);
      current_line += 4;
    } else {
      ssd1306_WriteString(child->name, *list_state->list_font, text_color);
    }
    current_line += list_state->list_font->height + 1;
  }
  ssd1306_UpdateScreen();
}

void simple_text_widget_render(menu_component_manager *self) {
  ssd1306_Fill(Black);
  ssd1306_SetCursor(8, 8);
  ssd1306_WriteString(self->active_component->name, Font_16x26, White);
}

menu_component get_simple_list_of_elements(char *name, char **elements,
                                           uint8_t num_elems) {
  menu_component **children = malloc(num_elems * sizeof(menu_component *));
  if (children == NULL) {
    LOGE(TAG, "Could not allocate children");
    return (menu_component){0};
  }

  for (int i = 0; i < num_elems; i++) {
    LOGI(TAG, "New child with name :%s", elements[i]);
    children[i] = malloc(sizeof(menu_component));
    *children[i] = (menu_component){.name = elements[i],
                                    .render = simple_text_widget_render};
    LOGI(TAG, "Became child: %s", children[i]->name);
  }

  // --- THIS IS THE FIX ---

  // 1. Allocate the state on the HEAP, not the stack
  menu_component_list_state *state_ptr =
      malloc(sizeof(menu_component_list_state));

  if (state_ptr == NULL) {
    // Handle allocation failure (and free 'children'!)
    free(children);
    return (menu_component){0};
  }

  // 2. Initialize the struct *at that heap address*
  *state_ptr = (menu_component_list_state){.current_index = 0,
                                           .list_font = &Font_6x8,
                                           .title_font = &Font_7x10,
                                           .is_light = false};

  menu_component list = {.children = children,
                         .num_children = num_elems,
                         .name = name,
                         .update = menu_list_update,
                         .render = menu_list_render,
                         .needs_render = true,
                         .state = state_ptr};
  return list;
}

void tick_menu_component_manager(menu_component_manager *manager) {
  manager->active_component->update(manager);
  if (manager->active_component->needs_render) {
    manager->active_component->render(manager);
    manager->active_component->needs_render = false;
  }
}

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

void menu_component_manager_switch_active_component(
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
  menu_component_manager_switch_active_component(
      manager, manager->active_component->parent);
}

void menu_component_manager_go_to_child(menu_component_manager *manager,
                                        uint8_t child_index) {
  if (manager == NULL || manager->active_component == NULL) {
    return;
  }
  if (child_index >= manager->active_component->num_children) {
    return;
  }
  menu_component_manager_switch_active_component(
      manager, manager->active_component->children[child_index]);
}
