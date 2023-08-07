#include "pieces.h"
#include "gamestate.h"

/**
 * corner cases when moving a piece:
 * 1) pawn en passant
 * 2) pawn first move double advance
 * 3) king check
 */
constexpr int knight_dx[] = {-2, -2, -1, -1, 1, 1, 2, 2};
constexpr int knight_dy[] = {-1, 1, -2, 2, -2, 2, -1, 1};

constexpr int king_dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
constexpr int king_dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

PieceImpl::~PieceImpl() = default;
PieceImpl::PieceImpl(PlaySide color) : color(color) {}

void PieceImpl::move_piece(GameState &state, int x, int y, int new_x, int new_y) {
  state.board[x][y] = nullptr;
  state.board[new_x][new_y] = this;
  set_first_move(false);
}

bool PieceImpl::is_king() const {
  return false;
}

PlaySide PieceImpl::get_color() const {
  return color;
}

void PieceImpl::set_first_move(bool value) {}

bool PieceImpl::get_first_move() const {
  return false;
}

Piece PieceImpl::get_type() const {
  return Piece::PAWN;
}

Pawn::Pawn(PlaySide color) : PieceImpl(color), first_move(true) {}

std::vector<std::pair<int, int>> Pawn::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> vision;
  int dir;
  if (color == PlaySide::WHITE) {
    dir = 1;
  } else {
    dir = -1;
  }

  if (state.board[x][y + dir] == nullptr) {
    vision.emplace_back(x, y + dir);
    if (((y == 2 && dir == 1) || (y == 7 && dir == -1)) && state.board[x][y + 2 * dir] == nullptr)
      vision.emplace_back(x, y + 2 * dir);
  }

  if (x + 1 <= BOARD_SIZE && state.board[x + 1][y + dir] != nullptr
    && state.board[x + 1][y + dir]->get_color() != color) {
    vision.emplace_back(x + 1, y + dir);
  }

  if (x - 1 >= 1 && state.board[x - 1][y + dir] != nullptr
      && state.board[x - 1][y + dir]->get_color() != color) {
    vision.emplace_back(x - 1, y + dir);
  }

  return vision;
}

void Pawn::move_piece(GameState &state, int x, int y, int new_x, int new_y) {
  PieceImpl::move_piece(state, x, y, new_x, new_y);
  first_move = false;
}

void Pawn::set_first_move(bool value) {
  first_move = value;
}

bool Pawn::get_first_move() const {
  return first_move;
}

Piece Pawn::get_type() const {
  return Piece::PAWN;
}

Knight::Knight(PlaySide color) : PieceImpl(color) {}

std::vector<std::pair<int, int>> Knight::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> vision;
  for (int i = 0; i < 8; ++i) {
    int dir_x = knight_dx[i];
    int dir_y = knight_dy[i];
    if (1 <= x + dir_x && x + dir_x <= BOARD_SIZE
      && 1 <= y + dir_y && y + dir_y <= BOARD_SIZE
      && (state.board[x + dir_x][y + dir_y] == nullptr
      || state.board[x + dir_x][y + dir_y]->get_color() != color))
      vision.emplace_back(x + dir_x, y + dir_y);

    if (1 <= x + dir_y && x + dir_y <= BOARD_SIZE
      && 1 <= y + dir_x && y + dir_x <= BOARD_SIZE
      && (state.board[x + dir_y][y + dir_x] == nullptr
      || state.board[x + dir_y][y + dir_x]->get_color() != color))
      vision.emplace_back(x + dir_y, y + dir_x);
  }
  return vision;
}

Piece Knight::get_type() const {
  return Piece::KNIGHT;
}

// It has to be virtual because queen inherits both rook and bishop
Bishop::Bishop(PlaySide color) : PieceImpl(color) {}

std::vector<std::pair<int, int>> Bishop::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> vision;
  // up right
  for (int dir = 1; dir + std::max(x, y) <= BOARD_SIZE; dir += 1) {
    if (state.board[x + dir][y + dir] != nullptr && state.board[x + dir][y + dir]->get_color() == color)
      break;
    vision.emplace_back(x + dir, y + dir);
    if (state.board[x + dir][y + dir] != nullptr)
      break;
  }
  // up left
  for (int dir = 1; x - dir >= 1 && y + dir <= BOARD_SIZE; dir += 1) {
    if (state.board[x - dir][y + dir] != nullptr && state.board[x - dir][y + dir]->get_color() == color)
      break;
    vision.emplace_back(x - dir, y + dir);
    if (state.board[x - dir][y + dir] != nullptr)
      break;
  }
  // down right
  for (int dir = 1; x + dir <= BOARD_SIZE && y - dir >= 1; dir += 1) {
    if (state.board[x + dir][y - dir] != nullptr && state.board[x + dir][y - dir]->get_color() == color)
      break;
    vision.emplace_back(x + dir, y - dir);
    if (state.board[x + dir][y - dir] != nullptr)
      break;
  }
  // down left
  for (int dir = 1; std::min(x, y) - dir >= 1; dir += 1) {
    if (state.board[x - dir][y - dir] != nullptr && state.board[x - dir][y - dir]->get_color() == color)
      break;
    vision.emplace_back(x - dir, y - dir);
    if (state.board[x - dir][y - dir] != nullptr)
      break;
  }
  return vision;
}

