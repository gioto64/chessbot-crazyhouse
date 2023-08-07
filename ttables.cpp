#include "ttables.h"

constexpr int table_size = (1 << 20);
constexpr int entry_limit = 128;
constexpr int hash_codes = 1 + 8 * 8 * 6 * 2 + 6 * 2;

std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());

std::vector<table_info> table[table_size];

int64_t hash_table[hash_codes + 1];

void init_hash() {
  for (int i = 0; i <= hash_codes; ++i)
    hash_table[i] = rng();
}

int64_t calc_hash(const GameState &state) {
  int64_t hash = 0;
  if (state.color == PlaySide::BLACK) {
    hash = hash_table[0];
  }

  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      if (state.board[i + 1][j + 1] == nullptr)
        continue;
      int type = state.board[i + 1][j + 1]->get_type();
      int pos;
      if (state.board[i + 1][j + 1]->get_color() == PlaySide::WHITE)
        pos = 1 + (i * 8 + j) * 6 + type;
      else
        pos = 1 + (8 * 8 * 6) + (i * 8 + j) * 6 + type;
      hash ^= hash_table[pos];
    }
  }

  for (auto it : state.bag[PlaySide::WHITE]) {
    int type = it->get_type();
    int pos = 1 + 8 * 8 * 6 * 2 + type;
    hash ^= hash_table[pos];
  }

  for (auto it : state.bag[PlaySide::BLACK]) {
    int type = it->get_type();
    int pos = 1 + 8 * 8 * 6 * 2 + 6 + type;
    hash ^= hash_table[pos];
  }

  return hash;
}

void add_entry(int64_t hash, int depth, int max_depth, int score, int best, int flag) {
  int rem = hash & (table_size - 1);
  for (auto &it : table[rem]) {
    if (it.hash == hash) {
      if (it.max_depth == max_depth && (depth <= it.depth || it.flag == FLAG_EXACT))
        return;
      it = {hash, depth, max_depth, score, best, flag};
      return;
    }
  }

  if ((int)table[rem].size() < entry_limit) {
    table[rem].push_back({hash, depth, max_depth, score, best});
  } else {
    for (auto it = table[rem].begin(); it != table[rem].end(); ++it) {
      if (it->max_depth != max_depth) {
        std::swap(*it, table[rem].back());
        table[rem].back() = {hash, depth, max_depth, score, best, flag};
        return;
      }
    }

    for (auto it = table[rem].begin(); it != table[rem].end(); ++it) {
      if (it->depth > depth) {
        std::swap(*it, table[rem].back());
        table[rem].back() = {hash, depth, max_depth, score, best, flag};
        return;
      }
    }
  }
}

void clear_entries() {
  for (int i = 0; i < table_size; ++i)
    table[i].clear();
}

const table_info& get_entry(int64_t hash) {
  int rem = hash & (table_size - 1);
  for (auto &it : table[rem])
    if (it.hash == hash)
      return it;

  return null_info; 
}
