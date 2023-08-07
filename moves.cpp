#include <string>
#include "gamestate.h"
#include "moves.h"

std::string position_to_string(const int x, const int y) {
  char file = 'a' + (char)x - 1;
  char nr = (char)y + '0';

  return std::string({file, nr});
}

std::pair<int, int> string_to_position(std::string position) {
  char file = position[0] - 'a' + 1;
  char nr = position[1] - '0';

  return {file, nr};
}

MoveImpl::~MoveImpl() = default;
MoveImpl::MoveImpl() = default;

bool MoveImpl::is_castle() {
  return false;
}

Piece MoveImpl::get_drop() {
  return Piece::PAWN;
}

MovePiece::MovePiece(int x, int y, int x2, int y2) : x(x), y(y), x2(x2), y2(y2) {}

Move* MovePiece::to_engine() {
  return Move::moveTo(position_to_string(x, y), position_to_string(x2, y2));
}

void MovePiece::exec_move(GameState &state) {
  memento = state.board[x2][y2];
  first_move = state.board[x][y]->get_first_move();
  en_passant = false;
  if (memento != nullptr) {
    if (memento->is_promoted) {
      state.bag[state.color].push_back(new Pawn(state.color));
    } else {
      memento->color = state.color;
      state.bag[state.color].push_back(memento);
    }
  } else if (x != x2 && state.board[x][y]->get_type() == Piece::PAWN) {
    // en passant
    en_passant = true;
    if (y2 == 6) {
      memento = state.board[x2][5];
      state.board[x2][5] = nullptr;
    } else {
      memento = state.board[x2][4];
      state.board[x2][4] = nullptr;
    }
    memento->color = state.color;
    state.bag[state.color].push_back(memento);
  }

  state.board[x][y]->move_piece(state, x, y, x2, y2);
}

void MovePiece::undo_move(GameState &state) {
  if (memento != nullptr) {
    if (memento != state.bag[state.color].back()) {
      delete (state.bag[state.color].back());
    } else {
      memento->color = reverse_color(memento->color);
    }
    state.bag[state.color].pop_back();
  }

  state.board[x][y] = state.board[x2][y2];
  state.board[x][y]->set_first_move(first_move);

  if (en_passant) {
    state.board[x2][y2] = nullptr;
    if (y2 == 6)
      state.board[x2][5] = memento;
    else
      state.board[x2][4] = memento;
  } else {
    state.board[x2][y2] = memento;
  }
}

std::pair<int,int> MovePiece::get_start() {
  return {x, y};
}

std::pair<int,int> MovePiece::get_end() {
  return {x2, y2};
}

uint32_t MovePiece::get_hash(const GameState &state) {
  return (((x - 1) * 8 + y - 1) << 2) | (state.board[x][y]->get_type() << 8);
}

MoveCastle::MoveCastle(int x, int y) : x(x), y(y) {}

void MoveCastle::exec_move(GameState &state) {
  if (x == 7) {
    // e - g -> rocada mica
    state.board[5][y]->move_piece(state, 5, y, 7, y);
    state.board[8][y]->move_piece(state, 8, y, 6, y);
  } else {
    // e - a -> rocada mare
    state.board[5][y]->move_piece(state, 5, y, 3, y);
    state.board[1][y]->move_piece(state, 1, y, 4, y);
  }
}

void MoveCastle::undo_move(GameState &state) {
  if (x == 7) {
    // e - g -> rocada mica
    state.board[7][y]->move_piece(state, 7, y, 5, y);
    state.board[6][y]->move_piece(state, 6, y, 8, y);
    state.board[5][y]->set_first_move(true);
    state.board[8][y]->set_first_move(true);
  } else {
    // e - a -> rocada mare
    state.board[3][y]->move_piece(state, 3, y, 5, y);
    state.board[4][y]->move_piece(state, 4, y, 1, y);
    state.board[5][y]->set_first_move(true);
    state.board[1][y]->set_first_move(true);
  }
}

Move* MoveCastle::to_engine() {
  return Move::moveTo(position_to_string(5, y), position_to_string(x, y));
}

std::pair<int,int> MoveCastle::get_start() {
  return {5, y};
}

std::pair<int,int> MoveCastle::get_end() {
  return {x, y};
}

bool MoveCastle::is_castle() {
  return true;
}

uint32_t MoveCastle::get_hash(const GameState &state) {
  return 1 | (((x - 1) * 8 + y - 1) << 2);
}

MovePromotion::MovePromotion(int x, int y, int x2, int y2, Piece p) : MovePiece(x, y, x2, y2), p(p) {}

