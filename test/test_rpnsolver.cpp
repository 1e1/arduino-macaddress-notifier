#include "RpnSolver.h"

#include <cstdio>

/* tiny assertion harness -- no external dependency */
static int g_total = 0;
static int g_failures = 0;

#define EXPECT_EQ(actual, expected) do {                                 \
    ++g_total;                                                           \
    const long _a = (long) (actual);                                     \
    const long _e = (long) (expected);                                   \
    if (_a != _e) {                                                      \
      ++g_failures;                                                      \
      std::printf("  FAIL  %s:%d  (%s) => %ld, expected %ld\n",          \
                  __FILE__, __LINE__, #actual, _a, _e);                  \
    }                                                                    \
  } while (0)

/* '@id' reference resolver: returns the id itself, easy to assert against */
static int mapId(int id) { return id; }

int main() {
  RpnSolver solver;
  solver.addMapper(mapId);

  std::printf("RpnSolver host tests\n");

  /* arithmetic (readme grammar table) */
  EXPECT_EQ(solver.resolve("1 2 +"), 3);
  EXPECT_EQ(solver.resolve("10 4 -"), 6);
  EXPECT_EQ(solver.resolve("5 2 *"), 10);
  EXPECT_EQ(solver.resolve("12 3 /"), 4);

  /* comparisons -> boolean */
  EXPECT_EQ(solver.resolve("2 1 <"), 0);
  EXPECT_EQ(solver.resolve("2 2 <"), 0);
  EXPECT_EQ(solver.resolve("1 2 <"), 1);
  EXPECT_EQ(solver.resolve("9 9 ="), 1);
  EXPECT_EQ(solver.resolve("9 8 ="), 0);
  EXPECT_EQ(solver.resolve("7 1 >"), 1);

  /* if-then-else : "then else if ?" */
  EXPECT_EQ(solver.resolve("7 3 1 ?"), 7);
  EXPECT_EQ(solver.resolve("8 4 0 ?"), 4);

  /* not */
  EXPECT_EQ(solver.resolve("0 !"), 1);
  EXPECT_EQ(solver.resolve("1 !"), 0);

  /* multi-digit literals */
  EXPECT_EQ(solver.resolve("10 20 +"), 30);

  /* '@id' references resolve through the mapper */
  EXPECT_EQ(solver.resolve("@3"), 3);
  EXPECT_EQ(solver.resolve("@3 @4 +"), 7);

  /* readme examples */
  EXPECT_EQ(solver.resolve("@0 @1 + @2 + @3 +"), 6);   /* 0+1+2+3 */
  EXPECT_EQ(solver.resolve("@0 @1 * @2 * @3 *"), 0);   /* 0*1*2*3 */

  /* division-by-zero guard (regression for the divide fix) */
  EXPECT_EQ(solver.resolve("12 0 /"), 0);
  EXPECT_EQ(solver.resolve("5 5 - 9 /"), 0);           /* 0/9 = 0 */

  /* check(): well-formed equations */
  EXPECT_EQ(solver.check("1 2 +"), 1);
  EXPECT_EQ(solver.check("@0"), 1);
  EXPECT_EQ(solver.check("7 3 1 ?"), 1);

  /* check(): malformed equations */
  EXPECT_EQ(solver.check("1 2"), 0);   /* two values left on the stack */
  EXPECT_EQ(solver.check("1 +"), 0);   /* operator underflows the stack */
  EXPECT_EQ(solver.check("@"), 0);     /* '@' with no id */

  std::printf("%d/%d checks passed\n", g_total - g_failures, g_total);
  if (g_failures) {
    std::printf("FAILED (%d)\n", g_failures);
    return 1;
  }
  std::printf("OK\n");
  return 0;
}
