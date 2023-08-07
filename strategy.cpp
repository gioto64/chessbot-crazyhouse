#include "strategy_constants.h"
#include "strategy.h"
#include "ttables.h"

std::unordered_map<int, int> tt_table[MAX_DEPTH + 1];
auto startTime = std::chrono::high_resolution_clock::now();
bool timeout;

int MAX_TIME = 7000;
int MAX_TIME_FORCED = 600;

inline int score_piece(Piece type) {
  if (type == Piece::PAWN)
    return PAWN_SCORE;
  if (type == Piece::BISHOP)
    return BISHOP_SCORE;
  if (type == Piece::KNIGHT)
    return KNIGHT_SCORE;
  if (type == Piece::ROOK)
    return ROOK_SCORE;

  return QUEEN_SCORE;
}

inline int score_piece_hand(Piece type) {
  if (type == Piece::PAWN)
    return PAWN_HAND_SCORE;
  if (type == Piece::BISHOP)
    return BISHOP_HAND_SCORE;
  if (type == Piece::KNIGHT)
    return KNIGHT_HAND_SCORE;
  if (type == Piece::ROOK)
    return ROOK_HAND_SCORE;

  return QUEEN_HAND_SCORE;
}

inline int score_piece_attack(Piece type) {
  if (type == Piece::PAWN)
    return 6;
  if (type == Piece::BISHOP)
    return 4;
  if (type == Piece::KNIGHT)
    return 4;
  if (type == Piece::ROOK)
    return 3;
  if (type == Piece::QUEEN)
    return 1;
  return 0;
}

inline int score_piece_undefended(Piece type) {
  if (type == Piece::PAWN)
    return PAWN_SCORE / 8;
  if (type == Piece::BISHOP)
    return PAWN_SCORE / 6;
  if (type == Piece::KNIGHT)
    return PAWN_SCORE / 6;
  if (type == Piece::ROOK)
    return PAWN_SCORE / 4;
  if (type == Piece::QUEEN)
    return PAWN_SCORE / 2;
  return 0;
}

inline int score_piece_table(Piece type, int x, int y, PlaySide color) {
  int pos2 = (y + 1) * 12 + x + 1;

  if (color == PlaySide::WHITE)
    y = 9 - y;
  else
    x = 9 - x;

  --x; --y;

  int pos = y * 8 + x;
  if (type == Piece::PAWN) {
    if (color == PlaySide::WHITE)
      return white_pawn[pos2];
    return black_pawn[pos2];
  }

  if (type == Piece::BISHOP)
    return bishop[pos2];

  if (type == Piece::KNIGHT) {
    if (color == PlaySide::WHITE)
      return black_knight[pos2];
    return black_knight[pos2];
  }

  if (type == Piece::ROOK) {
    if (color == PlaySide::WHITE)
      return black_rook[pos2];
    return black_rook[pos2];
  }

  if (type == Piece::QUEEN)
    return queen_table[pos];
  return king_table[pos];
}

void reorder_moves(std::vector<MoveImpl*> &moves, GameState &state) {
   auto get_score = [&](MoveImpl* move) -> int {
    if (move->is_castle()) {
      return PAWN_SCORE;
    }

    auto start = move->get_start();
    auto end = move->get_end();

    if (start.first < 0) {
      return score_piece(move->get_drop()) - 1;
    }

    if (state.board[end.first][end.second] != nullptr) {
      return score_piece(state.board[end.first][end.second]->get_type());
    }

    return 0;
  };

  sort(moves.begin(), moves.end(), [&](auto &&a, auto &&b) {
      return get_score(a) > get_score(b);
      });
  //shuffle(moves.begin(), moves.end(), rng);
}

void reorder_perm(std::vector<MoveImpl*> &moves, std::vector<int> &perm) {
  std::vector<MoveImpl*> copy_moves = moves;
  for (size_t i = 0; i < copy_moves.size(); ++i)
    moves[i] = copy_moves[perm[i]];
}

