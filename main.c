#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define FPS 60
#define SCREEN_WIDTH  2000
#define SCREEN_HEIGHT 1000
#define PADDLE_WIDTH  200
#define PADDLE_HEIGHT 25
#define PADDLE_SPEED  1500
#define PLAYER_LIVES  3
#define BALL_WIDTH    25
#define BALL_HEIGHT   25
#define BALL_SPEED    1000
#define BRICK_WIDTH   150
#define BRICK_HEIGHT  50
#define GRID_COLS     10
#define GRID_ROWS     5
#define MAX_LEVELS    2

typedef enum game_mode {
  MENU,
  START,
  PLAY,
  COMPLETED,
  OVER,
} game_mode;

typedef struct game {
  int level_index;
  game_mode mode;
} game;

typedef struct paddle {
  int width;
  int height;
  Vector2 position;
  Vector2 direction;
  int lives;
} paddle;

typedef struct ball {
  int width;
  int height;
  Vector2 position;
  Vector2 direction;
  Color color;
} ball;

typedef struct brick {
  int width;
  int height;
  Vector2 position;
  Color color;
  int lives;
  bool is_dead;
} brick;

void process_input(float dt);
void init_level();
void render_player(void);
void render_ball(void);
void render_menu(void);
void render_bricks(void);
void render_game_over(void);
void render_game_start(void);
void render_level_completed(int);
void render_screen_center(void);
void update_ball(float dt);
void update_player(void);
bool check_collision(Vector2, int, int, Vector2, int, int);
void reset_positions(void);

//
// Init Global Variables
//

game g = {
	.mode = START
};

paddle player = {
    .position.x = ((float)SCREEN_WIDTH / 2) - (float)PADDLE_WIDTH / 2,
    .position.y = SCREEN_HEIGHT - PADDLE_HEIGHT,
    .width      = PADDLE_WIDTH,
    .height     = PADDLE_HEIGHT,
    .lives      = PLAYER_LIVES,
};

ball b = {
    .width       = BALL_WIDTH,
    .height      = BALL_HEIGHT,
    .direction.x = 0.5f,
    .direction.y = 1,
    .color       = GREEN,
};

int level_bricks_count;

brick bricks[GRID_ROWS*GRID_COLS];

char level_1[GRID_ROWS][GRID_COLS] = {
	"bbbbbbbbbb",
	"..........",
	"..........",
	"..........",
	"..........",
};


int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");
  InitAudioDevice();
  SetTargetFPS(FPS);

  Sound collision_sound = LoadSound("assets/collision.mp3");

  init_level();

  // Main game loop
  while (!WindowShouldClose()) {
    // Update
    //----------------------------------------------------------------------------------
    float dt = GetFrameTime();
    process_input(dt);

    if (player.lives == 0) {
      g.mode = OVER;
    }

    if (level_bricks_count == 0) {
      g.mode = COMPLETED;
    }

    if (g.mode == START) {
      update_player();
      update_ball(dt);
    } else if (g.mode == PLAY) {
      update_player();
      update_ball(dt);

      if (check_collision(b.position, b.width, b.height, player.position,
                          player.width, player.height)) {
        // NOTE: Calculate ball direction depending on where it hit the paddle
        b.direction.y *= -1;
      }

      for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i) {
        if (bricks[i].is_dead)
          continue;

        // NOTE: Calculate ball direction depending on where it hit the brick
        // Also there is a big if you hit the brick on the side
        if (check_collision(b.position, b.width, b.height, bricks[i].position,
                            bricks[i].width, bricks[i].height)) {
          b.direction.y *= -1;
          bricks[i].lives--;
          if (bricks[i].lives == 0) {
            bricks[i].is_dead = true;
            level_bricks_count--;
          }
        }
      }
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);
	DrawText(TextFormat("Bricks in level: %i", level_bricks_count), 100, SCREEN_HEIGHT - 200, 30, GREEN);

	//
	// Render Bricks Grid
	//
	switch (g.mode) {
		case MENU:
			render_menu();
			break;
		case OVER:
			render_game_over();
			break;
		case START:
			render_player();
			render_ball();
			break;
		case PLAY:
			render_player();
			render_ball();
			render_bricks();
			break;
		default:
			break;
	}

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadSound(collision_sound);
  CloseAudioDevice();
  CloseWindow();
  //--------------------------------------------------------------------------------------

  return 0;
}

