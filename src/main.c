#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

// TODO: Render the game on different scren sizes
// Based on an aspect ratio to work for every screen
#define FPS 60
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 800
#define PADDLE_WIDTH 200
#define PADDLE_HEIGHT 25
#define PADDLE_SPEED 1000
#define PLAYER_LIVES 3
#define BALL_WIDTH 25
#define BALL_HEIGHT 25
#define BALL_SPEED 500
#define BRICK_WIDTH 150
#define BRICK_HEIGHT 50
#define GRID_COLS 10
#define GRID_ROWS 5
#define MAX_LEVELS 3

typedef enum direction_t
{
    EAST,
    WEST,
    NORTH,
    SOUTH,
} direction_t;

typedef enum game_mode
{
    MENU,
    START,
    PLAY,
    COMPLETED,
    OVER,
    FINISH,
} game_mode;

typedef struct game
{
    game_mode mode;
    int       level_index;
    bool      control_ball;
} game;

typedef struct paddle
{
    int     width;
    int     height;
    Vector2 position;
    Vector2 direction;
    int     lives;
} paddle;

typedef struct ball
{
    int     width;
    int     height;
    Vector2 position;
    Vector2 direction;
    Color   color;
} ball;

typedef struct brick
{
    Vector2 position;
    Color   color;
    int     width;
    int     height;
    int     lives;
    bool    is_dead;
} brick;

void process_input(float dt);
void init_level(int level_index);

void render_player(void);
void render_ball(void);
void render_bricks(void);
void render_at_center(const char *, float, Color);
void render_menu(void);
void render_level_completed(int);
void render_screen_center(void);
void render_debug_info(void);

Vector2 get_text_center(const char *, float);

void update_ball(float dt);
void update_player(void);

bool  check_collision(Vector2, int, int, Vector2, int, int);
float calculate_distance_from_centers(void);

direction_t get_hit_direction(Vector2);
Vector2     get_closest_point(ball, brick);

void bounce_off_paddle(void);
void reset_positions(void);
void reset_ball_position(void);
void kill_brick(void);

// What does the game do? Keeps its state and levels.
game   g;
paddle player;
ball   b;
int    level_bricks_count;

brick bricks[GRID_ROWS * GRID_COLS];

char levels[MAX_LEVELS][GRID_ROWS][GRID_COLS] = {
    {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
    },
    {
        "rrrrrrrrrr",
        "..........",
        "..........",
        "..........",
        "..........",
    },
    {
        "r.r.r.r.r.",
        ".v.v.v.v.v",
        "g.g.g.g.g.",
        "b.b.b.b.b.",
        ".p.p.p.p.p",
    },
};

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");
    InitAudioDevice();
    SetTargetFPS(FPS);

    // NOTE: Unused for now
    // Sound collision_sound = LoadSound("assets/collision.mp3");

    g.mode         = START;
    g.level_index  = 0;
    g.control_ball = false;

    player.position.x = ((float)SCREEN_WIDTH / 2) - (float)PADDLE_WIDTH / 2,
    player.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT, player.width = PADDLE_WIDTH, player.height = PADDLE_HEIGHT,
    player.lives = PLAYER_LIVES,

    b.width       = BALL_WIDTH;
    b.height      = BALL_HEIGHT;
    b.direction.x = 0.5f;
    b.direction.y = 1;
    b.color       = GREEN;

    // brick test_b;
    // test_b.width      = BRICK_WIDTH;
    // test_b.height     = BRICK_HEIGHT;
    // test_b.position.x = 500;
    // test_b.position.y = 400;
    // test_b.is_dead    = false;
    // test_b.lives      = 1;
    // test_b.color      = ORANGE;

    // Initialize the first level
    // Level index
    // Levels
    // Level bricks count
    init_level(g.level_index);

    while (!WindowShouldClose())
    {
        if (player.lives <= 0)
        {
            g.mode = OVER;
        }

        if (g.level_index == MAX_LEVELS)
        {
            g.mode = FINISH;
        }

        if (level_bricks_count == 0)
        {
            g.mode = COMPLETED;
        }

        float dt = GetFrameTime();
        process_input(dt);

        if (g.mode == START)
        {
            update_player();
            update_ball(dt);
        }
        else if (g.mode == PLAY)
        {
            update_player();
            update_ball(dt);

            if (check_collision(b.position, b.width, b.height, player.position, player.width, player.height))
            {
                bounce_off_paddle();
            }

			// NOTE: (Optimization) Here if there are no bricks in the level don't continue in the loop
            for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
            {
                if (bricks[i].is_dead)
                    continue;

                if (check_collision(b.position, b.width, b.height, bricks[i].position, bricks[i].width,
                                    bricks[i].height))
                {
                    bricks[i].lives--;
                    if (bricks[i].lives == 0)
                    {
                        bricks[i].is_dead = true;
                        level_bricks_count--;
                    }
                    else
                    {
                        bricks[i].color = ColorAlpha(bricks[i].color, 0.5f);
                    }

                    // NOTE: Collision Resolution and ball direction
                    // Resolution and direction need to happen here
                    Vector2     closest_point = get_closest_point(b, bricks[i]);
                    direction_t hit_direction = get_hit_direction(closest_point);

                    if (hit_direction == EAST || hit_direction == WEST)
                    {
                        b.direction.x *= -1;
                    }
                    else if (hit_direction == NORTH || hit_direction == SOUTH)
                    {
                        b.direction.y *= -1;
                    }

                    // NOTE: WE have no resolution for now
                    // If we have a hit
                }
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);

        switch (g.mode)
        {
        case MENU:
            render_at_center("PAUSE", 64.0f, PURPLE);
            break;
        case OVER:
            render_at_center("GAME OVER", 64.0f, RED);
            break;
        case START:
            render_at_center("Press SPACE to start", 64.0f, GREEN);
            render_player();
            render_ball();
            render_bricks();
            break;
        case PLAY:
            render_player();
            render_ball();
            render_bricks();
            break;
        case COMPLETED:
            g.level_index++;
            render_level_completed(g.level_index);
            init_level(g.level_index);
            g.mode = START;
            break;
        case FINISH:
            render_at_center("GAME COMPLETED.", 64.0f, GOLD);
            break;
        default:
            break;
        }

        EndDrawing();
    }

    // UnloadSound(collision_sound);
    CloseAudioDevice();
    CloseWindow();

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

    if (IsKeyPressed(KEY_ESCAPE))
    {
        WindowShouldClose();
    }
    else if (IsKeyPressed(KEY_C))
    {
        g.control_ball = !g.control_ball;
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

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        kill_brick();
    }
}

