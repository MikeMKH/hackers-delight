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

void hil_xy_from_s(uint32_t s, int32_t n, uint32_t *xp,  uint32_t *yp) {
  uint32_t state, x, y, row;
  int32_t i;
  
  state = 0;
  x = y = 0;
  for (i = 2*n - 2; i >= 0; i -= 2) {
    row = 4*state | ((s >> i) & 3);
    x = (x << 1) | ((0x936C >> row) & 1);
    y = (y << 1) | ((0x39C6 >> row) & 1);
    state = ((0x3E6B94C1 >> 2*row) & 3);
  }
  /* pass back results */
  *xp = x;
  *yp = y;
}

Test(hil_xy_from_s, order_0_single_point) {
  /* Order 0: 2^0 = 1 point, only distance 0 is valid */
  uint32_t x, y;
  hil_xy_from_s(0, 0, &x, &y);
  cr_assert_eq(x, 0, "Order 0: x should be 0");
  cr_assert_eq(y, 0, "Order 0: y should be 0");
}

Test(hil_xy_from_s, order_1_four_points) {
  /*
    Order 1: 2^1 = 2 width/height, 4 points on curve
    Hilbert curve order 1 visits: (0,0) -> (0,1) -> (1,1) -> (1,0)
   */
  uint32_t x, y;
  
  hil_xy_from_s(0, 1, &x, &y);
  cr_assert_eq(x, 0, "Order 1, s=0: x should be 0");
  cr_assert_eq(y, 0, "Order 1, s=0: y should be 0");
  
  hil_xy_from_s(1, 1, &x, &y);
  cr_assert_eq(x, 0, "Order 1, s=1: x should be 0");
  cr_assert_eq(y, 1, "Order 1, s=1: y should be 1");
  
  hil_xy_from_s(2, 1, &x, &y);
  cr_assert_eq(x, 1, "Order 1, s=2: x should be 1");
  cr_assert_eq(y, 1, "Order 1, s=2: y should be 1");
  
  hil_xy_from_s(3, 1, &x, &y);
  cr_assert_eq(x, 1, "Order 1, s=3: x should be 1");
  cr_assert_eq(y, 0, "Order 1, s=3: y should be 0");
}

Test(hil_xy_from_s, order_2_sixteen_points) {
  /*
    Order 2: 2^2 = 4 width/height, 16 points on curve
    Verify a few key points and corners
   */
  uint32_t x, y;
  
  /* First point should be origin */
  hil_xy_from_s(0, 2, &x, &y);
  cr_assert_eq(x, 0, "Order 2, s=0: x should be 0");
  cr_assert_eq(y, 0, "Order 2, s=0: y should be 0");
  
  /* Last point should be in bounds [0, 3] */
  hil_xy_from_s(15, 2, &x, &y);
  cr_assert_leq(x, 3, "Order 2, s=15: x should be <= 3");
  cr_assert_leq(y, 3, "Order 2, s=15: y should be <= 3");
  cr_assert_geq(x, 0, "Order 2, s=15: x should be >= 0");
  cr_assert_geq(y, 0, "Order 2, s=15: y should be >= 0");
}

Test(hil_xy_from_s, order_3_all_in_bounds) {
  /*
    Order 3: 2^3 = 8 width/height, 64 points on curve
    All coordinates must stay within [0, 7]
   */
  uint32_t x, y;
  uint32_t max_s = (1U << 6) - 1; /* 2^(2*n) - 1 = 63 for n=3 */
  
  for (uint32_t s = 0; s <= max_s; s++) {
    hil_xy_from_s(s, 3, &x, &y);
    cr_assert_leq(x, 7, "Order 3, s=%u: x=%u exceeds bounds", s, x);
    cr_assert_leq(y, 7, "Order 3, s=%u: y=%u exceeds bounds", s, y);
    cr_assert_geq(x, 0, "Order 3, s=%u: x=%u below bounds", s, x);
    cr_assert_geq(y, 0, "Order 3, s=%u: y=%u below bounds", s, y);
  }
}