void process_input(float dt) {
  if (IsKeyDown(KEY_LEFT)) {
    player.position.x -= PADDLE_SPEED * dt;
  } else if (IsKeyDown(KEY_RIGHT)) {
    player.position.x += PADDLE_SPEED * dt;
  } else if (IsKeyPressed(KEY_SPACE)) {
    g.mode = PLAY;
  } else if (IsKeyPressed(KEY_P)) {
    if (g.mode == MENU) {
      g.mode = PLAY;
    } else if (g.mode == PLAY) {
      g.mode = MENU;
    }
  }
}

void init_level()
{
	for (int row = 0; row < GRID_ROWS; ++row)
	{
		for (int col = 0; col < GRID_COLS; ++col)
		{
			if (level_1[row][col] == 'b')
			{
				int padding = 50;
				int gap = 5;

				brick new_brick;
				new_brick.width      = BRICK_WIDTH;
				new_brick.height     = BRICK_HEIGHT;
				new_brick.position.x = col * (BRICK_WIDTH  + gap) + padding;
				new_brick.position.y = row * (BRICK_HEIGHT + gap) + padding;
				new_brick.lives      = 1;
				new_brick.color      = RED;
				new_brick.is_dead    = false;

				bricks[level_bricks_count] = new_brick;
				level_bricks_count++;
			}
		}
	}
}

void update_player() {
  if (player.position.x >= SCREEN_WIDTH - PADDLE_WIDTH) {
    player.position.x = SCREEN_WIDTH - PADDLE_WIDTH;
  } else if (player.position.x <= 0) {
    player.position.x = 0;
  }
}

void update_ball(float dt) {
  if (g.mode == START) {
    b.position.x =
        player.position.x + ((float)player.width / 2) - (float)b.width / 2;
    b.position.y = player.position.y - player.height;
  } else {
    b.position.x += b.direction.x * BALL_SPEED * dt;
    b.position.y += b.direction.y * BALL_SPEED * dt;

    if (b.position.x >= SCREEN_WIDTH - BALL_WIDTH) {
      b.direction.x = -1;
    } else if (b.position.x <= 0) {
      b.direction.x = 1;
    }
    // adding a little bit of space for ball to drop
    if (b.position.y >= SCREEN_HEIGHT + 10) {
      player.lives--;
      reset_positions();
      g.mode = START;
    } else if (b.position.y <= 0) {
      b.direction.y = 1;
    }
  }
}

void reset_positions() {
  b.position.x =
      player.position.x + ((float)player.width / 2) - (float)b.width / 2;
  b.position.y = player.position.y - player.height;
  b.direction.y = -1;

  player.position.x = ((float)SCREEN_WIDTH / 2) - (float)PADDLE_WIDTH / 2;
  player.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
}

bool check_collision(Vector2 ball, int ball_width, int ball_height, Vector2 rec,
                     int rec_width, int rec_height) {
  return ball.x + ball_width > rec.x   &&
	     ball.x < rec.x + rec_width    &&
         ball.y + ball_height > rec.y  &&
		 ball.y < rec.y + rec_height;
}

void render_menu() {
  DrawText("Pause", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 32, PURPLE);
}

void render_game_start() {
  const char *text = "Press SPACE to start";
  int font_size = 64;
  int text_width = MeasureText(text, font_size);
  Font font = GetFontDefault();
  Vector2 text_dimensions = MeasureTextEx(font, text, font_size, 0);

  DrawText(TextFormat("Text dimensions: x = %i y = %i", text_dimensions.x,
                      text_dimensions.y),
           200, 200, 16, BLUE);

  DrawText(text, SCREEN_WIDTH / 2 - text_width / 2, SCREEN_HEIGHT / 2,
           font_size, WHITE);
}

void render_game_over() {
  const char *text = "GAME OVER";
  int font_size = 64;
  int text_width = MeasureText(text, font_size);
  Font font = GetFontDefault();
  Vector2 text_dimensions = MeasureTextEx(font, text, font_size, 0);

  DrawText(text, SCREEN_WIDTH / 2 - text_width, 100, font_size, RED);
}

void render_level_completed(int level_number) {
  const char *text = TextFormat("Level Completed %i", level_number);
  int font_size = 64;
  int text_width = MeasureText(text, font_size);
  DrawText(text, text_width / 2, SCREEN_HEIGHT / 2, font_size, GREEN);
}

void render_screen_center() {
  DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, PURPLE);
  DrawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, PURPLE);
}

void render_player() {
  DrawRectangle(player.position.x, player.position.y, player.width,
                player.height, BLUE);
}

void render_ball() {
  DrawRectangle(b.position.x, b.position.y, b.width, b.height, b.color);
}

void render_bricks() {
  for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i) {
    if (bricks[i].is_dead)
      continue;

    DrawRectangle(bricks[i].position.x, bricks[i].position.y, bricks[i].width,
                  bricks[i].height, bricks[i].color);
  }
}
