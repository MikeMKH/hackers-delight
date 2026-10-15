#include <criterion/criterion.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
  One row of the book's output table.
  Where the curve is after a step, and how it got there.

  dir    The value hilbert() passed to step(). It is a running count of
         quarter turns, so it can be negative or larger than 3. Only dir
         mod 4 says which way the step went: 0 = +x, 1 = +y, 2 = -x,
         3 = -y (see hilbert_heading()).
  index  How many steps from the start of the curve to this point. The
         book calls it "the length along the curve".
  x, y   The coordinates of the end of this step.

  Row 0 is the starting point, (0, 0), not a real step. Its dir is 0 because
  the pen arrives there from an imaginary point (-1, 0) by a step of 0.
*/
typedef struct {
  int32_t dir;
  uint32_t index;
  uint32_t x;
  uint32_t y;
} hilbert_point;

/*
  Where the output comes from. hilbert() only ever calls step(dir); it has no
  idea about coordinates, counting, or printing. The "pen" that does is this
  recorder, driven by hilbert_step(). It writes into a caller-supplied array.
*/
typedef struct {
  hilbert_point *points;  /* caller-supplied storage */
  size_t capacity;        /* how many points fit */
  size_t count;           /* steps taken so far, whether or not they fit */
  int32_t x, y;           /* where the pen is now */
} hilbert_recorder;

/* The largest order supported: 4^15 points still fits a 32-bit index. */
#define HILBERT_MAX_ORDER 15

static const int32_t DX[4] = { 1, 0, -1, 0 };
static const int32_t DY[4] = { 0, 1, 0, -1 };
static const char HEADING_NAME[4] = { 'E', 'N', 'W', 'S' };

/* 4^order, the number of points on a curve of that order. order must be 0..HILBERT_MAX_ORDER. */
size_t hilbert_point_count(int32_t order) { return (size_t)1 << (2 * order); }

/* dir mod 4 as 0..3 (0 = +x, 1 = +y, 2 = -x, 3 = -y), correct for negative dir. */
unsigned hilbert_heading(int32_t dir) {
  /*
    Converting to unsigned wraps modulo 2^32, a multiple of 4, so this is
    exactly dir mod 4 even when dir is negative.
  */
  return (unsigned)dir & 3u;
}

/* The book's step(): move the pen one unit along dir and record the point. */
void hilbert_step(hilbert_recorder *r, int32_t dir) {
  unsigned h = hilbert_heading(dir);
  r->x += DX[h];
  r->y += DY[h];
  if (r->count < r->capacity) {
    hilbert_point *p = &r->points[r->count];
    p->dir = dir;
    p->index = (uint32_t)r->count;
    p->x = (uint32_t)r->x;
    p->y = (uint32_t)r->y;
  }
  r->count++;
}

/*
  The book's hilbert(), unchanged except that the recorder is passed along.

  Draws a curve of the given order starting at the pen's position. dir is
  the direction from the start of the curve to its end, and rot (+1 or -1)
  is which way it bulges: +1 to the left of that direction, -1 to the right.
  Each level is four curves of the next order down joined by three steps.
*/
void hilbert(int32_t dir, int32_t rot, int32_t order, hilbert_recorder *r) {
  if (order == 0) return;
 
  dir = dir + rot;
  hilbert(dir, -rot, order - 1, r);
  hilbert_step(r, dir);
  dir = dir - rot;
  hilbert(dir, rot, order - 1, r);
  hilbert_step(r, dir);
  hilbert(dir, rot, order - 1, r);
  dir = dir - rot;
  hilbert_step(r, dir);
  hilbert(dir, -rot, order - 1, r);
}

static int32_t order_is_usable(int32_t order, size_t capacity) {
  return order >= 0 && order <= HILBERT_MAX_ORDER && capacity >= hilbert_point_count(order);
}

