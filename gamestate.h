#ifndef CHESSBOT_GAMESTATE_HPP
#define CHESSBOT_GAMESTATE_HPP
#include <algorithm>
#include <utility>
#include "pieces.h"
#include "PlaySide.h"
#include "Move.h"
#include "moves.h"

constexpr int BOARD_SIZE = 8;

inline PlaySide reverse_color(const PlaySide color) {
  return (color == PlaySide::BLACK) ? PlaySide::WHITE : PlaySide::BLACK;
}

class GameState {
public:
  GameState();
  std::vector<MoveImpl*> get_moves();
  Move* do_move(PlaySide color);
  void record_move(Move* move, PlaySide color);
  bool square_check(int i, int j);
  bool king_check(int x, int y);
  std::pair<int, int> king_pos(PlaySide king_color);
  bool check_move(std::pair<int, int> king_position, bool king_checked, MoveImpl* move);
  void print_board();
  void check_en_passant(Move *move);

  PlaySide color;
  PieceImpl* board[BOARD_SIZE + 1][BOARD_SIZE + 1];
  std::vector<PieceImpl*> bag[2];
};

#endif // CHESSBOT_GAMESTATE_HPP
