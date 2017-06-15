#include "flappy_bird.h"

int main(int argc, char **argv) {
  object_list_t *objects = init_game();
  while (!bird_coll(objects)) {
    render_game(objects);
    char c = 0;

    c = getch();
    //printf("\n%c\n", c);
    if (c == ' ') {
      for_all(objects, flap);
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