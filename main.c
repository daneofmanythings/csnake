#include <ncurses.h>

#define SNEK_CAPACITY 10
#define SNEK_CHAR ' '
#define APPLE_CHAR '@'

typedef struct {
  int y;
  int x;
} Point;

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

void snek_undraw(const Snek *s, WINDOW *scr) {
  for (int i = 0; i < s->size; ++i) {
    mvwaddch(scr, s->body[i].y, s->body[i].x, SNEK_CHAR);
  }
}

void snek_draw(const Snek *s, WINDOW *scr) {

  attron(A_REVERSE);
  for (int i = 0; i < s->size; ++i) {
    mvwaddch(scr, s->body[i].y, s->body[i].x, SNEK_CHAR);
  }
  attroff(A_REVERSE);
}

void _snek_move(Snek *s, Point next, int pos) {
  if (pos == s->size) {
    return;
  }
  Point next_next = s->body[pos];
  s->body[pos] = next;
  _snek_move(s, next_next, pos + 1);
}

void snek_move_up(Snek *s) {
  Point next = {s->body[0].y - 1, s->body[0].x};
  _snek_move(s, next, 0);
}
void snek_move_down(Snek *s) {
  Point next = {s->body[0].y + 1, s->body[0].x};
  _snek_move(s, next, 0);
}
void snek_move_left(Snek *s) {
  Point next = {s->body[0].y, s->body[0].x - 1};
  _snek_move(s, next, 0);
}
void snek_move_right(Snek *s) {
  Point next = {s->body[0].y, s->body[0].x + 1};
  _snek_move(s, next, 0);
}

int main(void) {
  Point body[SNEK_CAPACITY];
  Point p0 = {2, 2};
  Point apple = {3, 3};

  Snek s = {.body = body, .capacity = SNEK_CAPACITY, .size = 0};
  for (int i = 0; i < 5; ++i) {
    snek_add_point(&s, p0);
  }

  initscr();
  curs_set(0);
  noecho();
  cbreak();
  keypad(stdscr, true);

  char ch;

  snek_draw(&s, stdscr);

  while (true) {
    ch = getch();
    snek_undraw(&s, stdscr);
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
    case 27:
      goto CLEANUP;
    default:
      continue;
    }
    snek_draw(&s, stdscr);
    refresh();
  }

  // mvaddch(apple.y, apple.x, APPLE_CHAR);
  // getch();

CLEANUP:
  endwin();
}