Test(hil_xy_from_s, order_2_connectivity) {
  /*
    Verify that consecutive points are adjacent (4-connected) 
    A Hilbert curve should be continuous without gaps
   */
  uint32_t x1, y1, x2, y2;
  
  for (uint32_t s = 0; s < 15; s++) {
    hil_xy_from_s(s, 2, &x1, &y1);
    hil_xy_from_s(s + 1, 2, &x2, &y2);
    
    uint32_t dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);
    uint32_t dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
    
    /* Points must be adjacent (Manhattan distance = 1) */
    uint32_t manhattan = dx + dy;
    cr_assert_eq(manhattan, 1, 
      "s=%u (%u,%u) to s=%u (%u,%u): not adjacent (distance=%u)",
      s, x1, y1, s+1, x2, y2, manhattan);
  }
}

Test(hil_xy_from_s, no_null_pointer_dereference) {
  uint32_t x, y;
  
  hil_xy_from_s(0, 2, &x, &y);
  cr_assert_eq(x, 0, "Origin x coordinate");
  cr_assert_eq(y, 0, "Origin y coordinate");
}



Test(hil_xy_from_s, order_4_large_grid) {
  /*
     Order 4: 2^4 = 16 width/height, 256 points
     Verify bounds and a sample of points
   */
  uint32_t x, y;
  uint32_t max_s = (1U << 8) - 1; /* 256 - 1 */
  
  /* Test first, middle, and last points */
  hil_xy_from_s(0, 4, &x, &y);
  cr_assert_eq(x, 0, "Order 4, s=0: should start at origin");
  cr_assert_eq(y, 0, "Order 4, s=0: should start at origin");
  
  hil_xy_from_s(127, 4, &x, &y);
  cr_assert_leq(x, 15, "Order 4, s=127: x in bounds");
  cr_assert_leq(y, 15, "Order 4, s=127: y in bounds");
  
  hil_xy_from_s(max_s, 4, &x, &y);
  cr_assert_leq(x, 15, "Order 4, s=255: x in bounds");
  cr_assert_leq(y, 15, "Order 4, s=255: y in bounds");
}

Test(hil_xy_from_s, symmetry_order_1) {
  /*
     Order 1 Hilbert curve has rotational properties
     Verify specific coordinate values
   */
  uint32_t x, y;
  
  /* Trace through order 1: (0,0)->(0,1)->(1,1)->(1,0) */
  const struct {
    uint32_t s;
    uint32_t expected_x;
    uint32_t expected_y;
  } points[] = {
    {0, 0, 0},
    {1, 0, 1},
    {2, 1, 1},
    {3, 1, 0},
  };
  
  for (size_t i = 0; i < sizeof(points)/sizeof(points[0]); i++) {
    hil_xy_from_s(points[i].s, 1, &x, &y);
    cr_assert_eq(x, points[i].expected_x, 
      "Order 1, s=%u: x mismatch", points[i].s);
    cr_assert_eq(y, points[i].expected_y,
      "Order 1, s=%u: y mismatch", points[i].s);
  }
}

Test(hil_xy_from_s, order_zero_boundary) {
  /* Edge case: order 0 is a 1x1 grid */
  uint32_t x, y;
  hil_xy_from_s(0, 0, &x, &y);
  cr_assert_eq(x, 0, "Order 0: x at origin");
  cr_assert_eq(y, 0, "Order 0: y at origin");
}

Test(hil_xy_from_s, increasing_orders_contain_previous) {
  /*
     Points from order n should correspond to order n+1
     (within the same quadrant structure)
   */
  uint32_t x1, y1, x2, y2;
  
  /* First point of order n should match first point of order n+1 */
  for (int n = 0; n < 4; n++) {
    hil_xy_from_s(0, n, &x1, &y1);
    hil_xy_from_s(0, n+1, &x2, &y2);
    cr_assert_eq(x1, 0, "Order %d->%d: origin x mismatch", n, n+1);
    cr_assert_eq(y1, 0, "Order %d->%d: origin y mismatch", n, n+1);
  }
}