int eval_state(GameState &state) {
  int score = 0;
  PlaySide rev_color = reverse_color(state.color);

  auto moves_my = state.get_moves();

  if (moves_my.size() == 0) {
    auto king_pos = state.king_pos(state.color);
    if (state.square_check(king_pos.first, king_pos.second))
      return -INF;
    return 0;
  }

  for (auto it : moves_my)
    delete it;

  // calculate piece values + piece table values
  int ap[2][6];
  int attacked[2][9][9];
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 6; ++j)
      ap[i][j] = 0;

  for (int i = 1; i <= 8; ++i)
    for (int j = 1; j <= 8; ++j)
      attacked[0][i][j] = attacked[1][i][j] = false;

  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      if (state.board[i][j] == nullptr)
        continue;
      PlaySide piece_color = state.board[i][j]->get_color();
      Piece type = state.board[i][j]->get_type();
      ++ap[piece_color][type];

      int piece_score = score_piece(type);
      piece_score += score_piece_table(type, i, j, state.board[i][j]->get_color());

      auto vision = state.board[i][j]->get_vision(state, i, j);
      piece_score += vision.size() * MOBILITY;

      int val = score_piece_attack(type);
      for (auto it : vision) {
        attacked[piece_color][it.first][it.second] = std::max(attacked[piece_color][it.first][it.second], val);
      }

      if (piece_color == state.color) {
        score += piece_score;
      } else {
        score -= piece_score;
      }
    }
  }

  // attack squares scores

  // if queen is attacked we lose tempo
  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      if (state.board[i][j] != nullptr && state.board[i][j]->get_type() == Piece::QUEEN) {
        if (state.board[i][j]->get_color() == state.color && attacked[rev_color][i][j])
          score -= PAWN_SCORE / 2;
        else if (state.board[i][j]->get_color() != state.color && attacked[state.color][i][j])
          score += PAWN_SCORE / 2;
      }
    }
  }

  // look for undefended pieces
  for (int i = 1; i <= 8; ++i) {
    for (int j = 1; j <= 8; ++j) {
      if (state.board[i][j] != nullptr) {
        PlaySide color = state.board[i][j]->get_color();
        if (attacked[reverse_color(color)][i][j] && !attacked[color][i][j]) {
          if (color == state.color)
            score -= score_piece_undefended(state.board[i][j]->get_type());
          else
            score += score_piece_undefended(state.board[i][j]->get_type());
        }
      }
    }
  }

  // bonus points for bishop pair
  int nr_pairs = 0;
  if (ap[state.color][Piece::BISHOP] > 1)
    ++nr_pairs;
  if (ap[rev_color][Piece::BISHOP] > 1)
    --nr_pairs;

  score += nr_pairs * BISHOP_PAIR;

  // bonus points if enemy king is in check and minus points if it is undefended
  auto king_pos = state.king_pos(state.color);
  if (state.square_check(king_pos.first, king_pos.second))
    score += KING_CHECK;

  for (int dx = -1; dx <= 1; ++dx) {
    int x = dx + king_pos.first;
    if (x >= 1 && x <= 8) {
      for (int dy = -1; dy <= 1; ++dy) {
        int y = dy + king_pos.second;
        if (y >= 1 && y <= 8) {
          if (attacked[rev_color][x][y] && !attacked[state.color][x][y])
            score -= KING_DEFENSE * 2;
        }
      }
    }
  }

  king_pos = state.king_pos(rev_color);
  if (state.square_check(king_pos.first, king_pos.second))
    score -= KING_CHECK;

  for (int dx = -1; dx <= 1; ++dx) {
    int x = dx + king_pos.first;
    if (x >= 1 && x <= 8) {
      for (int dy = -1; dy <= 1; ++dy) {
        int y = dy + king_pos.second;
        if (y >= 1 && y <= 8) {
          if (attacked[state.color][x][y] && !attacked[rev_color][x][y])
            score += KING_DEFENSE * 2;
        }
      }
    }
  }

  // get value of pieces in hand
  int f[6];
  for (int i = 0; i < 6; ++i)
    f[i] = 0;
  for (auto it : state.bag[state.color]) {
    Piece tmp = it->get_type();
    ++f[tmp];
    score += score_piece_hand(tmp) / f[tmp];
  }
  score -= state.bag[state.color].size() * BIG_HAND_PENALTY;

  for (int i = 0; i < 6; ++i)
    f[i] = 0;
  for (auto it : state.bag[rev_color]) {
    Piece tmp = it->get_type();
    ++f[tmp];
    score -= score_piece_hand(tmp) / f[tmp];
  }
  score += state.bag[rev_color].size() * BIG_HAND_PENALTY;

 

  return score;
}

