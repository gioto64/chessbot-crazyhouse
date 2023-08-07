#ifndef CHESSBOT_MOVES_HPP
#define CHESSBOT_MOVES_HPP

#include <memory>
#include "Move.h"
#include "pieces.h"

class GameState;

// rocada e efectiv de unde merge regele pana unde merge
// promotion e efectiv de unde merge pionul pana ajunge si la final o litera in functie de piesa in care se transforma
// dropin -> numele piesei@pozitia

std::pair<int, int> string_to_position(std::string position);
std::string serializeMove(Move* move);

class MoveImpl {
public:
  virtual ~MoveImpl();
  MoveImpl();
  virtual Move* to_engine() = 0;
  virtual void exec_move(GameState &state) = 0;
  virtual void undo_move(GameState &state) = 0;
  virtual std::pair<int,int> get_start() = 0;
  virtual std::pair<int,int> get_end() = 0;
  virtual bool is_castle();
  virtual Piece get_drop();
  virtual uint32_t get_hash(const GameState &state) = 0;
};

class MovePiece : public MoveImpl {
public:
  MovePiece(int x, int y, int x2, int y2);
  Move* to_engine() override;
  void exec_move(GameState &state) override;
  void undo_move(GameState &state) override;
  std::pair<int,int> get_start() override;
  std::pair<int,int> get_end() override;
  uint32_t get_hash(const GameState &state) override;
  int x, y, x2, y2;
  PieceImpl *memento;
  bool first_move, en_passant;
};

class MoveCastle : public MoveImpl {
public:
  MoveCastle(int x, int y);
  Move* to_engine() override;
  void exec_move(GameState &state) override;
  void undo_move(GameState &state) override;
  std::pair<int,int> get_start() override;
  std::pair<int,int> get_end() override;
  bool is_castle() override;
  uint32_t get_hash(const GameState &state) override;
  int x, y;
};

class MovePromotion : public MovePiece {
public:
  MovePromotion(int x, int y, int x2, int y2, Piece p);
  Move* to_engine() override;
  void exec_move(GameState &state) override;
  void undo_move(GameState &state) override;
  uint32_t get_hash(const GameState &state) override;
  PieceImpl *memento_prom;
  Piece p;
};

class MoveDropIn : public MoveImpl {
public:
  MoveDropIn(Piece p, int x, int y);

  Move* to_engine() override;
  void exec_move(GameState &state) override;
  void undo_move(GameState &state) override;
  std::pair<int,int> get_start() override;
  std::pair<int,int> get_end() override;
  Piece get_drop() override;
  uint32_t get_hash(const GameState &state) override;
  Piece p;
  int x, y;
  int memento;
};

MoveImpl* generate_normal(GameState &state, Move *move);
MoveImpl* generate_promotion(GameState &state, Move *move);
MoveImpl* generate_drop(GameState &state, Move *move);

#endif // CHESSBOT_MOVES_HPP
