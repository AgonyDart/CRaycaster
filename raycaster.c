#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

// Screen dimensions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 512

// Map dimensions and tile size
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define TILE_SIZE 64

// Field of view and number of rays
#define FOV (M_PI / 3) // 60 degrees
#define NUM_RAYS SCREEN_WIDTH

// Minimap
#define MINIMAP_SIZE 200
#define MINIMAP_X 10
#define MINIMAP_Y 10

// Player settings
typedef struct
{
    float x, y;  // Position
    float angle; // Direction angle
    float fov;   // Field of view
} Player;

Player player = {150, 150, 0, M_PI / 3};

// Map (1 = wall, 0 = empty space)
int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}};

// SDL variables
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// Player movement
#define MOVE_SPEED 1.0
#define ROTATE_SPEED 0.01

void handleInput(bool *running)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_W])
    {
        player.x += cos(player.angle) * MOVE_SPEED;
        player.y += sin(player.angle) * MOVE_SPEED;
    }

    if (keystate[SDL_SCANCODE_S])
    {
        player.x -= cos(player.angle) * MOVE_SPEED;
        player.y -= sin(player.angle) * MOVE_SPEED;
    }

    if (keystate[SDL_SCANCODE_A])
    {
        player.angle -= ROTATE_SPEED;
    }

    if (keystate[SDL_SCANCODE_D])
    {
        player.angle += ROTATE_SPEED;
    }

    if (keystate[SDL_SCANCODE_ESCAPE])
    {
        *running = false;
    }
}

// Render the 2D map
void renderMap()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (map[y][x] == 1)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            SDL_Rect rect = {x * TILE_SIZE - 1, y * TILE_SIZE - 1, TILE_SIZE - 1, TILE_SIZE - 1};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

// Render the player
void renderPlayer()
{
    // Draw the player as a circle
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    int radius = 8;
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w; // Horizontal distance to the center
            int dy = radius - h; // Vertical distance to the center
            if ((dx * dx + dy * dy) <= (radius * radius))
            {
                SDL_RenderDrawPoint(renderer, player.x + dx, player.y + dy);
            }
        }
    }
    // Draw the player's direction
    // SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    // int lineX = player.x + cos(player.angle) * 64;
    // int lineY = player.y + sin(player.angle) * 64;
    // SDL_RenderDrawLine(renderer, player.x, player.y, lineX, lineY);
}

// Cast a single ray
float castRay(float rayAngle)
{
    // Normalize angle to the range [0, 2*PI)
    rayAngle = fmod(rayAngle, 2 * M_PI);
    if (rayAngle < 0)
        rayAngle += 2 * M_PI;

    // Ray starting position
    float rayX = player.x;
    float rayY = player.y;

    // Ray direction increments
    float rayStepX = cos(rayAngle);
    float rayStepY = sin(rayAngle);

    // Step until ray hits a wall or goes out of bounds
    while (1)
    {
        int mapX = (int)(rayX / TILE_SIZE);
        int mapY = (int)(rayY / TILE_SIZE);

        // Check if the ray is out of bounds
        if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT)
        {
            break;
        }

        // Check if the ray hit a wall
        if (map[mapY][mapX] == 1)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawLine(renderer, player.x, player.y, rayX, rayY);
            break;
        }

        // Move the ray forward
        rayX += rayStepX;
        rayY += rayStepY;
    }
    float distance = sqrt((rayX - player.x) * (rayX - player.x) + (rayY - player.y) * (rayY - player.y));
    return distance;
}

// Render the rays in the player's field of view
void renderRays()
{
    int numRays = 512; // Number of rays to cast
    float startAngle = player.angle - player.fov / 2;
    float angleStep = player.fov / numRays;

    for (int i = 0; i < numRays; i++)
    {
        float rayAngle = startAngle + i * angleStep;
        castRay(rayAngle);
    }
}

// Render 3D view
void render3DView()
{
    float distance = 0;
    float wallHeight = 0;

    // Set offset to right edge of the screen
    int offset = SCREEN_WIDTH / 2;

    // Calculate the distance to the wall
    int numRays = 128; // Use screen width for number of rays
    float startAngle = player.angle - player.fov / 2;
    float angleStep = player.fov / numRays;

    for (int i = 0; i < numRays; i++)
    {
        float rayAngle = startAngle + i * angleStep;
        distance = castRay(rayAngle);             // Assuming castRay() returns the distance
        distance *= cos(player.angle - rayAngle); // Fix fish-eye effect

        // Calculate the height of the wall slice
        wallHeight = (TILE_SIZE / distance) * 277; // Scale factor for wall height

        // Calculate the color based on distance
        // Closer walls will be brighter, distant walls will be darker
        float brightness = 1.0f / (distance * 0.01);  // Smooth brightness factor
        int colorIntensity = (int)(255 * brightness); // Max intensity is 255 (white)

        // Ensure minimum brightness to avoid completely dark walls
        if (colorIntensity < 64)
            colorIntensity = 64;
        if (colorIntensity > 255)
            colorIntensity = 255;

        // Calculate color based on the intensity
        SDL_SetRenderDrawColor(renderer, colorIntensity, 0, colorIntensity, 255);

        // Draw the wall slice
        SDL_Rect rect = {offset, SCREEN_HEIGHT / 2 - wallHeight / 2, SCREEN_WIDTH / numRays, wallHeight};
        SDL_RenderFillRect(renderer, &rect);

        offset += SCREEN_WIDTH / numRays;
    }
}

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window)
    {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main game loop
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        handleInput(&running);

        // Render scene
        SDL_SetRenderDrawColor(renderer, 2, 2, 2, 255);
        SDL_RenderClear(renderer);

        renderMap();
        renderPlayer();
        renderRays();

        // Render 3D view
        render3DView();

        SDL_RenderPresent(renderer);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}