Piece Bishop::get_type() const {
  return Piece::BISHOP;
}

// It has to be virtual because queen inherits both rook and bishop
Rook::Rook(PlaySide color) : PieceImpl(color), first_move(true) {}

std::vector<std::pair<int, int>> Rook::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> vision;
  // horizontally left
  for (int dir_x = -1; x + dir_x >= 1; dir_x -= 1) {
    if (state.board[x + dir_x][y] != nullptr && state.board[x + dir_x][y]->get_color() == color)
      break;
    vision.emplace_back(x + dir_x, y);
    if (state.board[x + dir_x][y] != nullptr)
      break;
  }
  // horizontally right
  for (int dir_x = 1; x + dir_x <= BOARD_SIZE; dir_x += 1) {
    if (state.board[x + dir_x][y] != nullptr && state.board[x + dir_x][y]->get_color() == color)
      break;
    vision.emplace_back(x + dir_x, y);
    if (state.board[x + dir_x][y] != nullptr)
      break;
  }
  // vertically down
  for (int dir_y = -1; y + dir_y >= 1; dir_y -= 1) {
    if (state.board[x][y + dir_y] != nullptr && state.board[x][y + dir_y]->get_color() == color)
      break;
    vision.emplace_back(x, y + dir_y);
    if (state.board[x][y + dir_y] != nullptr)
      break;
  }
  // vertically up
  for (int dir_y = 1; y + dir_y <= BOARD_SIZE; dir_y += 1) {
    if (state.board[x][y + dir_y] != nullptr && state.board[x][y + dir_y]->get_color() == color)
      break;
    vision.emplace_back(x, y + dir_y);
    if (state.board[x][y + dir_y] != nullptr)
      break;
  }
  return vision;
}

void Rook::set_first_move(bool value) {
  first_move = value;
}

bool Rook::get_first_move() const {
  return first_move;
}

Piece Rook::get_type() const {
  return Piece::ROOK;
}

void Rook::move_piece(GameState &state, int x, int y, int new_x, int new_y) {
  PieceImpl::move_piece(state, x, y, new_x, new_y);
  first_move = false;
}

Queen::Queen(PlaySide color)
      : PieceImpl(color), Rook(color), Bishop(color) {}

std::vector<std::pair<int, int>> Queen::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> queen_vision = Rook::get_vision(state, x, y);
  std::vector<std::pair<int, int>> bishop_vision = Bishop::get_vision(state, x, y);
  queen_vision.insert(queen_vision.end(), bishop_vision.begin(), bishop_vision.end());
  return queen_vision;
}

Piece Queen::get_type() const {
  return Piece::QUEEN;
}

King::King(PlaySide color) : PieceImpl(color), first_move(true) {}

bool King::is_king() const {
  return true;
}

std::vector<std::pair<int, int>> King::get_vision(const GameState &state, int x, int y) {
  std::vector<std::pair<int, int>> vision;
  for (int i = 0; i < 8; ++i) {
    if (x + king_dx[i] >= 1 && x + king_dx[i] <= BOARD_SIZE && y + king_dy[i] >= 1 && y + king_dy[i] <= BOARD_SIZE 
        && (state.board[x + king_dx[i]][y + king_dy[i]] == nullptr 
        || state.board[x + king_dx[i]][y + king_dy[i]]->get_color() != color)) {
      vision.emplace_back(x + king_dx[i], y + king_dy[i]);
    }
  }

  return vision;
}

void King::move_piece(GameState &state, int x, int y, int new_x, int new_y) {
  PieceImpl::move_piece(state, x, y, new_x, new_y);
  first_move = false;
}

void King::set_first_move(bool value) {
  first_move = value;
}

bool King::get_first_move() const {
  return first_move;
}

Piece King::get_type() const {
  return Piece::KING;
}
