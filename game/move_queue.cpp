#include "move_queue.h"

void addMove(MoveQueue& moves, MoveDirection move) {
  using namespace Gamma;

  if (moves.size >= MAX_MOVE_QUEUE_SIZE) {
    return;
  }

  moves.queue[moves.size++] = move;
}

MoveDirection checkNextMove(MoveQueue& moves, Gamma::uint8 moveCount) {
  if (moves.size <= moveCount) {
    return MoveDirection::NONE;
  }

  return moves.queue[moveCount];
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