Move* MovePromotion::to_engine() {
  return Move::promote(position_to_string(x, y), position_to_string(x2, y2), p);
}

void MovePromotion::exec_move(GameState &state) {
  memento_prom = state.board[x][y];
  state.board[x][y] = PromotedPieceFactory(p, state.color);
  MovePiece::exec_move(state);
}

void MovePromotion::undo_move(GameState &state) {
  MovePiece::undo_move(state);
  delete state.board[x][y];
  state.board[x][y] = memento_prom;
}

uint32_t MovePromotion::get_hash(const GameState &state) {
  return 2 | (((x - 1) * 8 + y - 1) << 2) | (p << 8);
}

MoveDropIn::MoveDropIn(Piece p, int x, int y) : MoveImpl(), p(p), x(x), y(y) {}

Move* MoveDropIn::to_engine() {
  return Move::dropIn(position_to_string(x, y), p);
}

void MoveDropIn::exec_move(GameState &state) {
  for (size_t i = 0; ; ++i) {
    if (state.bag[state.color][i]->get_type() == p) {
      memento = i;
      break;
    }
  }

  state.board[x][y] = state.bag[state.color][memento];
  state.board[x][y]->set_first_move(false);
  state.bag[state.color].erase(state.bag[state.color].begin() + memento);

  if (p == Piece::PAWN) {
    if ((y == 2 && state.color == PlaySide::WHITE) || (y == 7 && state.color == PlaySide::BLACK))
      state.board[x][y]->set_first_move(true);
  }
}

void MoveDropIn::undo_move(GameState &state) {
  state.bag[state.color].insert(state.bag[state.color].begin() + memento, state.board[x][y]);
  state.board[x][y] = nullptr;
}

std::pair<int,int> MoveDropIn::get_start() {
  return {-1, -1};
}

std::pair<int,int> MoveDropIn::get_end() {
  return {x, y};
}

uint32_t MoveDropIn::get_hash(const GameState &state) {
  return 3 | (((x - 1) * 8 + y - 1) << 2) | (p << 8);
}

Piece MoveDropIn::get_drop() {
  return p;
}

MoveImpl* generate_normal(GameState &state, Move *move) {
  int x, y, x2, y2;
  MoveImpl* tmp;

  std::tie(x, y) = string_to_position(move->getSource().value());
  std::tie(x2, y2) = string_to_position(move->getDestination().value());
  if (x == 5 && (y == 1 || y == 8) && (x2 == 3 || x2 == 7) 
      && state.board[x][y] != nullptr && state.board[x][y]->get_type() == Piece::KING)     
    tmp = new MoveCastle(x2, y2);
  else
    tmp = new MovePiece(x, y, x2, y2);
  return tmp;
}

MoveImpl* generate_promotion(GameState &state, Move *move) {
  int x, y, x2, y2;
  std::tie(x, y) = string_to_position(move->getSource().value());
  std::tie(x2, y2) = string_to_position(move->getDestination().value());

  Piece p = move->getReplacement().value();
  return new MovePromotion(x, y, x2, y2, p);
}

MoveImpl* generate_drop(GameState &state, Move *move) {
  int x, y;
  std::tie(x, y) = string_to_position(move->getDestination().value());

  Piece p = move->getReplacement().value();
  return new MoveDropIn(p, x, y);
}

std::string serializeMove(Move* move) {
  if (move->isNormal())
    return move->getSource().value() + move->getDestination().value();
  else if (move->isPromotion()) {
    std::string pieceCode = "";
    switch (move->getReplacement().value()) {
      case Piece::BISHOP:
        pieceCode = "b";
        break;
      case Piece::KNIGHT:
        pieceCode = "n";
        break;
      case Piece::ROOK:
        pieceCode = "r";
        break;
      case Piece::QUEEN:
        pieceCode = "q";
        break;
      default:
        break;
    }
    return move->getSource().value() + move->getDestination().value() +
           pieceCode;
  } else if (move->isDropIn()) {
    std::string pieceCode = "";
    switch (move->getReplacement().value()) {
      case Piece::BISHOP:
        pieceCode = "B";
        break;
      case Piece::KNIGHT:
        pieceCode = "N";
        break;
      case Piece::ROOK:
        pieceCode = "R";
        break;
      case Piece::QUEEN:
        pieceCode = "Q";
        break;
      case Piece::PAWN:
        pieceCode = "P";
        break;
      default:
        break;
    };
    return pieceCode + "@" + move->getDestination().value();
  }

  return "resign";
}