/*
  The whole thing: a Hilbert curve of the given order, starting at (0, 0),
  recorded into out. Returns the number of points written (4^order), or 0
  and writes nothing if the order is out of range or capacity is too small.
*/
size_t hilbert_curve(int32_t order, hilbert_point *out, size_t capacity) {
  if (!order_is_usable(order, capacity)) return 0;
 
  hilbert_recorder r = { out, capacity, 0, -1, 0 };
  hilbert_step(&r, 0);        /* arrive at (0, 0): row 0, the starting point */
  hilbert(0, 1, order, &r);
  return r.count;
}

/* Writes value as exactly width binary digits into buf (width + 1 bytes). */
void hilbert_to_binary(uint32_t value, int32_t width, char *buf) {
  for (int32_t i = 0; i < width; i++) {
    buf[i] = ((value >> (width - 1 - i)) & 1u) ? '1' : '0';
  }
  buf[width] = '\0';
}

/*
  Prints the book's table (dir, index in binary, x and y in binary) with
  an extra column spelling out the heading.
*/
void hilbert_print_table(const hilbert_point *points, size_t n, int32_t order) {
  int32_t bits = order > 0 ? order : 1;
  char ibuf[2 * HILBERT_MAX_ORDER + 1];
  char xbuf[HILBERT_MAX_ORDER + 1];
  char ybuf[HILBERT_MAX_ORDER + 1];
 
  printf(" dir  heading  index  x y\n");
  for (size_t i = 0; i < n; i++) {
    hilbert_to_binary(points[i].index, 2 * bits, ibuf);
    hilbert_to_binary(points[i].x, bits, xbuf);
    hilbert_to_binary(points[i].y, bits, ybuf);
    char heading = (i == 0) ? '-' : HEADING_NAME[hilbert_heading(points[i].dir)];
    printf("%4d    %c     %s  %s %s%s\n", points[i].dir, heading, ibuf, xbuf, ybuf,
           (i == 0) ? "   <- starting point, not a step" : "");
  }
}

/*
  Runs the same recursion as hilbert(), printing every call and every step
  as an indented tree. Calls with order 0 return immediately and are not
  shown. Records the curve into out and returns the number of points, like
  hilbert_curve(), so a test can confirm the trace matches the real thing.
*/
static void indent(int32_t depth) {
  for (int32_t i = 0; i < depth; i++) printf("  ");
}
 
static void trace_step(hilbert_recorder *r, int32_t dir, int32_t depth) {
  hilbert_step(r, dir);
  const hilbert_point *p = &r->points[r->count - 1];
  indent(depth);
  printf("step(%d)  heading %c  -> (%u,%u)  index %u\n",
         dir, HEADING_NAME[hilbert_heading(dir)], p->x, p->y, p->index);
}
 
static void trace_hilbert(int32_t dir, int32_t rot, int32_t order, int32_t depth, hilbert_recorder *r) {
  if (order == 0) return;
 
  indent(depth);
  printf("hilbert(dir=%d, rot=%+d, order=%d)\n", dir, rot, order);
 
  dir = dir + rot;
  trace_hilbert(dir, -rot, order - 1, depth + 1, r);
  trace_step(r, dir, depth + 1);
  dir = dir - rot;
  trace_hilbert(dir, rot, order - 1, depth + 1, r);
  trace_step(r, dir, depth + 1);
  trace_hilbert(dir, rot, order - 1, depth + 1, r);
  dir = dir - rot;
  trace_step(r, dir, depth + 1);
  trace_hilbert(dir, -rot, order - 1, depth + 1, r);
}

size_t hilbert_print_trace(int32_t order, hilbert_point *out, size_t capacity) {
  if (!order_is_usable(order, capacity)) return 0;
 
  hilbert_recorder r = { out, capacity, 0, -1, 0 };
  hilbert_step(&r, 0);
  printf("start: step(0) from the imaginary point (-1,0) -> (0,0)  index 0\n");
  trace_hilbert(0, 1, order, 0, &r);
  return r.count;
}

