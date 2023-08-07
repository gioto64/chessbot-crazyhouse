#ifndef CHESSBOT_PIECES_HPP
#define CHESSBOT_PIECES_HPP
#include <vector>
#include <memory>
#include "PlaySide.h"
#include "Piece.h"

class GameState;
/**
 * corner cases when moving a piece:
 * 1) pawn en passant
 * 2) pawn first move double advance
 * 3) king check
 */

class PieceImpl {
public:
  virtual ~PieceImpl();
  PieceImpl() = delete;
  PieceImpl(PlaySide color);

  virtual std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) = 0;

  virtual void move_piece(GameState &state, int x, int y, int new_x, int new_y);
  virtual bool is_king() const;
  virtual void set_first_move(bool value);
  virtual bool get_first_move() const;
  virtual Piece get_type() const;

  virtual PlaySide get_color() const;

  bool is_promoted = false;
  PlaySide color;
};

class Pawn : public PieceImpl {
public:
  Pawn(PlaySide color);

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) final;

  void move_piece(GameState &state, int x, int y, int new_x, int new_y) final;
  void set_first_move(bool value) override;
  bool get_first_move() const override;
  Piece get_type() const override;

protected:
  bool first_move;
};

class Knight : public PieceImpl {
public:
  Knight() = delete;
  Knight(PlaySide color);

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) final;
  Piece get_type() const override;
};

// It has to be virtual because queen inherits both rook and bishop
class Bishop : virtual public PieceImpl {
public:
  Bishop() = delete;
  Bishop(PlaySide color);

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) override;
  Piece get_type() const override;
};

// It has to be virtual because queen inherits both rook and bishop
class Rook : virtual public PieceImpl {
public:
  Rook() = delete;
  Rook(PlaySide color);

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) override;

  void move_piece(GameState &state, int x, int y, int new_x, int new_y) final;
  void set_first_move(bool value) override;
  bool get_first_move() const override;
  Piece get_type() const override;

protected:
  bool first_move;
};

class Queen : public Rook, public Bishop {
public:
  Queen(PlaySide color);

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) override;
  Piece get_type() const override;
};

class King : public PieceImpl {
public:
  King(PlaySide color);

  bool is_king() const override;

  std::vector<std::pair<int, int>> get_vision(const GameState &state, int x, int y) override;

  void move_piece(GameState &state, int x, int y, int new_x, int new_y) final;
  void set_first_move(bool value) override;
  bool get_first_move() const override;
  Piece get_type() const override;

protected:
  bool first_move;
};

inline PieceImpl* PromotedPieceFactory(Piece p, PlaySide color) {
  PieceImpl *tmp;
  if (p == Piece::QUEEN) {
    tmp = new Queen(color);
  } else if (p == Piece::KNIGHT) {
    tmp = new Knight(color);
  } else if (p == Piece::BISHOP) {
    tmp = new Bishop(color);
  } else {
    tmp = new Rook(color);
    tmp->set_first_move(false);
  }

  tmp->is_promoted = true;
  return tmp;
}

#endif //CHESSBOT_PIECES_HPP
