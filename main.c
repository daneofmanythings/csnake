#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gamedata.h"

#define MS_PER_FRAME 35
#define SNEK_CAPACITY 100
#define SNEK_START_SIZE 5
#define SNEK_CHAR ' '
#define APPLE_CHAR '@'

typedef struct {
  int y;
  int x;
} Point;

enum Dir { UP, DOWN, LEFT, RIGHT };

bool point_is_equal(const Point *left, const Point *right) {
  return left->y == right->y && left->x == right->x;
}

bool point_collides_with_collection(const Point *collection,
                                    int collection_size, const Point *p) {
  for (int i = 0; i < collection_size; ++i) {
    if (point_is_equal(&collection[i], p)) {
      return true;
    }
  }
  return false;
}

typedef struct {
  Point *body;
  int capacity;
  int size;
} Snek;

void snek_add_point(Snek *s, Point p) {
  if (s->size == s->capacity) {
    return;
  }
  s->body[s->size] = p;
  s->size += 1;
}

bool snek_collides_with_itself(Snek *s) {
  for (int i = 1; i < s->size; ++i) {
    if (point_is_equal(&s->body[i], &s->body[0])) {
      return true;
    }
  }
  return false;
}

void snek_draw(const Snek *s, WINDOW *scr) {
  attron(A_REVERSE);
  for (int i = 0; i < s->size; ++i) {
    mvwaddch(scr, s->body[i].y, s->body[i].x, SNEK_CHAR);
  }
  attroff(A_REVERSE);
}

static void snek_move(Snek *s, Point next, int pos) {
  if (pos == s->size) {
    return;
  }
  Point next_next = s->body[pos];
  s->body[pos] = next;
  snek_move(s, next_next, pos + 1);
}

void snek_move_up(Snek *s) {
  Point next = {s->body[0].y - 1, s->body[0].x};
  snek_move(s, next, 0);
}
void snek_move_down(Snek *s) {
  Point next = {s->body[0].y + 1, s->body[0].x};
  snek_move(s, next, 0);
}
void snek_move_left(Snek *s) {
  Point next = {s->body[0].y, s->body[0].x - 1};
  snek_move(s, next, 0);
}
void snek_move_right(Snek *s) {
  Point next = {s->body[0].y, s->body[0].x + 1};
  snek_move(s, next, 0);
}

void snek_move_in_direction(Snek *s, enum Dir *dir) {
  switch (*dir) {
  case UP:
    snek_move_up(s);
    break;
  case DOWN:
    snek_move_down(s);
    break;
  case LEFT:
    snek_move_left(s);
    break;
  case RIGHT:
    snek_move_right(s);
    break;
  }
}
typedef struct {
  Point apple;
  Snek *snek;
  int y_start;
  int y_end;
  int x_start;
  int x_end;
  enum Dir current_dir;
} GameData;

void gamedata_new_apple(GameData *gd) {
  while (point_collides_with_collection(gd->snek->body, gd->snek->size,
                                        &gd->apple)) {
    gd->apple.y = rand_bounded_int(gd->y_start + 1, gd->y_end - 1);
    gd->apple.x = rand_bounded_int(gd->x_start + 1, gd->x_end - 1);
  }
}

bool gamedata_is_snek_at_border(GameData *gd) {
  return gd->x_start == gd->snek->body[0].x ||
         gd->x_end == gd->snek->body[0].x ||
         gd->y_start == gd->snek->body[0].y || gd->y_end == gd->snek->body[0].y;
}

void gamedata_set_new_direction(GameData *gd, const char *ch) {
  enum Dir new_dir;
  switch (*ch) {
  case 'w':
    new_dir = UP;
    if (gd->current_dir != DOWN) {
      gd->current_dir = UP;
    }
    break;
  case 's':
    new_dir = DOWN;
    if (gd->current_dir != UP) {
      gd->current_dir = DOWN;
    }
    break;
  case 'a':
    new_dir = LEFT;
    if (gd->current_dir != RIGHT) {
      gd->current_dir = LEFT;
    }
    break;
  case 'd':
    new_dir = RIGHT;
    if (gd->current_dir != LEFT) {
      gd->current_dir = RIGHT;
    }
    break;
  default:
    break;
  }
}

