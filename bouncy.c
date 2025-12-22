#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>

#define G_CONST 90.81

typedef struct {
  float x;
  float y;
  float r;
  float xv;
  float yv;
} Balle;

typedef struct {
  Balle balles[50]; 
  int n_balles;
} BounceApp;

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
struct timeval t_start;
struct timeval t_end;
struct timeval td;
BounceApp app;

/* Cette fonction dessine une balle  */
void DrawBall(Balle balle) {
  SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);

  int x_start = balle.x - balle.r;
  int y_start = balle.y - balle.r;

  int x_stop = balle.x + balle.r;
  int y_stop = balle.y + balle.r;

  for (int x = x_start; x <= x_stop; x++) {
    for (int y = y_start; y <= y_stop; y++) {
      if (pow(x - balle.x, 2) + pow(y - balle.y, 2) <= pow(balle.r, 2))
        SDL_RenderPoint(renderer, (float)x, (float)y);
    }
  }
}

void InitBalls(Balle balles[], int n)
{
  for (int i = 0; i <= n; i++) 
  {
    DrawBall(balles[i]);
  }
}

void UpdateBalls(Balle balles[], int n)
{
  timersub(&t_start, &t_end, &td);
  double t_eval = (double)td.tv_sec + (double)td.tv_usec / 1000000;

  for (int i = 0; i <= n; i++) 
  {
    float x_start = balles[i].x;
    float y_start = balles[i].y;

    balles[i].x = x_start + balles[i].xv * t_eval;
    balles[i].y = y_start + balles[i].yv * t_eval + 1/2 * G_CONST * pow(t_eval, 2);

    balles[i].xv = (balles[i].x - x_start) / t_eval;
    balles[i].yv = (balles[i].y - y_start) / t_eval;

    printf("Ball rendue a x: %f, y; %f\n", balles[i].x, balles[i].y, balles[i].xv, balles[i].yv);

    DrawBall(balles[i]);
    printf("%f\n", t_eval);
  }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Bouncy Balls", "1.0", "org.jospeh.bouncy-balls");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("BouncyBall", 900, 600, 0, &window,
                                   &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderVSync(renderer, 1);

  *appstate = &app;


  app.balles[0] = (Balle){300, 300, 100, 0, 0};
  app.n_balles = 1;
  
  InitBalls(app.balles, app.n_balles);
  gettimeofday(&t_start, NULL);
  gettimeofday(&t_end, NULL);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
  }
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  gettimeofday(&t_start, NULL);
  UpdateBalls(app.balles, app.n_balles);
  gettimeofday(&t_end, NULL);

  SDL_RenderPresent(renderer);
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