/*
  What the book's output table means, column by column, for order 2:
 
    dir     what hilbert() handed to step() for this move. 0 = +x, 1 = +y,
            2 = -x, 3 = -y, but counted as running quarter turns, so it also
            shows -1 and -2. dir mod 4 is the real direction.
    index   how many steps from the start: 0000, 0001, ... 1111 (4 bits,
            because a 4x4 grid has 16 points)
    x y     where the step ended, each 2 bits (a 4x4 grid)
 
  The first row is the starting point, reached from an imaginary (-1, 0).
  Nothing in hilbert() prints anything; the table is what the pen,
  hilbert_step(), saw as hilbert() called it.
*/
static const struct {
  int32_t dir;
  const char *index;
  const char *x;
  const char *y;
} BOOK_ORDER_2[16] = {
  {  0, "0000", "00", "00" },
  {  0, "0001", "01", "00" },
  {  1, "0010", "01", "01" },
  {  2, "0011", "00", "01" },
  {  1, "0100", "00", "10" },
  {  1, "0101", "00", "11" },
  {  0, "0110", "01", "11" },
  { -1, "0111", "01", "10" },
  {  0, "1000", "10", "10" },
  {  1, "1001", "10", "11" },
  {  0, "1010", "11", "11" },
  { -1, "1011", "11", "10" },
  { -1, "1100", "11", "01" },
  { -2, "1101", "10", "01" },
  { -1, "1110", "10", "00" },
  {  0, "1111", "11", "00" },
};
 
static const int32_t UNIT_DX[4] = { 1, 0, -1, 0 };
static const int32_t UNIT_DY[4] = { 0, 1, 0, -1 };
 
static hilbert_point *make_curve(int32_t order, size_t *n) {
  size_t capacity = hilbert_point_count(order);
  hilbert_point *points = malloc(capacity * sizeof *points);
  cr_assert(points != NULL, "out of memory for order %d", order);
  *n = hilbert_curve(order, points, capacity);
  cr_assert_eq(*n, capacity, "order %d", order);
  return points;
}
 
Test(curve, order_2_reproduces_the_books_table) {
  hilbert_point points[16];
  cr_assert_eq(hilbert_curve(2, points, 16), (size_t)16);
 
  for (int32_t i = 0; i < 16; i++) {
    char index[5], x[3], y[3];
    hilbert_to_binary(points[i].index, 4, index);
    hilbert_to_binary(points[i].x, 2, x);
    hilbert_to_binary(points[i].y, 2, y);
    cr_assert_eq(points[i].dir, BOOK_ORDER_2[i].dir, "row %d: dir", i);
    cr_assert_str_eq(index, BOOK_ORDER_2[i].index, "row %d: index", i);
    cr_assert_str_eq(x, BOOK_ORDER_2[i].x, "row %d: x", i);
    cr_assert_str_eq(y, BOOK_ORDER_2[i].y, "row %d: y", i);
  }
}
 
/*
  Order 0 is just the starting point. Order 1 is the "U" that every larger
  curve is made of: up, right, down.
*/
Test(curve, orders_0_and_1_are_the_base_cases) {
  hilbert_point order0[1];
  cr_assert_eq(hilbert_curve(0, order0, 1), (size_t)1);
  cr_assert_eq(order0[0].dir, 0);
  cr_assert_eq(order0[0].index, 0u);
  cr_assert_eq(order0[0].x, 0u);
  cr_assert_eq(order0[0].y, 0u);
 
  static const int32_t dirs[4] = { 0, 1, 0, -1 };       /* row 0 is the start */
  static const uint32_t xs[4] = { 0, 0, 1, 1 };
  static const uint32_t ys[4] = { 0, 1, 1, 0 };
  hilbert_point order1[4];
  cr_assert_eq(hilbert_curve(1, order1, 4), (size_t)4);
  for (int32_t i = 0; i < 4; i++) {
    cr_assert_eq(order1[i].dir, dirs[i], "row %d", i);
    cr_assert_eq(order1[i].x, xs[i], "row %d", i);
    cr_assert_eq(order1[i].y, ys[i], "row %d", i);
  }
}
 