// TODO: This has bugs and I don't understand the initialization of current_level
void init_level(int level_index)
{
    level_bricks_count               = 0;
    char (*current_level)[GRID_COLS] = levels[level_index];

    for (int row = 0; row < GRID_ROWS; ++row)
    {
        for (int col = 0; col < GRID_COLS; ++col)
        {
            if (current_level[row][col] == '.')
            {
                continue;
            }
            else if (current_level[row][col] != '\0')
            {
                int padding = 50;
                int gap     = 5;

                brick new_brick;
                new_brick.width      = BRICK_WIDTH;
                new_brick.height     = BRICK_HEIGHT;
                new_brick.position.x = col * (BRICK_WIDTH + gap) + padding;
                new_brick.position.y = row * (BRICK_HEIGHT + gap) + padding;
                new_brick.is_dead    = false;
                new_brick.lives      = 1;

                switch (current_level[row][col])
                {
                case 'r':
                    new_brick.color = RED;
                    break;
                case 'g':
                    new_brick.color = GREEN;
                    break;
                case 'b':
                    new_brick.color = BLUE;
                    break;
                case 'v':
                    new_brick.color = VIOLET;
                    break;
                case 'p':
                    new_brick.color = PINK;
                    break;
                default:
                    bricks->color = YELLOW;
                    break;
                }

                bricks[level_bricks_count] = new_brick;
                level_bricks_count++;
            }
        }
    }

    reset_positions();
    g.mode = START;
}

void update_player(void)
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

float calculate_distance_from_centers(void)
{
    Vector2 p1 = player.position;
    p1.x += (float)player.width / 2;

    Vector2 b1 = b.position;
    b1.x += (float)b.width / 2;

    float distance = b1.x - p1.x;
    distance       = distance / 100;

    distance = Clamp(distance, -1.0, 1.0);

    return distance;
}

void bounce_off_paddle(void)
{
    float distance = calculate_distance_from_centers();

    Vector2 new_direction;
    new_direction.y = b.direction.y * -1;
    new_direction.x = distance;
    new_direction   = Vector2Normalize(new_direction);

    b.direction = new_direction;
}

void update_ball(float dt)
{
    if (g.mode == START)
    {
        reset_ball_position();
    }
    else if (g.control_ball)
    {
        Vector2 mouse_position = GetMousePosition();
        b.position             = mouse_position;
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
        // adding a little bit of space for the ball to drop
        if (b.position.y >= SCREEN_HEIGHT + 10)
        {
            player.lives--;
            reset_ball_position();
            g.mode = START;
        }
        else if (b.position.y <= 0)
        {
            b.direction.y = 1;
        }
    }
}

void update_bricks(void) {}

void reset_ball_position(void)
{
    b.position.x  = player.position.x + ((float)player.width / 2) - (float)b.width / 2;
    b.position.y  = player.position.y - player.height;
    b.direction.y = -1;
}

void reset_positions(void)
{
    b.position.x  = player.position.x + ((float)player.width / 2) - (float)b.width / 2;
    b.position.y  = player.position.y - player.height;
    b.direction.y = -1;

    player.position.x = ((float)SCREEN_WIDTH / 2) - (float)PADDLE_WIDTH / 2;
    player.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
}

bool check_collision(Vector2 ball, int ball_width, int ball_height, Vector2 rec, int rec_width, int rec_height)
{
    return ball.x + ball_width > rec.x && ball.x < rec.x + rec_width && ball.y + ball_height > rec.y &&
           ball.y < rec.y + rec_height;
}

