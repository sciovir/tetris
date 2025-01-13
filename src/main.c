#include "raylib.h"
#include <stdio.h>

#define SQUARE_SIZE 20
#define PIECE_SIZE 4

#define GRID_HORIZONTAL_SIZE 12
#define GRID_VERTICAL_SIZE 20

typedef enum GridSquare { EMPTY, MOVING, FULL, BLOCK, FADING } GridSquare;

static const int screen_width = 800;
static const int screen_height = 450;

static bool game_over = false;
static bool pause = false;
static bool begin = true;

static bool collision = false;
static bool active_piece = false;

static int level = 1;
static int lines = 0;

static int speed = 30;
static int lateral_speed = 10;
static int turn_speed = 12;
static int movement_tick = 0;
static int lateral_movement_tick = 0;
static int turn_movement_tick = 0;

static GridSquare grid[GRID_HORIZONTAL_SIZE][GRID_VERTICAL_SIZE];
static GridSquare piece[PIECE_SIZE][PIECE_SIZE];
static GridSquare incoming_piece[PIECE_SIZE][PIECE_SIZE];

static int piece_position_x = 0;
static int piece_position_y = 0;

static void ResetIncomingPiece() {
  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      incoming_piece[i][j] = EMPTY;
    }
  }
}

static void GetRandomPiece() {
  int rand = GetRandomValue(0, 6);
  ResetIncomingPiece();

  switch (rand) {
  case 0: // Cube
    incoming_piece[1][1] = MOVING;
    incoming_piece[1][2] = MOVING;
    incoming_piece[2][1] = MOVING;
    incoming_piece[2][2] = MOVING;
    break;
  case 1: // L
    incoming_piece[1][0] = MOVING;
    incoming_piece[1][1] = MOVING;
    incoming_piece[1][2] = MOVING;
    incoming_piece[2][2] = MOVING;
    break;
  case 2: // L inversa
    incoming_piece[2][0] = MOVING;
    incoming_piece[2][1] = MOVING;
    incoming_piece[2][2] = MOVING;
    incoming_piece[1][2] = MOVING;
    break;
  case 3: // Recta
    incoming_piece[0][1] = MOVING;
    incoming_piece[1][1] = MOVING;
    incoming_piece[2][1] = MOVING;
    incoming_piece[3][1] = MOVING;
    break;
  case 4: // Creu tallada
    incoming_piece[1][0] = MOVING;
    incoming_piece[1][1] = MOVING;
    incoming_piece[1][2] = MOVING;
    incoming_piece[2][1] = MOVING;
    break;
  case 5: // S
    incoming_piece[1][1] = MOVING;
    incoming_piece[2][1] = MOVING;
    incoming_piece[2][2] = MOVING;
    incoming_piece[3][2] = MOVING;
    break;
  case 6: // S inversa
    incoming_piece[1][2] = MOVING;
    incoming_piece[2][2] = MOVING;
    incoming_piece[2][1] = MOVING;
    incoming_piece[3][1] = MOVING;
    break;
  }
}

static bool SpawnPiece() {
  piece_position_x = (int)((GRID_HORIZONTAL_SIZE - PIECE_SIZE) / 2);
  piece_position_y = 0;

  if (begin) {
    GetRandomPiece();
    begin = false;
  }

  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      piece[i][j] = incoming_piece[i][j];
    }
  }

  GetRandomPiece();

  for (int i = piece_position_x; i < piece_position_x + PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      if (piece[i - (int)piece_position_x][j] == MOVING) {
        grid[i][j] = MOVING;
      }
    }
  }

  return true;
}

static bool IsCollided() {
  for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
    for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
      if (grid[i][j] == MOVING &&
          (grid[i][j + 1] == FULL || grid[i][j + 1] == BLOCK)) {
        return true;
      }
    }
  }

  return false;
}

static void Falling() {
  if (collision) {
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
      for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
        if (grid[i][j] == MOVING) {
          grid[i][j] = FULL;
          collision = false;
          active_piece = false;
        }
      }
    }
  } else {
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
      for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
        if (grid[i][j] == MOVING) {
          grid[i][j + 1] = MOVING;
          grid[i][j] = EMPTY;
        }
      }
    }

    piece_position_y++;
  }
}

static bool LateralMove() {
  bool can_move = true;
  if (IsKeyDown(KEY_LEFT)) {
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
      for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
        if (grid[i][j] == MOVING) {
          if ((i - 1 <= 0) || (grid[i - 1][j] == FULL) ||
              (grid[i - 1][j] == BLOCK)) {
            can_move = false;
          }
        }
      }
    }

    if (can_move) {
      for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
        for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
          if (grid[i][j] == MOVING) {
            grid[i - 1][j] = MOVING;
            grid[i][j] = EMPTY;
          }
        }
      }

      piece_position_x--;
    }
  } else if (IsKeyDown(KEY_RIGHT)) {
    can_move = true;
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
      for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
        if (grid[i][j] == MOVING) {
          if ((i + 1 >= GRID_HORIZONTAL_SIZE - 1) || (grid[i + 1][j] == FULL) ||
              (grid[i + 1][j] == BLOCK)) {
            can_move = false;
          }
        }
      }
    }

    if (can_move) {
      for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--) {
        for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++) {
          if (grid[i][j] == MOVING) {
            grid[i + 1][j] = MOVING;
            grid[i][j] = EMPTY;
          }
        }
      }

      piece_position_x++;
    }
  }

  printf("%d\n", can_move);
  return can_move;
}

