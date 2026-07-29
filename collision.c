#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

static int window_width  = 1600;
static int window_height = 1200;

static bool running;
static bool control_mouse;

float min(float value, float max) { return value > max ? max : value; }

float max(float value, float min) { return value < min ? min : value; }

float clamp(float value, float minv, float maxv) { return max(minv, min(value, maxv)); }

int main()
{
    InitWindow(window_width, window_height, "Collision Tests");
    SetTargetFPS(60);

    running       = true;
    control_mouse = true;

    float   circle_radius   = 50.0f;
    Vector2 circle_position = {100.0f, 100.0f};
    Vector2 circle_velocity = {200.0f, 200.0f};
    Color   circle_color    = BLUE;

    int     rect_width    = 400;
    int     rect_height   = 200;
    Vector2 rect_position = {300.0f, 300.0f};

    Vector2 rect_center;
    rect_center.x = rect_position.x + (rect_width / 2.0);
    rect_center.y = rect_position.y + (rect_height / 2.0);

    // NOTE: This need renaming
    Vector2 difference;

    while (!WindowShouldClose() && running)
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            running = false;
        }
        else if (IsKeyPressed(KEY_C))
        {
            control_mouse = !control_mouse;
        }

        float dt = GetFrameTime();

        if (control_mouse)
        {
            Vector2 mouse_position = GetMousePosition();
            circle_position.x      = mouse_position.x;
            circle_position.y      = mouse_position.y;
        }
        else
        {
            circle_position.x += circle_velocity.x * dt;
            circle_position.y += circle_velocity.y * dt;
        }

        if (circle_position.x < 0 + circle_radius || circle_position.x > window_width - circle_radius)
        {
            circle_velocity.x *= -1;
        }
        else if (circle_position.y < 0 + circle_radius || circle_position.y >= window_height - circle_radius)
        {
            circle_velocity.y *= -1;
        }

        // Check Ball and Rect collision
        // Find the closest point from the circle center to the rect
        // If the closest point is greater than the circle radius then there is no collision
        // 1. Get the vector from the circle center (position) to the rect center
        Vector2 distance_from_centers;
        distance_from_centers.x = circle_position.x - rect_center.x;
        distance_from_centers.y = circle_position.y - rect_center.y;

        // 2. Calculate the halfs of the rectangle - h / 2 and w / 2;
        float rect_half_width  = rect_width / 2.0;
        float rect_half_height = rect_height / 2.0;

        // 3. Clamp the vector where they intersect to the half extents (h/2 and w/2) of the rect
        float clamped_x = clamp(distance_from_centers.x, -rect_half_width, rect_half_width);
        float clamped_y = clamp(distance_from_centers.y, -rect_half_height, rect_half_height);

        // 4. Add the clamped x and y to the rect center to find the closest point coordinates
        float closest_x = rect_center.x + clamped_x;
        float closest_y = rect_center.y + clamped_y;

        difference.x = closest_x + circle_position.x;
        difference.y = closest_y + circle_position.y;

        // 5. Find the lengt of the difference vector and compare if its lower than the radius
        // Using pythagorean theorem

        float difference_length = sqrt(pow(difference.x, 2) + pow(difference.y, 2));

        if (difference_length < circle_radius)
        {
            circle_color = RED;
        }
        else
        {
            circle_color = GREEN;
        }

        // Collision Resolution
        // Find the level of penetration and adjust the ball position by it
        // 1. Get the vector from the circle center to the closest point

		Vector2 p;
		p.x = circle_position.x + difference.x;
		p.y = circle_position.y + difference.y;

		// // 2. Get the
		//       Vector2 v = {p.x - circle_position.x, p.y - circle_position.y};
		//
		// // 3. Get the level of penetration
		//       Vector2 r = {v.x - circle_radius, v.y - circle_radius};

		// 4. Adjust circle position
		// circle_position.x -= r.x;
		// circle_position.y -= r.y;

        BeginDrawing();
        ClearBackground(BLACK);

        DrawCircleLines(circle_position.x, circle_position.y, circle_radius, circle_color);
        DrawRectangleLines(rect_position.x, rect_position.y, rect_width, rect_height, GREEN);

        DrawLine(circle_position.x, circle_position.y, rect_center.x, rect_center.y, BLUE);

        // Draw debug information
        DrawRectangleLines(closest_x, closest_y, 5, 5, RED);
        DrawRectangleLines(p.x, p.y, 5, 5, PURPLE);

        DrawLine(circle_position.x, circle_position.y, p.x, p.y, PURPLE);
        // DrawRectangleLines(v.x, v.y, 5, 5, VIOLET);
        // DrawRectangleLines(r.x, r.y, 5, 5, PINK);

        DrawText(TextFormat("P x:%.0f y:%0.f\n", p.x, p.y), 100, window_height - 250, 24, GREEN);
        // DrawText(TextFormat("V x:%.0f y:%0.f\n", v.x, v.y), 100, window_height - 200, 24, GREEN);

        DrawText(TextFormat("Closest x:%.0f y:%0.f\n", closest_x, closest_y), 100, window_height - 150, 24, GREEN);
        DrawText(TextFormat("Clamped Points x:%.0f y:%0.f\n", clamped_x, clamped_y), 100, window_height - 100, 24, GREEN);
        DrawText(TextFormat("Diff x:%.0f y:%0.f\n", difference.x, difference.y), 100, window_height - 50, 24, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
