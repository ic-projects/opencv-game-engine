#include "snake.h"

int main(int argc, char **argv) {
  object_list_t *objects = init_game();
  vector_t snake_dir = {.x = -1, .y = 0};
  while (!snake_hit(objects)) {
    render_game(objects, snake_dir);
    char c = 0;

    c = getch();
    //printf("\n%c\n", c);
    if (c == 'W') {
      snake_dir = (vector_t){.x = 0, .y = -1};
    }
    if (c == 'A') {
      snake_dir = (vector_t){.x = -1, .y = 0};
    }
    if (c == 'S') {
      snake_dir = (vector_t){.x = 0, .y = 1};
    }
    if (c == 'D') {
      snake_dir = (vector_t){.x = 1, .y = 0};
    }
//    for_all(objects, print_object);

    //usleep(100*1000);
  }
  sleep(5);
  endwin();
  printf("\nYou died!!!!\n");
  for_all(objects, print_object);
  return EXIT_SUCCESS;
}