/* dir counts quarter turns without wrapping; only dir mod 4 is a direction. */
Test(curve, heading_is_dir_mod_4_even_for_negative_dir) {
  static const struct { int32_t dir; unsigned heading; } cases[] = {
    { -5, 3 }, { -4, 0 }, { -3, 1 }, { -2, 2 }, { -1, 3 },
    {  0, 0 }, {  1, 1 }, {  2, 2 }, {  3, 3 },
    {  4, 0 }, {  5, 1 }, {  7, 3 },
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(hilbert_heading(cases[i].dir), cases[i].heading, "dir %d", cases[i].dir);
  }
}
 
/*
  The first column really is the direction of each move: the pen moved
  exactly one unit, along the compass direction dir mod 4. So -1 means
  "down", -2 means "left", and 5 would mean "up".
*/
Test(curve, dir_names_the_direction_of_every_move) {
  for (int32_t order = 1; order <= 8; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    for (size_t i = 1; i < n; i++) {
      unsigned h = hilbert_heading(p[i].dir);
      int32_t dx = (int32_t)p[i].x - (int32_t)p[i - 1].x;
      int32_t dy = (int32_t)p[i].y - (int32_t)p[i - 1].y;
      cr_assert_eq(dx, UNIT_DX[h], "order %d row %zu: dir %d", order, i, p[i].dir);
      cr_assert_eq(dy, UNIT_DY[h], "order %d row %zu: dir %d", order, i, p[i].dir);
    }
    free(p);
  }
}
 
/*
  The index column is just the row number, and there are 4^order points.
  Each level makes 4 copies of the level below and adds 3 steps, so the step
  count follows S(n) = 4 * S(n-1) + 3: 0, 3, 15, 63, 255, ...
*/
Test(curve, index_is_the_row_number_and_steps_follow_4s_plus_3) {
  size_t previous_steps = 0;
  for (int32_t order = 0; order <= 8; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    cr_assert_eq(n, (size_t)1 << (2 * order), "order %d", order);
    for (size_t i = 0; i < n; i++) {
      cr_assert_eq(p[i].index, (uint32_t)i, "order %d row %zu", order, i);
    }
    size_t steps = n - 1;     /* row 0 is the start, not a step */
    if (order > 0) {
      cr_assert_eq(steps, 4 * previous_steps + 3, "order %d", order);
    }
    previous_steps = steps;
    free(p);
  }
}
 
Test(curve, starts_at_the_origin_and_ends_at_the_far_end_of_the_bottom_edge) {
  for (int32_t order = 0; order <= 8; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    uint32_t side = 1u << order;
    cr_assert_eq(p[0].x, 0u, "order %d", order);
    cr_assert_eq(p[0].y, 0u, "order %d", order);
    cr_assert_eq(p[n - 1].x, side - 1, "order %d", order);
    cr_assert_eq(p[n - 1].y, 0u, "order %d", order);
    free(p);
  }
}
 
/* A space-filling curve: every point of the 2^n by 2^n grid, once, no gaps. */
Test(curve, visits_every_grid_point_exactly_once) {
  for (int32_t order = 0; order <= 8; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    uint32_t side = 1u << order;
    uint8_t *seen = calloc((size_t)side * side, 1);
    cr_assert(seen != NULL);
    for (size_t i = 0; i < n; i++) {
      cr_assert(p[i].x < side && p[i].y < side, "order %d row %zu left the grid", order, i);
      size_t cell = (size_t)p[i].y * side + p[i].x;
      cr_assert_eq(seen[cell], 0, "order %d: (%u,%u) visited twice", order, p[i].x, p[i].y);
      seen[cell] = 1;
    }
    /* n equals side*side and none repeat, so every cell was hit. */
    cr_assert_eq(n, (size_t)side * side, "order %d", order);
    free(seen);
    free(p);
  }
}
 
