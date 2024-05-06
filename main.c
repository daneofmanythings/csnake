#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define SNEK_CAPACITY 100
#define SNEK_CHAR ' '
#define APPLE_CHAR '@'

// ACS_ULCORNER	/* upper left corner */
// ACS_LLCORNER	/* lower left corner */
// ACS_URCORNER	/* upper right corner */
// ACS_LRCORNER	/* lower right corner */
// ACS_LTEE	/* tee pointing right */
// ACS_RTEE	/* tee pointing left */
// ACS_BTEE	/* tee pointing up */
// ACS_TTEE	/* tee pointing down */
// ACS_HLINE /* horizontal line */
// ACS_VLINE /* vertical line */

int rand_bounded_int(int min, int max) {
  // WARN: this isn't random i don't think
  srand(time(0));
  return (rand() % max - min + 1) + min;
}

typedef struct {
  int y;
  int x;
} Point;

bool point_is_equal(const Point *left, const Point *right) {
  return left->y == right->y && left->x == right->x;
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

typedef struct {
  Point apple;
  Point snek_head; // TODO: update to whole body.
  int y_start;
  int y_end;
  int x_start;
  int x_end;
  int refresh_per_second;
} GameData;

void gamedata_new_apple(GameData *gd) {
  while (point_is_equal(&gd->snek_head, &gd->apple)) {
    gd->apple.y = rand_bounded_int(gd->y_start, gd->y_end);
    gd->apple.x = rand_bounded_int(gd->x_start, gd->x_end);
  }
}

void gamedata_draw_box_to_window(GameData *gd, WINDOW *scr) {
  // ACS_ULCORNER	/* upper left corner */
  // ACS_LLCORNER	/* lower left corner */
  // ACS_URCORNER	/* upper right corner */
  // ACS_LRCORNER	/* lower right corner */
  // ACS_LTEE	/* tee pointing right */
  // ACS_RTEE	/* tee pointing left */
  // ACS_BTEE	/* tee pointing up */
  // ACS_TTEE	/* tee pointing down */
  // ACS_HLINE /* horizontal line */
  // ACS_VLINE /* vertical line */
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

void update_gamestate(GameData *gd, Snek *s) {
  // update snek positional data // TODO: update to whole body.
  gd->snek_head = s->body[0];

  // check snek positional data
  // 1. hit itself
  // a. lose game. TODO:

  // 2. hit boundary
  // a. lose game TODO:

  // 3. hit apple
  // a. increase snek length (snek_add_point)
  if (point_is_equal(
          &gd->snek_head,
          &gd->apple)) { // TODO: update logic when whole body is implemented.
    snek_add_point(s, gd->apple);
  }
  // b. generate random apple pos and update
  gamedata_new_apple(gd);
}

int main(void) {
  initscr();
  curs_set(0);
  noecho();
  cbreak();
  keypad(stdscr, true);

  Point body[SNEK_CAPACITY];
  Point p0 = {2, 2};
  Point apple = {3, 3};
  int rows, cols, height, width, starty, startx;
  getmaxyx(stdscr, rows, cols);

  starty = 1;
  startx = 1;
  height = rows - starty;
  width = cols - startx;

  Snek s = {.body = body, .capacity = SNEK_CAPACITY, .size = 0};
  GameData gd = {.apple = apple,
                 .snek_head = s.body[0],
                 .y_start = starty,
                 .y_end = height - starty,
                 .x_start = startx,
                 .x_end = width - startx,
                 .refresh_per_second = 1};
  for (int i = 0; i < 5; ++i) {
    snek_add_point(&s, p0);
  }

  char ch;

  while (true) {
    update_gamestate(&gd, &s);

    werase(stdscr);

    char *header =
        "rows, cols, (%d, %d), apple: {%d, %d}, y_end, x_end: [%d, %d]";
    printw(header, rows, cols, gd.apple.y, gd.apple.x, gd.y_end, gd.x_end);
    gamedata_draw_box_to_window(&gd, stdscr);
    snek_draw(&s, stdscr);
    mvwaddch(stdscr, gd.apple.y, gd.apple.x, APPLE_CHAR);

    wrefresh(stdscr);

    ch = getch();
    switch (ch) {
    case 'w':
      snek_move_up(&s);
      break;
    case 's':
      snek_move_down(&s);
      break;
    case 'a':
      snek_move_left(&s);
      break;
    case 'd':
      snek_move_right(&s);
      break;
    case 27: // ASCII Escape code
      goto CLEANUP;
    default:
      continue;
    }
  }

CLEANUP:
  endwin();
}
