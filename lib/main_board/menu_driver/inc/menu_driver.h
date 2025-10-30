#ifndef MENU_DRIVER_H
#define MENU_DRIVER_H

#define MAX_CHILDREN 15
#define MAX_PAGES 128
#define MAX_PAGE_NAME_LEN 64

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

struct menu_page;
struct menu_manager;
struct menu_page_binding;
enum binding_type;

typedef enum {
  BINDING_TYPE_INT,
  BINDING_TYPE_FLOAT,
  BINDING_TYPE_STR,
  BINDING_TYPE_RESULT,
} binding_type;

/**
 * @brief creates a binding between a key and a piece of data
 */
typedef struct {
  const char *key;   // Name of the binding
  void *value_ptr;   // Address of the binding value
  binding_type type; // Type of the binding value
} menu_page_binding;

/**
 * @brief State of a List page node, stores the current index;
 */
typedef struct {
  unsigned char current_index;
} menu_page_state_list;

/**
 * @brief Union of all posisble page states.
 */
typedef union {
  menu_page_state_list list;
} menu_active_page_state;

/**
 */
typedef struct {
  void *state;
  unsigned char id;

  char name[MAX_PAGE_NAME_LEN];
  unsigned char children[MAX_CHILDREN];

  menu_page_binding *bindings;

  void (*init)(void *state);
  void (*update)(struct menu_manager *manager);
  void (*render)(struct menu_manager *manager);
  void (*destruct)(void *state);
} menu_page_t;

typedef struct {
  unsigned char active_page_id;
  menu_active_page_state active_state;
  menu_input (*get_input)(void);
} menu_manager_t;

static menu_page_t menu_pages_pool[MAX_PAGES];

#endif // !MENU_DRIVER_H