Test(hil_xy_from_s, output_parameter_modification) {
  /* Ensure the function actually modifies the output parameters */
  uint32_t x = 0xDEADBEEF, y = 0xCAFEBABE;
  
  hil_xy_from_s(5, 2, &x, &y);
  
  /* After calling, both should be changed from their sentinel values and should be valid coordinates */
  cr_assert_neq(x, 0xDEADBEEF, "x parameter not modified");
  cr_assert_neq(y, 0xCAFEBABE, "y parameter not modified");
  cr_assert_leq(x, 3, "x for order 2 should be in [0,3]");
  cr_assert_leq(y, 3, "y for order 2 should be in [0,3]");
}

Test(hil_xy_from_s, deterministic_output) {
  /* Same input should always produce same output */
  uint32_t x1, y1, x2, y2;
  
  hil_xy_from_s(42, 3, &x1, &y1);
  hil_xy_from_s(42, 3, &x2, &y2);
  
  cr_assert_eq(x1, x2, "Function not deterministic for x");
  cr_assert_eq(y1, y2, "Function not deterministic for y");
}

Test(hil_xy_from_s, order_5_stress_test) {
  /*
     Order 5: 32x32 grid, 1024 points - stress test traversal
     Verify all points are unique and in bounds
   */
  uint32_t x, y;
  int seen[32][32] = {0};
  uint32_t max_s = (1U << 10) - 1; /* 1024 points */
  
  for (uint32_t s = 0; s <= max_s; s++) {
    hil_xy_from_s(s, 5, &x, &y);
    
    cr_assert_leq(x, 31, "Order 5, s=%u: x out of bounds", s);
    cr_assert_leq(y, 31, "Order 5, s=%u: y out of bounds", s);
    cr_assert_eq(seen[x][y], 0, "Order 5: duplicate at (%u,%u)", x, y);
    
    seen[x][y] = 1;
  }
}

/*
  One level of the Lam and Shapiro method (one row of Table 16-6).
 
  Reads bit i of *x and bit i of *y, which are (xi, yi), and does what the
  table says for that pair to the bits still to be read:
 
    (xi, yi)   then                         returns (append to s)
    (0, 0)     swap x and y                 0b00
    (0, 1)     no change                    0b01
    (1, 0)     swap and complement x, y     0b11
    (1, 1)     no change                    0b10
 
  "Complement" here means the whole 32-bit word, not only the bits below i.
  Domain: 0 <= i <= 31.
*/
uint32_t hil_lam_shapiro_step(uint32_t *x, uint32_t *y, int i) {
  uint32_t xi = (*x >> i) & 1u;     /* Get bit i of x. */
  uint32_t yi = (*y >> i) & 1u;     /* Get bit i of y. */

  /*
    Rows (0,0) and (1,0) of the table, the two with yi = 0: swap x and y,
    and when xi is 1 complement them too. -xi is 0 when xi is 0 and all ones
    when xi is 1 (unsigned wraparound, well defined), so xor-ing with it
    either leaves a word alone or complements it, with no branch.
  */
  if (yi == 0) {
    uint32_t temp = *x;
    *x = *y ^ (-xi);
    *y = temp ^ (-xi);
  }

  /*
    The two bits to append: 00, 01, 11, 10 for (xi, yi) = (0,0), (0,1),
    (1,0), (1,1). The high bit is xi. The low bit is xi ^ yi.
  */
  return 2u * xi + (xi ^ yi);
}

/*
  The Hilbert curve's index s for the point (x, y) on a curve of order n,
  i.e. how many steps along the curve it is. The inverse of walking the curve.
  Works from the top bit of x and y down, taking one table row per bit pair
  and appending two bits to s each time, so s has 2n bits.
 
  Domain: 0 <= n <= 16 (so that s fits in 32 bits). Bits of x and y at
  position n and above are ignored. Callers need not mask them.
*/
uint32_t hil_s_from_xy(uint32_t x, uint32_t y, int n) {
  uint32_t s = 0;
  for (int i = n - 1; i >= 0; i--) {
    s = 4u * s + hil_lam_shapiro_step(&x, &y, i); /* Append two bits to s. */
  }
  return s;
}

