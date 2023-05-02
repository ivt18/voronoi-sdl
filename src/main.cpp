#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// #define DEBUG

// window dimensions
#define WIDTH 1000
#define HEIGHT 1000

// number of centroids
#define CENTROID_CNT 10
#define CENTROID_RADIUS 5

struct Vector2 {
    Uint32 x, y;

    Vector2() {}
    Vector2(Uint32 x, Uint32 y) {
        this->x = x;
        this->y = y;
    }
};

struct Color {
    Uint8 r, g, b;

    Color() {}
    Color(Uint8 r, Uint8 g, Uint8 b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

static Vector2 centroid_positions[CENTROID_CNT];
static Color centroid_colors[CENTROID_CNT];

static inline void generate_centroids() {
    for (size_t i = 0; i < CENTROID_CNT; ++i) {
        centroid_positions[i].x = rand() % WIDTH;
        centroid_positions[i].y = rand() % HEIGHT;
        centroid_colors[i].r = rand() % UINT8_MAX;
        centroid_colors[i].g = rand() % UINT8_MAX;
        centroid_colors[i].b = rand() % UINT8_MAX;

        #ifdef DEBUG
            printf("Centroids:\n");
            printf("(%d, %d)\n", centroid_positions[i].x, centroid_positions[i].y);
        #endif
    }
}

double dist2(Vector2 src, Vector2 dst) {
    Uint32 distx = src.x - dst.x;
    Uint32 disty = src.y - dst.y;
    return distx * distx + disty * disty;
}

void draw_pixel(SDL_Renderer *renderer, Vector2 pos, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, pos.x, pos.y);
}

void draw_circle(SDL_Renderer *renderer, Vector2 pos, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    
    int diameter = 2 * CENTROID_RADIUS;
    int x0 = CENTROID_RADIUS - 1;
    int y0 = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - diameter;

    while (x0 >= y0) {
        SDL_RenderDrawLine(renderer, pos.x + x0, pos.y + y0, pos.x - x0, pos.y + y0);
        SDL_RenderDrawLine(renderer, pos.x + y0, pos.y + x0, pos.x - y0, pos.y + x0);
        SDL_RenderDrawLine(renderer, pos.x - x0, pos.y - y0, pos.x + x0, pos.y - y0);
        SDL_RenderDrawLine(renderer, pos.x - y0, pos.y - x0, pos.x + y0, pos.y - x0);

        if (error <= 0) {
            ++y0;
            error += ty;
            ty += 2;
        }

        if (error > 0) {
            --x0;
            tx += 2;
            error += tx - diameter;
        }
    }
}

void update(SDL_Renderer *renderer, SDL_Texture *buf) {
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, buf, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void render(SDL_Renderer *renderer, SDL_Texture *buf) {
    SDL_SetRenderTarget(renderer, buf);

    // render each one of the cells (can use some optimization)
    double maxDistance = WIDTH * WIDTH + HEIGHT * HEIGHT;
    for (Uint32 x = 0; x < WIDTH; ++x) {
        for (Uint32 y = 0; y < HEIGHT; ++y) {
            double d = maxDistance;
            Color *c;
            for (size_t i = 0; i < CENTROID_CNT; ++i) {
                double nd = dist2(Vector2(x, y), centroid_positions[i]);
                if (nd < d) {
                    d = nd;
                    c = &centroid_colors[i];
                }
            }
            draw_pixel(renderer, Vector2(x, y), *c);
        }
    }

    // render the dots for the centroids
    for (size_t i = 0; i < CENTROID_CNT; ++i) {
        draw_circle(renderer, centroid_positions[i], Color(0, 0, 0));
    }
}

int main() {
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window* win = SDL_CreateWindow("Voronoi",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       WIDTH, HEIGHT,
                                       SDL_WINDOW_SHOWN);

    if (win == NULL)
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());

    // create renderer
    Uint32 renderFlags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, renderFlags);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // create a texture to use as a buffer
    SDL_Texture *buf = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            WIDTH, HEIGHT);
    SDL_SetRenderTarget(renderer, buf);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // generate the centroid positions
    generate_centroids();

    // render the application
    render(renderer, buf);
    update(renderer, buf);

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_SPACE:
                            generate_centroids();
                            render(renderer, buf);
                            update(renderer, buf);
                            break;

                        case SDLK_q:
                        case SDLK_ESCAPE:
                            quit = true;
                            break;

                        default: break;
                    }

                default: break;
            }
        }
    }

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