void gamedata_draw_box_to_window(GameData *gd, WINDOW *scr) {
  mvwaddch(scr, gd->y_start, gd->x_start, ACS_ULCORNER);
  mvwaddch(scr, gd->y_start, gd->x_end, ACS_URCORNER);
  mvwaddch(scr, gd->y_end, gd->x_start, ACS_LLCORNER);
  mvwaddch(scr, gd->y_end, gd->x_end, ACS_LRCORNER);

  for (int i = gd->x_start; i < gd->x_end - 1; ++i) {
    mvwaddch(scr, gd->y_start, gd->x_start + i, ACS_HLINE);
    mvwaddch(scr, gd->y_end, gd->x_start + i, ACS_HLINE);
  }
  for (int i = gd->y_start; i < gd->y_end - 1; ++i) {
    mvwaddch(scr, gd->y_start + i, gd->x_start, ACS_VLINE);
    mvwaddch(scr, gd->y_start + i, gd->x_end, ACS_VLINE);
  }
}

void update_gamestate(GameData *gd, const char *ch, WINDOW *scr) {
  // determine the direction
  gamedata_set_new_direction(gd, ch);
  snek_move_in_direction(gd->snek, &gd->current_dir);

  // check snek positional data
  // 1. hit itself or border
  if (snek_collides_with_itself(gd->snek) || gamedata_is_snek_at_border(gd)) {
    char *exit_msg = "GAME OVER";
    int y = (gd->y_end + gd->y_start) / 2;
    int x = (gd->x_end + gd->x_start) / 2 - strlen(exit_msg) / 2;
    mvprintw(y, x, "%s", exit_msg);
    getch();
    delwin(scr);
    endwin();
    exit(0);
  }

  // 2. hit apple
  // a. increase snek length (snek_add_point)
  if (point_is_equal(&gd->snek->body[0], &gd->apple)) {
    snek_add_point(gd->snek, gd->apple);
  }
  // b. generate random apple pos and update
  gamedata_new_apple(gd);
}

void draw_gamestate(GameData *gd, WINDOW *scr) {
  werase(scr);
  static char *header = "SCORE: %d";
  wprintw(scr, header, gd->snek->size - SNEK_START_SIZE, gd->apple.y,
          gd->apple.x);

  gamedata_draw_box_to_window(gd, scr);
  snek_draw(gd->snek, stdscr);
  mvwaddch(stdscr, gd->apple.y, gd->apple.x, APPLE_CHAR | A_BOLD);

  wrefresh(scr);
}

typedef struct {
  GameData *gd;
  const char *ch;
  WINDOW *scr;
} ThreadArgs;

void *threaded_run_game(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;

  while (true) {
    if (args->gd->current_dir == UP || args->gd->current_dir == DOWN) {
      napms(MS_PER_FRAME * 2);
    } else {
      napms(MS_PER_FRAME);
    }
    update_gamestate(args->gd, args->ch, args->scr);
    draw_gamestate(args->gd, args->scr);
  }
  return NULL;
}

int main(void) {
  initscr();
  curs_set(0);
  noecho();
  cbreak();
  keypad(stdscr, true);

  // setting up the snek
  Point body[SNEK_CAPACITY];
  Point snek_starting_pos = {2, 2};
  Snek s = {.body = body, .capacity = SNEK_CAPACITY, .size = 0};
  for (int i = 0; i < SNEK_START_SIZE; ++i) {
    snek_add_point(&s, snek_starting_pos);
  }

  // setting up the gamedata
  int rows, cols, height, width, y_start, x_start;
  getmaxyx(stdscr, rows, cols);
  y_start = 1;
  int y_end = rows - 2 * y_start;
  x_start = 1;
  int x_end = cols - 2 * x_start;
  GameData gd = {
      .apple = {rand_bounded_int(3, y_end), rand_bounded_int(3, x_end)},
      .snek = &s,
      .y_start = y_start,
      .y_end = y_end,
      .x_start = x_start,
      .x_end = x_end,
      .current_dir = RIGHT,
  };
  // gamedata_new_apple(&gd);

  // This variable is const referenced by the game_thread
  char ch;

  ThreadArgs ta = {
      .gd = &gd,
      .ch = &ch,
      .scr = stdscr,
  };

  pthread_t game_thread;
  pthread_create(&game_thread, NULL, threaded_run_game, (void *)&ta);

  while (true) {
    ch = getch();
  }

  endwin();
}