static const char *const ROW_ACTION[4] = {
  "swap x and y",                /* (0,0) */
  "no change",                   /* (0,1) */
  "swap and complement x and y", /* (1,0) */
  "no change",                   /* (1,1) */
};

static void print_bits(uint32_t value, int width) {
  for (int i = width - 1; i >= 0; i--) {
    putchar(((value >> i) & 1u) ? '1' : '0');
  }
}

/*
  The same computation, narrating every level: the bit pair read, which row
  of the table applies, what is appended to s, and what remains of x and y.
  Returns the same value as hil_s_from_xy().
*/
uint32_t hil_s_from_xy_print_trace(uint32_t x, uint32_t y, int n) {
  uint32_t s = 0;

  printf("hil_s_from_xy(x=%u, y=%u, n=%d)\n", x, y, n);
  printf("  start:  x = ");
  print_bits(x, n);
  printf("  y = ");
  print_bits(y, n);
  printf("\n");

  for (int i = n - 1; i >= 0; i--) {
    uint32_t xi = (x >> i) & 1u;
    uint32_t yi = (y >> i) & 1u;
    uint32_t row = 2u * xi + yi; /* 0..3, in the table's order */

    uint32_t appended = hil_lam_shapiro_step(&x, &y, i);
    s = 4u * s + appended;

    printf(
      "  i=%d  (xi,yi)=(%u,%u)  row %u: %-27s  append %c%c  s so far = ",
      i, xi, yi, row + 1, ROW_ACTION[row],
      '0' + (char)(appended >> 1), '0' + (char)(appended & 1u)
    );
    print_bits(s, 2 * (n - i));
    printf("\n");

    if (i > 0) {
      uint32_t mask = (1u << i) - 1u;
      printf("          bits still to read:  x = ");
      print_bits(x & mask, i);
      printf("  y = ");
      print_bits(y & mask, i);
      printf("\n");
    }
  }
  printf("  s = %u\n", s);
  return s;
}

/*
  Prints the index s of every cell of a 2^n by 2^n grid, with y increasing
  upward as in the curve's own coordinates. Meant for small n (0..8).
*/
void hil_print_grid(int n) {
  if (n < 0 || n > 8) return;

  uint32_t side = 1u << n;
  uint32_t last = side * side - 1u;
  int width = 1;
  for (uint32_t v = last; v >= 10u; v /= 10u) { width++; }

  for (uint32_t row = 0; row < side; row++) {
    uint32_t y = side - 1u - row;
    printf("  y=%-3u |", y);
    for (uint32_t x = 0; x < side; x++) {
      printf(" %*u", width, hil_s_from_xy(x, y, n));
    }
    printf("\n");
  }
  printf("  x       ");
  for (uint32_t x = 0; x < side; x++) {
    printf(" %*u", width, x);
  }
  printf("\n");
}

/*
  Table 16-6, read as geometry. At each level the next two bits (xi, yi) say
  which quarter of the current square the point is in, and the quarter's
  place along the curve (0..3) is appended to s as two bits:

                   xi = 0        xi = 1
               +-----------+-----------+
      yi = 1   |  upper    |  upper    |
               |  left     |  right    |
               |  01       |  10       |
               +-----------+-----------+
      yi = 0   |  lower    |  lower    |
               |  left     |  right    |
               |  00       |  11       |
               +-----------+-----------+

  The curve visits the quarters in the order 00, 01, 10, 11: the U shape
  lower-left, upper-left, upper-right, lower-right. That is the order-1 curve.

  The curve inside each quarter is oriented differently from the whole. The
  two middle quarters are the same way up, so no change. The first quarter is
  the curve transposed, so swap x and y. The last is the curve reflected
  across the other diagonal, so swap and complement. (In the recursion of
  Figure 16-3 these are exactly the first and last quarters, the two drawn
  with -rot.) The swap or swap-and-complement re-expresses the remaining bits
  in the sub-curve's own frame, so the next level can use the same table.
*/

