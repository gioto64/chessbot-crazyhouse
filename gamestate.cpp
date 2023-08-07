#include "strategy.h"
#include "gamestate.h"
#include "pieces.h"
#include "moves.h"
#include <random>

constexpr int knight_dx[] = {-2, -2, -1, -1, 1, 1, 2, 2};
constexpr int knight_dy[] = {-1, 1, -2, 2, -2, 2, -1, 1};

constexpr int king_dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
constexpr int king_dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

int en_passant_opportunity[3];

GameState::GameState() {
  for (int i = 1; i <= BOARD_SIZE; ++i)
    for (int j = 1; j <= BOARD_SIZE; ++j)
      board[i][j] = nullptr;

  for (int i = 1; i <= BOARD_SIZE; ++i) {
    board[i][2] = new Pawn(PlaySide::WHITE);
    board[i][7] = new Pawn(PlaySide::BLACK);
  }

  auto color = PlaySide::WHITE;
  for (int i = 1; i <= BOARD_SIZE; i += 7) {
    board[1][i] = new Rook(color);
    board[2][i] = new Knight(color);
    board[3][i] = new Bishop(color);
    board[4][i] = new Queen(color);
    board[5][i] = new King(color);
    board[6][i] = new Bishop(color);
    board[7][i] = new Knight(color);
    board[8][i] = new Rook(color);
    color = PlaySide::BLACK;
  }
}

std::pair<int, int> GameState::king_pos(PlaySide king_color) {
  for (int i = 1; i <= BOARD_SIZE; ++i)
    for (int j = 1; j <= BOARD_SIZE; ++j)
      if (board[i][j] != nullptr && board[i][j]->get_type() == Piece::KING && board[i][j]->get_color() == king_color)
        return {i, j};
  // wont get here
  return {-1, -1};
}

