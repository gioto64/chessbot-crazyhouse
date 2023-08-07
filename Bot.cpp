#include "Bot.h"

extern PlaySide engineSide;

const std::string Bot::BOT_NAME = "giotobot"; /* Edit this, escaped characters are forbidden */

extern PlaySide getEngineSide(); 
//Bot::Bot() : state() {}
Bot::Bot() : state() {}

void Bot::recordMove(Move* move, PlaySide sideToMove) {
    /* You might find it useful to also separately
     * record last move in another custom field */ 
  state.record_move(move, sideToMove);
}

Move* Bot::calculateNextMove() {
  /* Play move for the side the engine is playing (Hint: Main.getEngineSide())
   * Make sure to record your move in custom structures before returning.
   *
   * Return move that you are willing to submit
   * Move is to be constructed via one of the factory methods declared in Move.h */
  return state.do_move(getEngineSide());
}

std::string Bot::getBotName() { return Bot::BOT_NAME; }