/*
  The recursion, seen from the output. An order-n curve is four order-(n-1)
  curves in a U: lower-left, upper-left, upper-right, lower-right. The three
  step() calls at the top level are the joints between them, and they are the
  order-1 curve's own moves: up (1), right (0), down (-1).
*/
Test(curve, each_level_is_four_quarters_in_a_u_joined_by_three_steps) {
  for (int32_t order = 1; order <= 7; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    size_t quarter = n / 4;
    uint32_t half = 1u << (order - 1);
 
    for (size_t i = 0; i < n; i++) {
      int32_t right = p[i].x >= half;
      int32_t upper = p[i].y >= half;
      size_t q = i / quarter;
      /* quarter 0: lower-left, 1: upper-left, 2: upper-right, 3: lower-right */
      int32_t want_right = (q == 2 || q == 3);
      int32_t want_upper = (q == 1 || q == 2);
      cr_assert_eq(right, want_right, "order %d row %zu", order, i);
      cr_assert_eq(upper, want_upper, "order %d row %zu", order, i);
    }
 
    cr_assert_eq(p[quarter].dir, 1, "order %d: joint into quarter 1", order);
    cr_assert_eq(p[2 * quarter].dir, 0, "order %d: joint into quarter 2", order);
    cr_assert_eq(p[3 * quarter].dir, -1, "order %d: joint into quarter 3", order);
    free(p);
  }
}
 
/*
  What the dir argument to hilbert() means: the direction from where a curve
  starts to where it ends. The four quarters are drawn by the calls
  hilbert(1, -1), hilbert(0, 1), hilbert(0, 1), hilbert(-1, -1), and each one
  does run from its first point to its last in that direction.
*/
Test(curve, each_quarter_runs_start_to_end_in_the_direction_of_its_dir_argument) {
  static const int32_t quarter_dir[4] = { 1, 0, 0, -1 };
  for (int32_t order = 1; order <= 7; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    size_t quarter = n / 4;
    int32_t span = (1 << (order - 1)) - 1;        /* side of a quarter, minus 1 */
 
    for (size_t q = 0; q < 4; q++) {
      const hilbert_point *first = &p[q * quarter];
      const hilbert_point *last = &p[(q + 1) * quarter - 1];
      unsigned h = hilbert_heading(quarter_dir[q]);
      cr_assert_eq((int32_t)last->x - (int32_t)first->x, span * UNIT_DX[h], "order %d quarter %zu", order, q);
      cr_assert_eq((int32_t)last->y - (int32_t)first->y, span * UNIT_DY[h], "order %d quarter %zu", order, q);
    }
    free(p);
  }
}
 
/*
  Independent oracle: the classic iterative index-to-coordinates conversion.
  No recursion and no dir or rot. It reads the index two bits at a time from
  the low end; each pair picks a quadrant and flips or transposes the smaller
  curve built so far.
*/
static void iterative_xy(uint32_t order, uint32_t d, uint32_t *x, uint32_t *y) {
  uint32_t px = 0, py = 0;
  for (uint32_t s = 1; s < (1u << order); s <<= 1) {
    uint32_t rx = 1u & (d >> 1);
    uint32_t ry = 1u & (d ^ rx);
    if (ry == 0) {
      if (rx == 1) {
        px = s - 1 - px;
        py = s - 1 - py;
      }
      uint32_t t = px;
      px = py;
      py = t;
    }
    px += s * rx;
    py += s * ry;
    d >>= 2;
  }
  *x = px;
  *y = py;
}
 
Test(curve, matches_an_independent_iterative_algorithm) {
  for (int32_t order = 1; order <= 8; order++) {
    size_t n;
    hilbert_point *p = make_curve(order, &n);
    for (size_t i = 0; i < n; i++) {
      uint32_t x, y;
      iterative_xy((uint32_t)order, (uint32_t)i, &x, &y);
      cr_assert_eq(p[i].x, x, "order %d index %zu", order, i);
      cr_assert_eq(p[i].y, y, "order %d index %zu", order, i);
    }
    free(p);
  }
}
 
Test(curve, to_binary_pads_to_the_requested_width) {
  char buf[8];
  hilbert_to_binary(0, 4, buf);   cr_assert_str_eq(buf, "0000");
  hilbert_to_binary(5, 4, buf);   cr_assert_str_eq(buf, "0101");
  hilbert_to_binary(2, 2, buf);   cr_assert_str_eq(buf, "10");
  hilbert_to_binary(15, 4, buf);  cr_assert_str_eq(buf, "1111");
}
 