int negamax_forced(int depth, int alpha, int beta, const int max_depth, GameState &state, const PlaySide first_color) {
  if (timeout)
    return -INF;

  if (depth == 0) {
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

    if (duration.count() >= MAX_TIME_FORCED)
      timeout = true;

    return eval_state(state);
  }

  auto moves = state.get_moves();
  if (moves.size() == 0) {
    auto king_pos = state.king_pos(state.color);
    if (state.square_check(king_pos.first, king_pos.second))
      return -INF;
    return 0;
  }

  if (state.color != first_color) {
    if (moves.size() > MAX_MOVES_FORCED) {
      auto stopTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

      if (duration.count() >= MAX_TIME_FORCED)
        timeout = true;

      for (auto it : moves)
        delete it;

      if (depth <= 5)
        return eval_state(state);
      return INF;
    }
  }

  auto state_hash = calc_hash(state);
  auto ret = get_entry(state_hash);
  bool found = false;

  if (ret.hash != 0 && ret.best < (int)moves.size()) {
    if (depth <= ret.depth && max_depth == ret.max_depth) {
      if (ret.flag == FLAG_EXACT || ret.score >= beta) {
        for (auto it : moves)
          delete it;
        return ret.score;
      }
    }
    std::swap(moves[ret.best], moves[0]);
    found = true;
  }

  std::string curr_moves;
  int score = -INF;

  std::vector<std::pair<int, int>> values;
  int inc = 1, best_pos = 0;
  bool found_move = false;
  for (size_t pos = 0; pos < moves.size(); pos += inc) {
    auto move = moves[pos];
    move->exec_move(state);
    state.color = reverse_color(state.color);

    if (state.color != first_color) {
      auto king_pos = state.king_pos(state.color);
      if (!state.square_check(king_pos.first, king_pos.second)) {
        state.color = reverse_color(state.color);
        move->undo_move(state);
        continue;
      }
    }

    int move_score;
    if (pos == 0 || !found_move) {
      move_score = -negamax_forced(depth - 1, -beta, -alpha, max_depth, state, first_color);
    } else {
      move_score = -negamax_forced(depth - 1, -(alpha + 1), -alpha, max_depth, state, first_color);
      if (move_score > alpha && move_score < beta)
        move_score = -negamax_forced(depth - 1, -beta, -alpha, max_depth, state, first_color);
    }

    found_move = true;

    state.color = reverse_color(state.color);
    move->undo_move(state);

    if (move_score > score) {
      score = move_score;
      best_pos = pos;
      alpha = std::max(alpha, score);
    }

    if (alpha >= beta) {
      break;
    }
  }

  for (auto it : moves)
    delete it;

  if (timeout)
    return -INF;

  if (!found_move && state.color == first_color)
    return -INF;

  if (found) {
    if (best_pos == 0)
      best_pos = ret.best;
    else if (best_pos == ret.best)
      best_pos = 0;
  }

  if (found || score >= beta)
    add_entry(state_hash, depth, max_depth, score, best_pos, FLAG_UPPER);
  else
    add_entry(state_hash, depth, max_depth, score, best_pos, FLAG_EXACT);

  return score;
}

int negamax(int depth, int alpha, int beta, const int max_depth, GameState &state) {
  if (timeout)
    return -INF;

  if (depth == 0) {
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

    if (duration.count() >= MAX_TIME)
      timeout = true;

    return eval_state(state);
    //return tt_table[depth][hash] = eval_state(state);
    //return {eval_state(state), ""};
  }

  auto moves = state.get_moves();
  if (moves.size() == 0) {
    auto king_pos = state.king_pos(state.color);
    if (state.square_check(king_pos.first, king_pos.second))
      return -INF;
      //return {-INF, ""};
    return 0;
    //return {0, ""};
  }

  auto state_hash = calc_hash(state);
  auto ret = get_entry(state_hash);
  bool found = false;

  //if (depth >= max_depth - 2)
  reorder_moves(moves, state);

  if (ret.hash != 0 && ret.best < (int)moves.size()) {
    if (depth <= ret.depth && max_depth == ret.max_depth) {
      if (ret.flag == FLAG_EXACT || ret.score >= beta) {
        for (auto it : moves)
          delete it;
        return ret.score;
      }
    }
    std::swap(moves[ret.best], moves[0]);
    found = true;
  }

  std::string curr_moves;
  int score = -INF;

  std::vector<std::pair<int, int>> values;
  int inc = 1, best_pos = 0;
  for (size_t pos = 0; pos < moves.size(); pos += inc) {
    auto move = moves[pos];
    //auto hash_move = move->get_hash(state);
    move->exec_move(state);
    state.color = reverse_color(state.color);

    int move_score;
    if (!found || pos == 0) {
      move_score = -negamax(depth - 1, -beta, -alpha, max_depth, state);
    } else {
      move_score = -negamax(depth - 1, -(alpha + 1), -alpha, max_depth, state);
      if (move_score > alpha && move_score < beta)
        move_score = -negamax(depth - 1, -beta, -alpha, max_depth, state);
      //int add = std::min(2LL * PAWN_SCORE, std::min((long long)beta - alpha, 500LL));
      //move_score = -negamax(depth - 1, -(alpha + add), -alpha, max_depth, state);
      //if (move_score > alpha && move_score < beta && add != beta - alpha)
      //  move_score = -negamax(depth - 1, -beta, -alpha, max_depth, state);
    }
    //std::string moves_str;
    //auto ret = negamax(depth - 1, -beta, -alpha, hash * hash_move, state);
    //move_score = -ret.first;
    //moves_str = ret.second;

    state.color = reverse_color(state.color);
    move->undo_move(state);

    if (move_score > score) {
      score = move_score;
      best_pos = pos;
      //curr_moves = serializeMove(move->to_engine()) + ", ";
      //curr_moves += moves_str;
      alpha = std::max(alpha, score);
    }

    if (alpha >= beta) {
      break;
    }
  }

  for (auto it : moves)
    delete it;

  if (timeout)
    return -INF;

  if (found) {
    if (best_pos == 0)
      best_pos = ret.best;
    else if (best_pos == ret.best)
      best_pos = 0;
  }

  if (found || score >= beta)
    add_entry(state_hash, depth, max_depth, score, best_pos, FLAG_UPPER);
  else
    add_entry(state_hash, depth, max_depth, score, best_pos, FLAG_EXACT);
  return score;  //return {score, curr_moves};
}