/* Table 16-6, one row per (xi, yi), indexed by 2*xi + yi, in the book's order. */
typedef struct {
  uint32_t xi, yi;
  int swap, complement;
  uint32_t append;
} table_row;

static const table_row TABLE_16_6[4] = {
  { 0, 0, 1, 0, 0x0 }, /* (0,0): swap x and y              append 00 */
  { 0, 1, 0, 0, 0x1 }, /* (0,1): no change                 append 01 */
  { 1, 0, 1, 1, 0x3 }, /* (1,0): swap and complement       append 11 */
  { 1, 1, 0, 0, 0x2 }, /* (1,1): no change                 append 10 */
};

/* Deterministic pseudo-random words so failures are reproducible. */
static uint32_t xorshift32(uint32_t *state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

/*
  Row 1, (0,0): swap x and y, append 00. i = 2, so the bits still to be read
  are the low two. Bit 2 of both is 0.
*/
Test(lam_shapiro, row_1_swaps_x_and_y_and_appends_00) {
  uint32_t x = 0x2;   /* 0b010 */
  uint32_t y = 0x1;   /* 0b001 */
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 2), 0x0u, "appended bits");
  cr_assert_eq(x, 0x1u, "x is the old y");
  cr_assert_eq(y, 0x2u, "y is the old x");
}

/* Row 2, (0,1): no change, append 01. Bit 2 of x is 0, of y is 1. */
Test(lam_shapiro, row_2_changes_nothing_and_appends_01) {
  uint32_t x = 0x3;   /* 0b011 */
  uint32_t y = 0x6;   /* 0b110 */
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 2), 0x1u, "appended bits");
  cr_assert_eq(x, 0x3u);
  cr_assert_eq(y, 0x6u);
}

/*
  Row 3, (1,0): swap and complement, append 11. Bit 2 of x is 1, of y is 0.
  The new x is the old y complemented and the new y is the old x
  complemented. The whole word is complemented; what matters is the low two
  bits, the ones still to be read.
*/
Test(lam_shapiro, row_3_swaps_and_complements_and_appends_11) {
  uint32_t x = 0x5;   /* 0b101 */
  uint32_t y = 0x2;   /* 0b010 */
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 2), 0x3u, "appended bits");
  cr_assert_eq(x, ~0x2u, "x is the old y, complemented");
  cr_assert_eq(y, ~0x5u, "y is the old x, complemented");
  cr_assert_eq(x & 0x3u, 0x1u, "the two bits still to be read in x: 01");
  cr_assert_eq(y & 0x3u, 0x2u, "the two bits still to be read in y: 10");
}

/* Row 4, (1,1): no change, append 10. Bit 2 of both is 1. */
Test(lam_shapiro, row_4_changes_nothing_and_appends_10) {
  uint32_t x = 0x6;   /* 0b110 */
  uint32_t y = 0x5;   /* 0b101 */
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 2), 0x2u, "appended bits");
  cr_assert_eq(x, 0x6u);
  cr_assert_eq(y, 0x5u);
}

/*
  The table again, exhaustively: every pair of 8-bit values, at every bit
  position 0..7, so every row is hit thousands of times with every possible
  surrounding bits.
*/
Test(lam_shapiro, table_16_6_holds_for_every_input_and_every_bit_position) {
  for (int i = 0; i < 8; i++) {
    for (uint32_t x = 0; x < 256; x++) {
      for (uint32_t y = 0; y < 256; y++) {
        uint32_t xi = (x >> i) & 1u;
        uint32_t yi = (y >> i) & 1u;
        const table_row *row = &TABLE_16_6[2 * xi + yi];

        uint32_t want_x = row->swap ? y : x;
        uint32_t want_y = row->swap ? x : y;
        if (row->complement) {
          want_x = ~want_x;
          want_y = ~want_y;
        }

        uint32_t nx = x, ny = y;
        uint32_t appended = hil_lam_shapiro_step(&nx, &ny, i);
        cr_assert_eq(appended, row->append, "x=%u y=%u i=%d", x, y, i);
        cr_assert_eq(nx, want_x, "x=%u y=%u i=%d", x, y, i);
        cr_assert_eq(ny, want_y, "x=%u y=%u i=%d", x, y, i);
      }
    }
  }
}

