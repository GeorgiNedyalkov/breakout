#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

// TODO: Render the game on different scren sizes
// Based on an aspect ratio to work for every screen
#define FPS                 60
#define SCREEN_WIDTH        1600
#define SCREEN_HEIGHT       800
#define GRID_COLS           10
#define GRID_ROWS           5
#define BRICKS_PER_ROW      10
#define BRICK_WIDTH         SCREEN_WIDTH / BRICKS_PER_ROW
#define BRICK_HEIGHT        40
#define MAX_LEVELS          4
#define PADDLE_WIDTH        100
#define PADDLE_HEIGHT       25
#define PADDLE_SPEED        1200
#define PLAYER_LIVES        3
#define HOVER_DISTANCE      25
#define BALL_WIDTH          25
#define BALL_HEIGHT         25
#define BALL_SPEED          500
#define BALL_SPEED_INCREASE 20

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
    int       score;
    int       level_index;
    int       level_bricks_count;
    bool      control_ball;
} game;

typedef struct paddle
{
    Vector2 position;
    Vector2 direction;
    Color   color;
    int     width;
    int     height;
    int     lives;
} paddle;

typedef struct ball
{
    Vector2 position;
    Vector2 direction;
    Color   color;
    float   speed;
    int     width;
    int     height;
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

typedef enum direction_t
{
    RIGHT,
    LEFT,
    UP,
    DOWN,
} direction_t;

void process_input(float dt);
void init_level(int level_index);

void    render_player(void);
void    render_ball(void);
void    render_bricks(void);
void    render_menu(void);
void    render_level_completed(int);
void    render_debug_info(void);
void    render_at_center(const char *, float, Color);
Vector2 get_text_center(const char *, float);

void update_ball(float dt);
void update_player(void);

bool        check_collision(Vector2, int, int, Vector2, int, int);
float       calculate_distance_from_centers(void);
direction_t get_hit_direction(Vector2);
Vector2     get_closest_point(ball, brick);
void        bounce_off_paddle(void);

void reset_ball_position(void);
void reset_positions(void);
void reset_game(void);

void kill_brick(void);

game   g;
paddle player;
ball   b;
brick  bricks[GRID_ROWS * GRID_COLS];

char levels[MAX_LEVELS][GRID_ROWS][GRID_COLS] = {
    {
        ".r.r.r.r.r",
		"g.g.g.g.g.",
        ".v.v.v.v.v",
        "b.b.b.b.b.",
        ".p.p.p.p.p",
    },
    {
        "..........",
        ".pppppppp.",
        "...oooo...",
        ".gg.rr.gg.",
        "g...rr...g",
    },

    {
        "..........",
        "..........",
        "..........",
        "..........",
        ".bbbbbbbb.",
    },
    {
        ".bbbbbbbb.",
        "..gggggg..",
        "...rrrr...",
        "....pp....",
        "..........",
    },
};

Music background_music;
Sound collision_sound;
Sound new_level_sound;

Texture2D background;

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");
    InitAudioDevice();
    SetTargetFPS(FPS);

    // Initialize Sound and Musc
    background_music = LoadMusicStream("../assets/background.mp3");
    collision_sound  = LoadSound("../assets/collision.mp3");
    new_level_sound  = LoadSound("../assets/new_level.mp3");

    if (IsMusicValid(background_music))
    {
        SetMusicVolume(background_music, 0.5f);
        PlayMusicStream(background_music);
    }

    // Initialize Textures
    background = LoadTexture("../assets/images/background_ocean.png");

    // First level initialization and every other
    init_level(0);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(background_music);

        if (player.lives <= 0)
        {
            g.mode = OVER;
        }

        if (g.level_index == MAX_LEVELS)
        {
            g.mode = FINISH;
        }

        if (g.level_bricks_count == 0)
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
                PlaySound(collision_sound);
            }

            // NOTE: (Optimization) Here if there are no bricks in the level don't continue in the loop
            for (int i = 0; i < sizeof(bricks) / sizeof(brick) && g.level_bricks_count > 0; ++i)
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
                        g.level_bricks_count--;
                    }
                    else
                    {
                        bricks[i].color = ColorAlpha(bricks[i].color, 0.5f);
                    }

                    Vector2     closest_point = get_closest_point(b, bricks[i]);
                    direction_t hit_direction = get_hit_direction(closest_point);

                    if (hit_direction == RIGHT || hit_direction == LEFT)
                    {
                        b.direction.x *= -1;
                    }
                    else if (hit_direction == UP || hit_direction == DOWN)
                    {
                        b.direction.y *= -1;
                    }

                    b.speed += BALL_SPEED_INCREASE;
                    g.score += 10;
                    PlaySound(collision_sound);
                }
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw background
        DrawTextureEx(background, (Vector2){0.0f, 0.0f}, 0, 2.8f, WHITE);

        // Draw HUD
        DrawText(TextFormat("Score: %d\n", g.score), 10, SCREEN_HEIGHT - 20, 16, BLACK);

        for (int i = 0; i < player.lives; ++i)
        {
            int pos_x = 100 + i * 20;
            DrawRectangle(pos_x, SCREEN_HEIGHT - 20, 10, 10, RED);
        }

        switch (g.mode)
        {
        case MENU:
            render_at_center("PAUSE", 24.0f, PURPLE);
            break;
        case OVER:
            render_at_center("GAME OVER", 64.0f, RED);
            break;
        case START:
            render_at_center("Press SPACE to start", 24.0f, GREEN);
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
            // NOTE: Here add a little time to view the screen
            render_level_completed(g.level_index);
            init_level(g.level_index);
            g.mode = START;
            break;
        case FINISH:
            render_at_center("GAME COMPLETED", 64.0f, GOLD);
            break;
        default:
            break;
        }

        EndDrawing();
    }

    UnloadSound(collision_sound);
    UnloadMusicStream(background_music);
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
            ResumeMusicStream(background_music);
        }
        else if (g.mode == PLAY)
        {
            PauseMusicStream(background_music);
            g.mode = MENU;
        }
    }
    else if (IsKeyPressed(KEY_R))
    {
        reset_game();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        kill_brick();
    }
}