std::pair<int, MoveImpl*> try_force(GameState &state, std::vector<std::pair<MoveImpl*, int>> &moves_scores) {
  timeout = false;
  clear_entries();

  int alpha = -INF, beta = INF;
  bool first = true;

  const PlaySide color = state.color;
  for (auto &move_score : moves_scores) {
    auto move = move_score.first;
    move->exec_move(state);
    state.color = reverse_color(state.color);

    auto king_pos = state.king_pos(state.color);
    if (!state.square_check(king_pos.first, king_pos.second)) {
      state.color = reverse_color(state.color);
      move->undo_move(state);
      continue;
    }

    if (first) {
      move_score.second = -negamax_forced(MAX_DEPTH_FORCED, -beta, -alpha, MAX_DEPTH_FORCED, state, color);
      first = false;
    } else {
      move_score.second = -negamax_forced(MAX_DEPTH_FORCED, -(alpha + 1), -alpha, MAX_DEPTH_FORCED, state, color);
      if (move_score.second > alpha)
        move_score.second = -negamax_forced(MAX_DEPTH_FORCED, -beta, -alpha, MAX_DEPTH_FORCED, state, color);
    }

    state.color = reverse_color(state.color);
    move->undo_move(state);

    if (timeout == true)
      break;

    std::cerr << move_score.second << '\n';
    if (move_score.second >= SCORE_STEP)
      return {INF, move_score.first};
    alpha = std::max(alpha, move_score.second);
  }

  return {0, nullptr};
}

MoveImpl* iterative_deepening(GameState &state) {
  startTime = std::chrono::high_resolution_clock::now();

  auto moves = state.get_moves();
  reorder_moves(moves, state);

  std::vector<std::pair<MoveImpl*, int>> moves_scores;
  for (auto move : moves)
    moves_scores.push_back({move, 0});
  moves.clear();

  /**
  auto force = try_force(state, moves_scores);
  if (force.first >= SCORE_STEP) {
    std::cerr << "FOUND GOOD FORCED POSITION\n" << std::endl << '\n';
    std::cerr << force.first << '\n';
    for (auto move : moves_scores) {
      if (move.first != force.second)
        delete move.first;
    }

    return force.second;
  } else {
    auto stopTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

    std::cerr << "TIME WASTED ON FORCED MOVE: " << duration.count() << "\n";
    for (auto &move_score : moves_scores)
      move_score.second = 0;
  }
  **/
  timeout = false;
  clear_entries();

  for (int depth = 2; depth <= MAX_DEPTH; ++depth) {
    int alpha = -INF, beta = INF;
    int cnt = 0;
    for (auto &move_score : moves_scores) {
      if (move_score.second == -INF)
        continue;

      auto move = move_score.first;
      move->exec_move(state);
      state.color = reverse_color(state.color);
      if (cnt == 0 || depth == 2) {
        move_score.second = -negamax(depth - 1, -beta, -alpha, depth, state);
      } else {
        move_score.second = -negamax(depth - 1, -(alpha + 1), -alpha, depth, state);
        if (move_score.second > alpha)
          move_score.second = -negamax(depth - 1, -beta, -alpha, depth, state);
      }

      state.color = reverse_color(state.color);
      move->undo_move(state);

      if (timeout)
        break;

      if (move_score.second > alpha)
        alpha = move_score.second;
      ++cnt;
    }

    if (timeout) {
      for (int i = 0; i < cnt; ++i) {
        if (moves_scores[i].second == alpha) {
          swap(moves_scores[i], moves_scores[0]);
          break;
        }
      }
      break;
    }

    stable_sort(moves_scores.begin(), moves_scores.end(),
        [&](auto &&x, auto &&y) {
          return x.second > y.second;
        });

  }

  auto best_move = moves_scores[0].first;
  for (size_t i = 1; i < moves_scores.size(); ++i)
    delete moves_scores[i].first;

  return best_move;
}

int n_moves = 0;
MoveImpl* find_move(GameState &state) {
  n_moves += 1;
  if (n_moves == 60)
    MAX_TIME = 1900;

  init_hash();
  return iterative_deepening(state);
}
