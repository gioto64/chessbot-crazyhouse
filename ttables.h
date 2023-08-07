#pragma once
#include "gamestate.h"

constexpr int FLAG_EXACT = 1;
constexpr int FLAG_UPPER = 2;

struct table_info {
  int64_t hash; 
  int depth, max_depth, score, best;
  int flag;
};

const table_info null_info = {0, 0, 0, 0, 0, 0};

void init_hash();
int64_t calc_hash(const GameState &state);
void add_entry(int64_t hash, int depth, int max_depth, int score, int best, int flag);
void clear_entries();
const table_info& get_entry(int64_t hash);