bool GameState::square_check(int x, int y) {
  // check if a queen / rook checks the square
  for (int dist = 1; dist + x <= 8; ++dist) {
    if (board[dist + x][y] != nullptr) {
      if (board[dist + x][y]->get_color() != color &&
        (board[dist + x][y]->get_type() == Piece::ROOK || board[dist + x][y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; -dist + x >= 1; ++dist) {
    if (board[-dist + x][y] != nullptr) {
      if (board[-dist + x][y]->get_color() != color &&
        (board[-dist + x][y]->get_type() == Piece::ROOK || board[-dist + x][y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; dist + y <= 8; ++dist) {
    if (board[x][dist + y] != nullptr) {
      if (board[x][dist + y]->get_color() != color && 
        (board[x][dist + y]->get_type() == Piece::ROOK || board[x][dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; -dist + y >= 1; ++dist) {
    if (board[x][-dist + y] != nullptr) {
      if (board[x][-dist + y]->get_color() != color && 
        (board[x][-dist + y]->get_type() == Piece::ROOK || board[x][-dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  // check if a bishop / queen checks the square
  for (int dist = 1; dist + x <= 8 && dist + y <= 8; ++dist) {
    if (board[dist + x][dist + y] != nullptr) {
      if (board[dist + x][dist + y]->get_color() != color && 
        (board[dist + x][dist + y]->get_type() == Piece::BISHOP || board[dist + x][dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; -dist + x >= 1 && dist + y <= 8; ++dist) {
    if (board[-dist + x][dist + y] != nullptr) {
      if (board[-dist + x][dist + y]->get_color() != color && 
        (board[-dist + x][dist + y]->get_type() == Piece::BISHOP || board[-dist + x][dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; dist + x <= 8 && -dist + y >= 1; ++dist) {
    if (board[dist + x][-dist + y] != nullptr) {
      if (board[dist + x][-dist + y]->get_color() != color &&
        (board[dist + x][-dist + y]->get_type() == Piece::BISHOP || board[dist + x][-dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }

  for (int dist = 1; -dist + x >= 1 && -dist + y >= 1; ++dist) {
    if (board[-dist + x][-dist + y] != nullptr) {
      if (board[-dist + x][-dist + y]->get_color() != color &&
        (board[-dist + x][-dist + y]->get_type() == Piece::BISHOP || board[-dist + x][-dist + y]->get_type() == Piece::QUEEN))
        return true;
      break;
    }
  }
 
  // check if a knight checks the square
  for (int i = 0; i < 8; ++i) {
    if (knight_dx[i] + x >= 1 && knight_dx[i] + x <= 8 && knight_dy[i] + y >= 1 && knight_dy[i] + y <= 8
        && board[knight_dx[i] + x][knight_dy[i] + y] != nullptr 
        && board[knight_dx[i] + x][knight_dy[i] + y]->get_color() != color 
        && board[knight_dx[i] + x][knight_dy[i] + y]->get_type() == Piece::KNIGHT)
      return true;
  }

  // check if the enemy king checks the square
  for (int i = 0; i < 8; ++i) {
    if (x + king_dx[i] >= 1 && x + king_dx[i] <= BOARD_SIZE && y + king_dy[i] >= 1 && y + king_dy[i] <= BOARD_SIZE 
        && board[x + king_dx[i]][y + king_dy[i]] != nullptr 
        && board[x + king_dx[i]][y + king_dy[i]]->get_color() != color
        && board[x + king_dx[i]][y + king_dy[i]]->get_type() == Piece::KING)
      return true;
  }

  // check if a pawn checks the square
  int pawn_dy = (color == PlaySide::WHITE) ? 1 : -1; 
  if (x + 1 <= 8 && y + pawn_dy >= 1 && y + pawn_dy <= 8
      && board[x + 1][y + pawn_dy] != nullptr
      && board[x + 1][y + pawn_dy]->get_color() != color && board[x + 1][y + pawn_dy]->get_type() == Piece::PAWN)
    return true;

  if (x - 1 >= 1 && y + pawn_dy >= 1 && y + pawn_dy <= 8
      && board[x - 1][y + pawn_dy] != nullptr
      && board[x - 1][y + pawn_dy]->get_color() != color && board[x - 1][y + pawn_dy]->get_type() == Piece::PAWN)
    return true;

  return false;
}

bool GameState::king_check(int x, int y) {
  return square_check(x, y);
}

bool same_vision(std::pair<int, int> p, std::pair<int, int> p2) {
  return (p.first == p2.first) 
    || (p.second == p2.second) 
    || (p.first + p.second == p2.first + p2.second) 
    || (p.first - p.second == p2.first - p2.second);
}

bool horse_vision(std::pair<int, int> p, std::pair<int, int> p2) {
  return (abs(p.first - p2.first) == 2 && abs(p.second - p2.second) == 1)
    || (abs(p.first - p2.first) == 1 && abs(p.second - p2.second) == 2);
}

bool GameState::check_move(std::pair<int, int> king_position, bool king_checked, MoveImpl* move) {
  if (!king_checked) {
    auto start = move->get_start();
    if (start.first < 0)
      return true;
    if (!same_vision(king_position, start))
      return true;
  } else {
    auto end = move->get_end();
    if (!same_vision(king_position, end) && !horse_vision(king_position, end))
      return false;
  }

  auto end = move->get_end();
  move->exec_move(*this);
  if (board[end.first][end.second] != nullptr && board[end.first][end.second]->get_type() == Piece::KING
      && board[end.first][end.second]->get_color() == color)
    king_position = end;
  bool ret = king_check(king_position.first, king_position.second);
  move->undo_move(*this);
  return !ret;
}

void GameState::print_board() {
  std::cerr << std::endl;
  for (int j = 1; j <= BOARD_SIZE; ++j) {
    for (int i = BOARD_SIZE; i >= 1; --i) {
      if (board[i][j] == nullptr)
        std::cerr << '_';
      else {
        char ch;
        if (board[i][j]->get_type() == Piece::PAWN)
          ch = 'p';
        else if (board[i][j]->get_type() == Piece::QUEEN)
          ch = 'q';
        else if (board[i][j]->get_type() == Piece::KNIGHT)
          ch = 'n';
        else if (board[i][j]->get_type() == Piece::BISHOP)
          ch = 'b';
        else if (board[i][j]->get_type() == Piece::ROOK)
          ch = 'r';
        else
          ch = 'k';
        if (board[i][j]->get_color() == PlaySide::BLACK)
          ch = ch - 'a' + 'A';
        std::cerr << ch;
      }

    }
    std::cerr << std::endl;
  }
}

std::vector<MoveImpl*> GameState::get_moves() {
  std::vector<MoveImpl*> moves;

  std::pair<int, int> king_position = king_pos(color);

  for (int i = 1; i <= BOARD_SIZE; ++i) {
    bool found = false;
    for (int j = 1; j <= BOARD_SIZE; ++j) {
      if (board[i][j] != nullptr && board[i][j]->get_type() == Piece::KING && board[i][j]->get_color() == color) {
        king_position = {i, j};
        found = true;
        break;
      }
    }
    if (found)
      break;
  }

  bool king_checked = king_check(king_position.first, king_position.second);
 // print_board();
  for (int i = 1; i <= BOARD_SIZE; ++i) {
    for (int j = 1; j <= BOARD_SIZE; ++j) {
      if (board[i][j] != nullptr && board[i][j]->get_color() == color) {
        auto piece_vision = board[i][j]->get_vision(*this, i, j);

        for (const auto &it : piece_vision) {
          MoveImpl* move;
          if ((it.second == 1 || it.second == 8) && board[i][j]->get_type() == Piece::PAWN) {
            /**
            move = new MovePromotion(i, j, it.first, it.second, Piece::KNIGHT);
            if (check_move(king_position, king_checked, move))
              moves.push_back(move);
            else
              delete move;
            **/
            move = new MovePromotion(i, j, it.first, it.second, Piece::QUEEN);
            if (check_move(king_position, king_checked, move))
              moves.push_back(move);
            else
              delete move;
            /**
            move = new MovePromotion(i, j, it.first, it.second, Piece::ROOK);
            if (check_move(king_position, king_checked, move))
              moves.push_back(move);
            else
              delete move;
            move = new MovePromotion(i, j, it.first, it.second, Piece::BISHOP);
            if (check_move(king_position, king_checked, move))
              moves.push_back(move);
            else
              delete move;
            **/
          } else {
            move = new MovePiece(i, j, it.first, it.second);
            if (check_move(king_position, king_checked, move))
              moves.push_back(move);
            else
              delete move;
          }
        }
      }
    }
  }

  // en passant
  /**
  int passant_x = en_passant_opportunity[color];
  if (passant_x && 1 == 0) {
    int y = (color == PlaySide::WHITE) ? 5 : 4;
    int dy = (color == PlaySide::WHITE) ? 1 : -1;
    if (passant_x - 1 >= 1 && board[passant_x - 1][y] != nullptr 
        && board[passant_x - 1][y]->get_type() == Piece::PAWN
        && board[passant_x - 1][y]->get_color() == color) {
      moves.push_back(new MovePiece(passant_x - 1, y, passant_x, y + dy));
    }

    if (passant_x + 1 <= 8 && board[passant_x + 1][y] != nullptr 
        && board[passant_x + 1][y]->get_type() == Piece::PAWN
        && board[passant_x + 1][y]->get_color() == color) {
      moves.push_back(new MovePiece(passant_x + 1, y, passant_x, y + dy));
    }
  }
  **/

  // castle
  int y = (color == PlaySide::WHITE) ? 1 : 8;

  if (board[5][y] != nullptr && board[5][y]->get_type() == Piece::KING && board[5][y]->get_first_move() == true
      && !square_check(5, y)) {
    if (board[6][y] == nullptr && board[7][y] == nullptr && board[8][y] != nullptr && board[8][y]->get_type() == Piece::ROOK && board[8][y]->get_first_move() == true) {
      if (!square_check(6, y) && !square_check(7, y)) {
        MoveImpl* move = new MoveCastle(7, y);
        moves.push_back(move);
      }
    }

    /**
    if (board[2][y] == nullptr && board[3][y] == nullptr && board[4][y] == nullptr && board[1][y] != nullptr && board[1][y]->get_type() == Piece::ROOK && board[1][y]->get_first_move() == true) {
      MoveImpl* move = new MoveCastle(3, y);
      if (!square_check(4, y) && !square_check(3, y)) {
        for (auto it : moves)
          delete it;
        moves.clear();
        moves.push_back(move);
        return moves;
      } else
        delete move;
    }
    **/
  }

  // drop in
  bool f[6];
  for (int i = 0; i < 6; ++i)
    f[i] = 0;
  for (auto it : bag[color]) {
    Piece type = it->get_type();
    if (f[type])
      continue;

    f[type] = 1;
    int mn = 1 + (type == Piece::PAWN);
    int mx = 8 - (type == Piece::PAWN);

    for (int i = 1; i <= 8; ++i) {
      for (int j = mn; j <= mx; ++j) {
        if (board[i][j] == nullptr) {
          MoveImpl* move = new MoveDropIn(type, i, j);
          if (check_move(king_position, king_checked, move))
            moves.push_back(move);
          else
            delete move;
        }
      }
    }
  }

  return moves;
}

void GameState::check_en_passant(Move *move) {
  int x, y, x2, y2;
  std::tie(x, y) = string_to_position(move->getSource().value());
  if (board[x][y]->get_type() != Piece::PAWN)
    return;

  std::tie(x2, y2) = string_to_position(move->getDestination().value());
  if (y - y2 == 2 || y - y2 == -2)
    en_passant_opportunity[reverse_color(color)] = x;
}

void GameState::record_move(Move* move, PlaySide color) {
  this->color = color;
  en_passant_opportunity[reverse_color(color)] = 0;
  MoveImpl *tmp = nullptr;
  if (move->isNormal()) {
    tmp = generate_normal(*this, move);
    check_en_passant(move);
  } else if (move->isPromotion()) {
    tmp = generate_promotion(*this, move);
  } else if (move->isDropIn()) {
    tmp = generate_drop(*this, move);
  }
  tmp->exec_move(*this);
}

Move* GameState::do_move(PlaySide color) {
  this->color = color;

  MoveImpl *move = find_move(*this);
  move->exec_move(*this);
  Move* ret = move->to_engine(); 

  delete move;
  return ret;
}
