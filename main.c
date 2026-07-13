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
#define BRICK_HEIGHT  25

typedef enum game_mode {
	MENU,
	START,
	PLAY,
	COMPLETED,
	OVER,
} game_mode;

typedef struct game {
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

game g = {
	.mode = START
};


paddle player = {
	.position.x = ((float) SCREEN_WIDTH / 2) - PADDLE_WIDTH,
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


int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");
	InitAudioDevice();

	SetTargetFPS(FPS);

    Sound collision_sound = LoadSound("assets/collision.mp3");

	int level_bricks_count = 1;
	brick bricks[level_bricks_count];

	for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
	{
		bricks[i].position.x = 1 + i * 200;
		bricks[i].position.y = 100;
		bricks[i].width      = BRICK_WIDTH;
		bricks[i].height     = BRICK_HEIGHT;
		bricks[i].color      = PURPLE;
		bricks[i].lives      = 1;
		bricks[i].is_dead    = false;
	}

	// Main game loop
    while (!WindowShouldClose())
    {
		if (player.lives == 0)
		{
			g.mode = OVER;
			render_game_over();
		}
		else if (level_bricks_count == 0)
		{
			g.mode = COMPLETED;
			render_level_completed(1);
		}


		// Update
		//----------------------------------------------------------------------------------
		float dt = GetFrameTime();
		process_input(dt);

		if (g.mode == START)
		{
			render_game_start();
		}

		if (g.mode == START || g.mode == PLAY)
		{
			update_player();
			update_ball(dt);

			if (check_collision(b.position, b.width, b.height, player.position, player.width, player.height))
			{
				b.direction.y *= -1;
			}

			for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
			{
				if (bricks[i].is_dead) continue;

				if (check_collision(b.position, b.width, b.height, bricks[i].position, bricks[i].width, bricks[i].height))
				{
					b.direction.y *= -1;
					bricks[i].lives--;

					if (bricks[i].lives == 0)
					{
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

		if (g.mode == MENU)
		{
			render_menu();
		}
		else if (g.mode == START)
		{

			render_player();
			render_ball();
			render_game_start();
			render_screen_center();
		}
		else if (g.mode == PLAY)
		{
			// Render Bricks
			for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
			{
				if (bricks[i].is_dead) continue;

				DrawRectangle(bricks[i].position.x, bricks[i].position.y, bricks[i].width, bricks[i].height, bricks[i].color);
			}

			render_player();
			render_ball();

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


void process_input(float dt)
{
	if (IsKeyDown(KEY_LEFT))
	{
		player.position.x -= PADDLE_SPEED * dt;
	}
	else if (IsKeyDown(KEY_RIGHT))
	{
		player.position.x += PADDLE_SPEED * dt;
	}
	else if (IsKeyPressed(KEY_SPACE))
	{
		g.mode = PLAY;
	}
	else if (IsKeyPressed(KEY_P))
	{
		if (g.mode == MENU)
		{
			g.mode = PLAY;
		}
		else if (g.mode == PLAY)
		{
			g.mode = MENU;
		}
	}
}

bool check_collision(Vector2 ball, int ball_width, int ball_height, Vector2 rec, int rec_width, int rec_height)
{
	return ball.x + ball_width > rec.x   &&
		   ball.x < rec.x + rec_width    &&
		   ball.y + ball_height >= rec.y &&
		   ball.y < rec.y + rec_height;
}

void update_player()
{
	if (player.position.x >= SCREEN_WIDTH - PADDLE_WIDTH)
	{
		player.position.x = SCREEN_WIDTH - PADDLE_WIDTH;
	}
	else if (player.position.x <= 0)
	{
		player.position.x = 0;
	}
}

void update_ball(float dt)
{
	if (g.mode == START)
	{
		b.position.x = player.position.x + (player.width / 2);
		b.position.y = player.position.y - player.height;
	}
	else
	{
		b.position.x += b.direction.x * BALL_SPEED * dt;
		b.position.y += b.direction.y * BALL_SPEED * dt;

		if (b.position.x >= SCREEN_WIDTH - BALL_WIDTH)
		{
			b.direction.x = -1;
		}
		else if (b.position.x <= 0)
		{
			b.direction.x = 1;
		}
		if (b.position.y >= SCREEN_HEIGHT + 10)
		{
			player.lives--;
			b.position.x = SCREEN_WIDTH / 2;
			b.position.y = SCREEN_HEIGHT / 2;
		}
		else if (b.position.y <= 0)
		{
			b.direction.y = 1;
		}
	}
}

void render_menu()
{
	DrawText("Pause", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 32, PURPLE);
}

void render_game_start()
{
	Font font             = GetFontDefault();
	const char* text      = "Press SPACE to start";
	int font_size         = 32;
	Vector2 text_position = MeasureTextEx(font, text, font_size, 0);    // Measure string size for Font

	DrawText(text, SCREEN_WIDTH / 2 - text_position.x, SCREEN_HEIGHT / 2 - text_position.y, font_size, WHITE);
}

void render_game_over()
{
	Font font             = GetFontDefault();
	const char* text      = "GAME OVER";
	int font_size         = 64;
	Vector2 text_position = MeasureTextEx(font, text, font_size, 0);    // Measure string size for Font

	DrawText(text, SCREEN_WIDTH / 2 - text_position.x, SCREEN_HEIGHT / 2 - text_position.y, font_size, RED);
}

void render_level_completed(int level_number)
{
	Font font             = GetFontDefault();
	const char* text      = TextFormat("Level Completed %i", level_number);
	int font_size         = 64;
	Vector2 text_position = MeasureTextEx(font, text, font_size, 0);    // Measure string size for Font

	DrawText(text, (SCREEN_WIDTH / 2) - text_position.x, SCREEN_HEIGHT / 2 - text_position.y, font_size, GREEN);
}

void render_screen_center()
{
	int start_x = 0;
	int start_y = SCREEN_HEIGHT / 2;
	int end_x   = SCREEN_WIDTH;
	int end_y   = SCREEN_HEIGHT / 2;

	DrawLine(start_x, start_y, end_x, end_y, PURPLE);
}


void render_player()
{
	// int space = 50;
	// for (int i = 1; i <= player.lives; ++i)
	// {
	// 	DrawRectangle(i * space, 50, 25, 25, RED);
	// }

  	DrawRectangle(player.position.x, player.position.y, player.width, player.height, BLUE);
}

void render_ball()
{
  DrawRectangle(b.position.x, b.position.y, b.width, b.height, b.color);
}

// void render_bricks()
// {
// 	for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
// 	{
// 		DrawRectangle(bricks[i].position.x, bricks[i].position.y, bricks[i].width, bricks[i].height, bricks[i].color);
// 	}
// }
//
