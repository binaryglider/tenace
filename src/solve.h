#include "bridge.h"

void init_solve();
void solve_statusbar(char *text);
//void board_dds(board *b);
void compute_dd_scores (board *b, int compute);
void invalidate_dd_scores (board *b);

void parscore(board *b);
char *par_label (board *b);

extern int run_dd;