/*
  With n = 1 there is only one level, so s is just the "append" column: the
  four rows are the four points of the order-1 curve, in the order the curve
  visits them (compare the order-1 curve from hilbert_curve()).
*/
Test(lam_shapiro, with_n_equal_1_the_table_is_the_order_1_curve) {
  cr_assert_eq(hil_s_from_xy(0, 0, 1), 0u, "lower-left  (0,0)");
  cr_assert_eq(hil_s_from_xy(0, 1, 1), 1u, "upper-left  (0,1)");
  cr_assert_eq(hil_s_from_xy(1, 1, 1), 2u, "upper-right (1,1)");
  cr_assert_eq(hil_s_from_xy(1, 0, 1), 3u, "lower-right (1,0)");

  hilbert_point curve[4];
  cr_assert_eq(hilbert_curve(1, curve, 4), (size_t)4);
  for (uint32_t i = 0; i < 4; i++) {
    cr_assert_eq(hil_s_from_xy(curve[i].x, curve[i].y, 1), i, "point %u of the curve", i);
  }
}

/*
  (1, 0) at n = 2. At the top level (xi, yi) = (0, 0): row 1, swap, append
  00. The low bits were (x0, y0) = (1, 0), but after the swap the next level
  reads (0, 1) and appends 01. Without the swap it would read (1, 0) and
  append 11, which would be the wrong answer. s = 0001 = 1.
*/
Test(lam_shapiro, worked_example_swap_changes_what_the_next_level_reads) {
  uint32_t x = 1, y = 0;
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 1), 0x0u, "top level: row 1, append 00");
  cr_assert_eq(x, 0u);
  cr_assert_eq(y, 1u);
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 0), 0x1u, "next level reads (0,1): row 2, append 01");
  cr_assert_eq(hil_s_from_xy(1, 0, 2), 1u);
}

/*
  (2, 0) at n = 2. At the top level (xi, yi) = (1, 0): row 3, swap and
  complement, append 11. The low bits were (0, 0). After the complement the
  next level reads (1, 1), not (0, 0), and appends 10, not 00. s = 1110 = 14.
*/
Test(lam_shapiro, worked_example_complement_changes_what_the_next_level_reads) {
  uint32_t x = 2, y = 0;
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 1), 0x3u, "top level: row 3, append 11");
  cr_assert_eq(x & 1u, 1u, "next level's xi");
  cr_assert_eq(y & 1u, 1u, "next level's yi");
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 0), 0x2u, "next level reads (1,1): row 4, append 10");
  cr_assert_eq(hil_s_from_xy(2, 0, 2), 14u);
}

/*
  (1, 3) at n = 2. Top level (xi, yi) = (0, 1): row 2, no change, append 01.
  Next level (1, 1): row 4, no change, append 10. s = 0110 = 6.
*/
Test(lam_shapiro, worked_example_with_no_changes) {
  uint32_t x = 1, y = 3;
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 1), 0x1u);
  cr_assert_eq(x, 1u);
  cr_assert_eq(y, 3u);
  cr_assert_eq(hil_lam_shapiro_step(&x, &y, 0), 0x2u);
  cr_assert_eq(hil_s_from_xy(1, 3, 2), 6u);
}

/* The whole order-2 grid, as the book's index column laid out on the plane. */
Test(lam_shapiro, order_2_index_map) {
  static const uint32_t MAP[4][4] = {      /* [y][x], y = 0 first */
    {  0,  1, 14, 15 },
    {  3,  2, 13, 12 },
    {  4,  7,  8, 11 },
    {  5,  6,  9, 10 },
  };
  for (uint32_t y = 0; y < 4; y++) {
    for (uint32_t x = 0; x < 4; x++) {
      cr_assert_eq(hil_s_from_xy(x, y, 2), MAP[y][x], "(x=%u, y=%u)", x, y);
    }
  }
}