/* Domain boundaries: refuse cleanly, and write nothing. */
Test(curve, rejects_bad_orders_and_small_buffers) {
  hilbert_point points[16];
  for (int32_t i = 0; i < 16; i++) points[i].dir = 12345;
 
  cr_assert_eq(hilbert_curve(-1, points, 16), (size_t)0);
  cr_assert_eq(hilbert_curve(HILBERT_MAX_ORDER + 1, points, 16), (size_t)0);
  cr_assert_eq(hilbert_curve(2, points, 15), (size_t)0, "one slot short");
 
  for (int32_t i = 0; i < 16; i++) {
    cr_assert_eq(points[i].dir, 12345, "slot %d was written by a rejected call", i);
  }
  cr_assert_eq(hilbert_curve(2, points, 16), (size_t)16, "exactly enough room");
}
 
/*
  No new assertions beyond "the trace and the real curve agree". Run with
  --verbose to see the table the book printed, then the recursion that
  produced it: every call, and every step with the heading and coordinates.
*/
Test(curve, table_and_trace) {
  hilbert_point direct[16];
  hilbert_point traced[16];
 
  cr_assert_eq(hilbert_curve(2, direct, 16), (size_t)16);
  
  printf("\n\n--Start Hilbert curve of order 2--\n");
  printf("\nThe book's order-2 table, with the heading spelled out:\n");
  hilbert_print_table(direct, 16, 2);
 
  printf("\nThe recursion that produces it:\n");
  cr_assert_eq(hilbert_print_trace(2, traced, 16), (size_t)16);
 
  for (int32_t i = 0; i < 16; i++) {
    cr_assert_eq(traced[i].dir, direct[i].dir, "row %d", i);
    cr_assert_eq(traced[i].index, direct[i].index, "row %d", i);
    cr_assert_eq(traced[i].x, direct[i].x, "row %d", i);
    cr_assert_eq(traced[i].y, direct[i].y, "row %d", i);
  }
  printf("\n--End of trace--\n\n");

/*
The book's order-2 table, with the heading spelled out:
 dir  heading  index  x y
   0    -     0000  00 00   <- starting point, not a step
   0    E     0001  01 00
   1    N     0010  01 01
   2    W     0011  00 01
   1    N     0100  00 10
   1    N     0101  00 11
   0    E     0110  01 11
  -1    S     0111  01 10
   0    E     1000  10 10
   1    N     1001  10 11
   0    E     1010  11 11
  -1    S     1011  11 10
  -1    S     1100  11 01
  -2    W     1101  10 01
  -1    S     1110  10 00
   0    E     1111  11 00

The recursion that produces it:
start: step(0) from the imaginary point (-1,0) -> (0,0)  index 0
hilbert(dir=0, rot=+1, order=2)
  hilbert(dir=1, rot=-1, order=1)
    step(0)  heading E  -> (1,0)  index 1
    step(1)  heading N  -> (1,1)  index 2
    step(2)  heading W  -> (0,1)  index 3
  step(1)  heading N  -> (0,2)  index 4
  hilbert(dir=0, rot=+1, order=1)
    step(1)  heading N  -> (0,3)  index 5
    step(0)  heading E  -> (1,3)  index 6
    step(-1)  heading S  -> (1,2)  index 7
  step(0)  heading E  -> (2,2)  index 8
  hilbert(dir=0, rot=+1, order=1)
    step(1)  heading N  -> (2,3)  index 9
    step(0)  heading E  -> (3,3)  index 10
    step(-1)  heading S  -> (3,2)  index 11
  step(-1)  heading S  -> (3,1)  index 12
  hilbert(dir=-1, rot=-1, order=1)
    step(-2)  heading W  -> (2,1)  index 13
    step(-1)  heading S  -> (2,0)  index 14
    step(0)  heading E  -> (3,0)  index 15
*/
}