static void InitGame() {
  pause = false;

  for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++) {
    for (int j = 0; j < GRID_VERTICAL_SIZE; j++) {
      if ((i == 0) || (i == GRID_HORIZONTAL_SIZE - 1) ||
          (j == GRID_VERTICAL_SIZE - 1)) {
        grid[i][j] = BLOCK;
      } else {
        grid[i][j] = EMPTY;
      }
    }
  }

  ResetIncomingPiece();
}

static void UpdateGame() {
  if (!game_over) {
    if (IsKeyPressed(KEY_P)) {
      pause = !pause;
    }

    if (pause) {
      return;
    }

    if (!active_piece) {
      active_piece = SpawnPiece();
    } else {
      movement_tick++;
      lateral_movement_tick++;
      turn_movement_tick++;

      if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        lateral_movement_tick = lateral_speed;
      }

      if (movement_tick >= speed) {
        collision = IsCollided();
        Falling();
        movement_tick = 0;
      }

      if (lateral_movement_tick >= lateral_speed) {
        LateralMove();
        lateral_movement_tick = 0;
      }
    }

    for (int j = 1; j < GRID_HORIZONTAL_SIZE - 1; j++) {
      if (grid[0][j] == FULL) {
        game_over = true;
        break;
      }
    }
  } else if (IsKeyPressed(KEY_ENTER)) {
    InitGame();
    game_over = false;
  }
}

static void DrawGame() {
  BeginDrawing();

  ClearBackground(RAYWHITE);
  if (!game_over) {
    Vector2 offset;
    offset.y = (screen_height - (GRID_VERTICAL_SIZE * SQUARE_SIZE)) / 2;

    for (int j = 0; j < GRID_VERTICAL_SIZE; j++) {
      offset.x = screen_width / 2 - (GRID_HORIZONTAL_SIZE * SQUARE_SIZE / 2) -
                 ((PIECE_SIZE + 1) * SQUARE_SIZE);

      for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++) {
        switch (grid[i][j]) {
        case EMPTY:
          DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y,
                   LIGHTGRAY);
          DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE,
                   LIGHTGRAY);
          DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE,
                   offset.y + SQUARE_SIZE, LIGHTGRAY);
          DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE,
                   offset.y + SQUARE_SIZE, LIGHTGRAY);
          offset.x += SQUARE_SIZE;
          break;
        case MOVING:
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, DARKGRAY);
          offset.x += SQUARE_SIZE;
          break;
        case FULL:
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);
          offset.x += SQUARE_SIZE;
          break;
        case BLOCK:
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE,
                        LIGHTGRAY);
          offset.x += SQUARE_SIZE;
          break;
        default:
          break;
        }
      }

      offset.y += SQUARE_SIZE;
    }

    offset.x = screen_width / 2 + SQUARE_SIZE * 2;
    offset.y =
        (screen_height - (GRID_VERTICAL_SIZE * SQUARE_SIZE)) / 2 + SQUARE_SIZE;

    DrawText("INCOMING:", offset.x, offset.y - SQUARE_SIZE, 10, GRAY);

    for (int j = 0; j < PIECE_SIZE; j++) {
      for (int i = 0; i < PIECE_SIZE; i++) {
        switch (incoming_piece[i][j]) {
        case EMPTY:
          DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y,
                   LIGHTGRAY);
          DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE,
                   LIGHTGRAY);
          DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE,
                   offset.y + SQUARE_SIZE, LIGHTGRAY);
          DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE,
                   offset.y + SQUARE_SIZE, LIGHTGRAY);
          offset.x += SQUARE_SIZE;
          break;
        case MOVING:
          DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE,
                        LIGHTGRAY);
          offset.x += SQUARE_SIZE;
          break;
        default:
          break;
        }
      }

      offset.x = screen_width / 2 + SQUARE_SIZE * 2;
      offset.y += SQUARE_SIZE;
    }

    DrawText(TextFormat("LINES:      %04i", lines), offset.x,
             offset.y + SQUARE_SIZE, 10, GRAY);
  }

  EndDrawing();
}

int main() {
  InitWindow(screen_width, screen_height, "Tetris");

  InitGame();
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    UpdateGame();
    DrawGame();
  }

  CloseWindow();

  return 0;
}