/*
  Table 16-6 says an order-n answer is built from an order-(n-1) answer:
  s(x, y, n) is the row's two bits, followed by s of the remaining low bits
  after the row's swap and complement. Stated here with explicit masks and a
  plain recursion, without the xor-with-minus-xi trick the code uses.
*/
Test(lam_shapiro, the_table_is_a_recursion_on_the_low_bits) {
  for (int n = 2; n <= 6; n++) {
    int top = n - 1;
    uint32_t low = (1u << top) - 1u;
    uint32_t side = 1u << n;
    for (uint32_t x = 0; x < side; x++) {
      for (uint32_t y = 0; y < side; y++) {
        uint32_t xi = (x >> top) & 1u;
        uint32_t yi = (y >> top) & 1u;
        const table_row *row = &TABLE_16_6[2 * xi + yi];

        uint32_t xl = x & low;
        uint32_t yl = y & low;
        uint32_t nx = row->swap ? yl : xl;
        uint32_t ny = row->swap ? xl : yl;
        if (row->complement) {
          nx = ~nx & low;
          ny = ~ny & low;
        }

        uint32_t want = (row->append << (2 * top)) | hil_s_from_xy(nx, ny, top);
        cr_assert_eq(hil_s_from_xy(x, y, n), want, "n=%d x=%u y=%u", n, x, y);
      }
    }
  }
}

/*
  hil_s_from_xy() is the inverse of walking the curve. The curve built by
  the recursion of Figure 16-3 (hilbert_curve) is an independent
  implementation, so for every point on it the s computed here must equal its
  index. This is the real test that the table is right, for every order from 0
  to 9 (262144 points).
*/
Test(lam_shapiro, matches_the_index_of_every_point_on_the_recursions_curve) {
  for (int order = 0; order <= 9; order++) {
    size_t capacity = hilbert_point_count(order);
    hilbert_point *curve = malloc(capacity * sizeof *curve);
    cr_assert(curve != NULL);
    cr_assert_eq(hilbert_curve(order, curve, capacity), capacity);
    for (size_t i = 0; i < capacity; i++) {
      cr_assert_eq(hil_s_from_xy(curve[i].x, curve[i].y, order), curve[i].index,
                   "order %d (x=%u, y=%u)", order, curve[i].x, curve[i].y);
    }
    free(curve);
  }
}

/*
  Full width: at order 16, s uses all 32 bits. Take random s, convert to
  (x, y) with the oracle, and confirm hil_s_from_xy() gets s back.
*/
Test(lam_shapiro, round_trips_at_order_16_using_all_32_bits_of_s) {
  uint32_t seed = 0x13579BDFu;
  for (int n = 0; n < 100000; n++) {
    uint32_t s = xorshift32(&seed);
    uint32_t x, y;
    iterative_xy(16, s, &x, &y);
    cr_assert_eq(hil_s_from_xy(x, y, 16), s, "s=0x%08x (x=%u, y=%u)", s, x, y);
  }
}

Test(lam_shapiro, domain_boundaries) {
  cr_assert_eq(hil_s_from_xy(5, 9, 0), 0u, "n = 0 has no levels, so s = 0");
  cr_assert_eq(hil_s_from_xy(0, 0, 16), 0u, "start of an order-16 curve");
  cr_assert_eq(hil_s_from_xy(0xFFFFu, 0, 16), 0xFFFFFFFFu, "end of an order-16 curve: 4^16 - 1");
}