Vector2 get_text_center(const char *text, float font_size)
{
    Vector2 text_dimensions = MeasureTextEx(GetFontDefault(), text, font_size, 1.0f);
    Vector2 text_position;
    text_position.x = ((float)SCREEN_WIDTH / 2) - (text_dimensions.x / 2);
    text_position.y = ((float)SCREEN_HEIGHT / 2) - (text_dimensions.y / 2);
    return text_position;
}

void render_at_center(const char *text, float font_size, Color color)
{
    Vector2 text_position = get_text_center(text, font_size);
    DrawText(text, text_position.x, text_position.y, font_size, color);
}

// Render Functions
// NOTE: Menu will include replaying episodes later
void render_menu(void) { render_at_center("PAUSE", 64.0f, PURPLE); }

void render_level_completed(int level_number)
{
    const char *text = TextFormat("Level Completed %i", level_number);
    render_at_center(text, 64.0f, GREEN);
}

void render_debug_info(void)
{
    float       font_size = 32.0f;
    const char *text      = TextFormat("Level Bricks Count = %i", level_bricks_count);
    DrawText(text, 100.0f, SCREEN_HEIGHT - 100.0f, font_size, GREEN);
}

void render_screen_center(void)
{
    DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, PURPLE);
    DrawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, PURPLE);
}

void render_player(void) { DrawRectangle(player.position.x, player.position.y, player.width, player.height, BLUE); }

void render_ball(void) { DrawRectangleLines(b.position.x, b.position.y, b.width, b.height, b.color); }

void render_bricks(void)
{
    for (int i = 0; i < sizeof(bricks) / sizeof(brick); ++i)
    {
        if (bricks[i].is_dead)
            continue;

        DrawRectangle(bricks[i].position.x, bricks[i].position.y, bricks[i].width, bricks[i].height, bricks[i].color);
    }
}

void kill_brick(void)
{
    Rectangle rec;
    rec.width  = BRICK_WIDTH;
    rec.height = BRICK_HEIGHT;

    Vector2 mouse_position = GetMousePosition();
    for (int i = 0; i < sizeof(bricks) / sizeof(bricks[0]); ++i)
    {
        rec.x = bricks[i].position.x;
        rec.y = bricks[i].position.y;
        if (CheckCollisionPointRec(mouse_position, rec) && !bricks[i].is_dead)
        {
            bricks[i].is_dead = true;
            level_bricks_count--;
        }
    }
}

// Returns the closest point from the ball center to the brick
// used to determine the hit direction later
Vector2 get_closest_point(ball b, brick test_b)
{
    // Get the centers of both objects
    Vector2 ball_center = {
        b.position.x + (b.width / 2.0),
        b.position.y + (b.height / 2.0),
    };
    Vector2 brick_center = {
        test_b.position.x + (test_b.width / 2.0),
        test_b.position.y + (test_b.height / 2.0),
    };

    // Calculate the vector from the centers
    Vector2 distance_from_centers = {
        ball_center.x - brick_center.x,
        ball_center.y - brick_center.y,
    };

    distance_from_centers.x = Clamp(distance_from_centers.x, -test_b.width / 2.0, test_b.width / 2.0);
    distance_from_centers.y = Clamp(distance_from_centers.y, -test_b.height / 2.0, test_b.height / 2.0);

    // Add it back to the rect center
    Vector2 closest_point = {brick_center.x + distance_from_centers.x, brick_center.y + distance_from_centers.y};

    // Get the Hit Direction
    Vector2 circle_to_closest = {
        ball_center.x - closest_point.x,
        ball_center.y - closest_point.y,
    };

    // NOTE: Debug only
    // Learn how to use these only in debug mode

    // DrawRectangle(brick_center.x, brick_center.y, 1, 1, RED);
    // DrawRectangle(ball_center.x, ball_center.y, 1, 1, RED);
    // DrawLine(0, 0, brick_center.x, brick_center.y, YELLOW);
    // DrawLine(0, 0, ball_center.x, ball_center.y, YELLOW);
    //
    // DrawLine(ball_center.x, ball_center.y, closest_point.x, closest_point.y, BLUE);
    // DrawRectangleLines(test_b.position.x, test_b.position.y, test_b.width, test_b.height, test_b.color);
    // DrawCircleLines(closest_point.x, closest_point.y, 5.0, BLUE);
    //
    // const char *cp_text = TextFormat("Closest point x: %.0f, y:%.0f", closest_point.x, closest_point.y);
    // DrawText(cp_text, 50, 800, 20, GREEN);
    //
    return circle_to_closest;
}

direction_t get_hit_direction(Vector2 target)
{
    // EAST, WEST, NORTH, SOUTH
    Vector2 directions[4] = {
        {1.0, 0.0},
        {-1.0, 0.0},
        {0.0, -1.0},
        {0.0, 1.0},
    };
    int best_match = -1;

    float max = 0.0;
    for (int i = 0; i < 4; ++i)
    {
        float dot = Vector2DotProduct(Vector2Normalize(target), directions[i]);
        if (dot > max)
        {
            max        = dot;
            best_match = i;
        }
    }

    // NOTE: Debug only
    // printf("Hit Direction: %d\n", hit_direction);

    return (direction_t)best_match;
}
