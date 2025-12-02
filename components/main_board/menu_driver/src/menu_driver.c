#include "menu_driver.h"
#include <stdbool.h>
#include <sys/_types.h>

static char *TAG = "MENU DRIVER";

void menu_manager_switch_page(menu_manager_t *manager,
                              unsigned char new_page_id) {
  manager->pages[manager->active_page_id].destruct(
      manager->pages[manager->active_page_id].state);
  manager->active_page_id = new_page_id;
  manager->pages[manager->active_page_id].init(
      manager->pages[manager->active_page_id].state);
}
