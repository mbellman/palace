#include "move_queue.h"

using namespace Gamma;

void commitMove(MoveQueue& moves, MoveDirection move) {
  using namespace Gamma;

  if (moves.size > 0 && move != moves.queue[0]) {
    // Reset the queue and start with the incoming move
    // if it represents a deviation from the current
    // queued movement pattern
    moves.size = 1;
    moves.queue[0] = move;

    return;
  }

  if (moves.size >= MAX_MOVE_QUEUE_SIZE) {
    // Allow incoming moves to overwrite the last queued
    // move if we're at queue capacity
    moves.queue[moves.size - 1] = move;

    return;
  }

  moves.queue[moves.size++] = move;
}

MoveDirection checkNextMove(MoveQueue& moves, u8 movesAhead) {
  if (moves.size <= movesAhead) {
    return MoveDirection::NONE;
  }

  return moves.queue[movesAhead];
}

MoveDirection takeNextMove(MoveQueue& moves) {
  if (moves.size == 0) {
    return MoveDirection::NONE;
  }

  auto move = moves.queue[0];

  for (u8 i = 0; i < moves.size; i++) {
    moves.queue[i] = moves.queue[i + 1];
  }

  moves.size--;

  return move;
}