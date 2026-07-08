#include "raylib.h"

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

typedef enum game_mode {
	MENU,
	START,
	PLAY,
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
} brick;

void process_input(float dt);
void render_player(void);
void render_ball(void);
void render_menu(void);
void render_game_over(void);
void update_ball(float dt);
void update_player(void);
void check_collision(void);

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
	.color       = WHITE
};


int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");
	InitAudioDevice();

	SetTargetFPS(FPS);

    Sound collision_sound = LoadSound("assets/collision.mp3");

	brick bricks[10];

	for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
	{
		bricks[i].position.x = 1 + i * 100;
		bricks[i].position.y = 100;
		bricks[i].width      = 50;
		bricks[i].height     = 50;
		bricks[i].color      = PURPLE;
	}

	// Main game loop
    while (!WindowShouldClose())
    {
		if (player.lives == 0)
		{
			g.mode = OVER;
			render_game_over();
		}
		// Update
		//----------------------------------------------------------------------------------
		float dt = GetFrameTime();
		process_input(dt);

		if (g.mode == START || g.mode == PLAY)
		{
			update_player();
			update_ball(dt);
			check_collision();
		}

		// Draw
		//----------------------------------------------------------------------------------
	    BeginDrawing();
		ClearBackground(BLACK);

		if (g.mode == MENU)
		{
			render_menu();
		}
		else if (g.mode == START || g.mode == PLAY)
		{
			for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
			{
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
	CloseWindow(); // Close window and OpenGL context
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

void check_collision()
{
	if ((b.position.x + b.width > player.position.x && b.position.x < player.position.x + player.width) &&
		 b.position.y + b.height >= player.position.y)
	{
		b.color = GREEN;
		b.direction.y *= -1;
	}
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

void render_game_over()
{
	DrawText("GAME OVER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 64, RED);
}

void render_player()
{
	int space = 50;
	for (int i = 1; i <= player.lives; ++i)
	{
		DrawRectangle(i * space, 50, 25, 25, RED);
	}

  	DrawRectangle(player.position.x, player.position.y, player.width, player.height, BLUE);
}

void render_ball()
{
  DrawRectangle(b.position.x, b.position.y, b.width, b.height, b.color);
}