/*
  Only the low n bits of x and y are read, so callers need not mask. The
  complement flips all 32 bits, but the bits it disturbs above position n are
  never looked at again.
*/
Test(lam_shapiro, bits_at_position_n_and_above_are_ignored) {
  uint32_t seed = 0x2468ACE1u;
  for (int n = 1; n <= 8; n++) {
    uint32_t side = 1u << n;
    for (uint32_t x = 0; x < side; x++) {
      for (uint32_t y = 0; y < side; y++) {
        uint32_t gx = xorshift32(&seed) << n;
        uint32_t gy = xorshift32(&seed) << n;
        cr_assert_eq(hil_s_from_xy(x | gx, y | gy, n), hil_s_from_xy(x, y, n),
                     "n=%d x=%u y=%u garbage 0x%08x 0x%08x", n, x, y, gx, gy);
      }
    }
  }
}

/*
  No new assertions use to see the table being applied level by level
  and the index of every cell of the order-2 grid.
*/
Test(lam_shapiro, trace_and_grid) {
  printf("\n--Start hil_s_from_xy() for (x=1, y=0), order 2--\n");
  
  printf("\nIndex s of each cell, order 2:\n");
  hil_print_grid(2);

  printf("\nRow 1 at work (swap): (x=1, y=0)\n");
  cr_assert_eq(hil_s_from_xy_print_trace(1, 0, 2), hil_s_from_xy(1, 0, 2));

  printf("\nRow 3 at work (swap and complement): (x=2, y=0)\n");
  cr_assert_eq(hil_s_from_xy_print_trace(2, 0, 2), hil_s_from_xy(2, 0, 2));

  printf("\nRows 2 and 4 (no change): (x=1, y=3)\n");
  cr_assert_eq(hil_s_from_xy_print_trace(1, 3, 2), hil_s_from_xy(1, 3, 2));

  printf("\nA bigger one, n = 4: (x=11, y=6)\n");
  cr_assert_eq(hil_s_from_xy_print_trace(11, 6, 4), hil_s_from_xy(11, 6, 4));
  
  printf("\n\n--End hil_s_from_xy() for (x=1, y=0), order 2--\n");
}
/*
Index s of each cell, order 2:
  y=3   |  5  6  9 10
  y=2   |  4  7  8 11
  y=1   |  3  2 13 12
  y=0   |  0  1 14 15
  x         0  1  2  3

Row 1 at work (swap): (x=1, y=0)
hil_s_from_xy(x=1, y=0, n=2)
  start:  x = 01  y = 00
  i=1  (xi,yi)=(0,0)  row 1: swap x and y                 append 00  s so far = 00
          bits still to read:  x = 0  y = 1
  i=0  (xi,yi)=(0,1)  row 2: no change                    append 01  s so far = 0001
  s = 1

Row 3 at work (swap and complement): (x=2, y=0)
hil_s_from_xy(x=2, y=0, n=2)
  start:  x = 10  y = 00
  i=1  (xi,yi)=(1,0)  row 3: swap and complement x and y  append 11  s so far = 11
          bits still to read:  x = 1  y = 1
  i=0  (xi,yi)=(1,1)  row 4: no change                    append 10  s so far = 1110
  s = 14

Rows 2 and 4 (no change): (x=1, y=3)
hil_s_from_xy(x=1, y=3, n=2)
  start:  x = 01  y = 11
  i=1  (xi,yi)=(0,1)  row 2: no change                    append 01  s so far = 01
          bits still to read:  x = 1  y = 1
  i=0  (xi,yi)=(1,1)  row 4: no change                    append 10  s so far = 0110
  s = 6

A bigger one, n = 4: (x=11, y=6)
hil_s_from_xy(x=11, y=6, n=4)
  start:  x = 1011  y = 0110
  i=3  (xi,yi)=(1,0)  row 3: swap and complement x and y  append 11  s so far = 11
          bits still to read:  x = 001  y = 100
  i=2  (xi,yi)=(0,1)  row 2: no change                    append 01  s so far = 1101
          bits still to read:  x = 01  y = 00
  i=1  (xi,yi)=(0,0)  row 1: swap x and y                 append 00  s so far = 110100
          bits still to read:  x = 0  y = 1
  i=0  (xi,yi)=(0,1)  row 2: no change                    append 01  s so far = 11010001
  s = 209
*/
