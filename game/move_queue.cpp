#include "move_queue.h"

void addMove(MoveQueue& moves, MoveDirection move, float time) {
  if (moves.size >= 5) {
    return;
  }

  moves.queue[moves.size++] = move;
  moves.lastMoveTime = time;
}

MoveDirection takeNextMove(MoveQueue& moves) {
  using namespace Gamma;

  if (moves.size == 0) {
    return MoveDirection::NONE;
  }

  auto move = moves.queue[0];

  for (uint8 i = 0; i < moves.size; i++) {
    moves.queue[i] = moves.queue[i + 1];
  }

  moves.size--;

  return move;
}