// TODO: This has bugs and I don't understand the initialization of current_level
void init_level(int level_index)
{
    if (level_index == 0)
    {
        g.level_index  = 0;
        g.control_ball = false;
        g.score        = 0;

        player.width  = PADDLE_WIDTH;
        player.height = PADDLE_HEIGHT;
        player.lives  = PLAYER_LIVES;
        player.color  = PURPLE;

        b.width       = BALL_WIDTH;
        b.height      = BALL_HEIGHT;
        b.color       = WHITE;
        b.direction.x = 0.5f;
        b.direction.y = 1;
    }

    if (level_index != 0)
    {
        PlaySound(new_level_sound);
    }

    g.mode               = START;
    b.speed              = BALL_SPEED;
    g.level_bricks_count = 0;

    char (*current_level)[GRID_COLS] = levels[level_index];

    for (int row = 0; row < GRID_ROWS; ++row)
    {
        for (int col = 0; col < GRID_COLS; ++col)
        {
            if (current_level[row][col] == '.')
            {
                continue;
            }
            // NOTE: What is this '\0'? The null terminator for a string ok but why?
            else if (current_level[row][col] != '\0')
            {
                int padding = 100;
                int gap     = 0;

                brick new_brick;
                new_brick.width      = BRICK_WIDTH;
                new_brick.height     = BRICK_HEIGHT;
                new_brick.position.x = col * BRICK_WIDTH;
                new_brick.position.y = row * BRICK_HEIGHT + padding;
                new_brick.is_dead    = false;
                new_brick.lives      = 1;

                switch (current_level[row][col])
                {
                case 'r':
                    new_brick.color = RED;
                    break;
                case 'y':
                    new_brick.color = YELLOW;
                    break;
                case 'o':
                    new_brick.color = ORANGE;
                    break;
                case 'l':
                    new_brick.color = LIME;
                    break;
                case 'm':
                    new_brick.color = MAROON;
                    break;
                case 'g':
                    new_brick.color = GREEN;
                    break;
                case 'b':
                    new_brick.color = BLUE;
                    new_brick.lives = 2;
                    break;
                case 'v':
                    new_brick.color = VIOLET;
                    break;
                case 'p':
                    new_brick.color = PINK;
                    break;
                default:
                    bricks->color = DARKGRAY;
                    break;
                }

                bricks[g.level_bricks_count] = new_brick;
                g.level_bricks_count++;
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
    b.position.y = player.position.y - b.height;

    float distance = calculate_distance_from_centers();

    Vector2 new_direction = {distance, b.direction.y * -1};
    new_direction         = Vector2Normalize(new_direction);
    b.direction           = new_direction;
}

void update_ball(float dt)
{
    if (g.mode == START)
    {
        reset_ball_position();
        b.speed = BALL_SPEED;
    }
    else if (g.control_ball)
    {
        Vector2 mouse_position = GetMousePosition();
        b.position             = mouse_position;
    }
    else
    {
        b.position.x += b.direction.x * b.speed * dt;
        b.position.y += b.direction.y * b.speed * dt;

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
            b.speed = BALL_SPEED;
            reset_ball_position();
            g.mode = START;
        }
        else if (b.position.y <= 0)
        {
            b.direction.y = 1;
        }
    }
}

void reset_ball_position(void)
{
    b.position.x  = player.position.x + ((float)player.width / 2) - (float)b.width / 2;
    b.position.y  = player.position.y - player.height;
    b.direction.y = -1;
}

void reset_positions(void)
{
    reset_ball_position();

    player.position.x = ((float)SCREEN_WIDTH / 2) - (float)PADDLE_WIDTH / 2;
    player.position.y = SCREEN_HEIGHT - PADDLE_HEIGHT - HOVER_DISTANCE;
}

void reset_game(void)
{
    player.lives  = PLAYER_LIVES;
    g.level_index = 0;

    init_level(0);
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
    const char *text      = TextFormat("Level Bricks Count = %i", g.level_bricks_count);
    DrawText(text, 100.0f, SCREEN_HEIGHT - 100.0f, font_size, GREEN);
}

void render_player(void)
{
    DrawRectangle(player.position.x, player.position.y, player.width, player.height, player.color);
}

void render_ball(void) { DrawRectangle(b.position.x, b.position.y, b.width, b.height, b.color); }

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
            g.level_bricks_count--;
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
