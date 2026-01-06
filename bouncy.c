#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define ACCEL 5000
#define WIDTH 900
#define HEIGTH 600
#define K_F 0

typedef struct {
  double x, y;
  double r;
  double xv, yv;
} Balle;

typedef struct {
  Balle *balles;
  int n; // Nombre balles
  SDL_Window *win;
  SDL_Renderer *ren;
  struct timespec last;
  size_t td;
  SDL_AudioStream *stream;
  Uint8 *audio_buf;
  Uint32 *audio_len;
} App;

/* Cette fonction dessine une balle  */
void DrawBall(SDL_Renderer *ren, Balle balle) {
  SDL_SetRenderDrawColor(ren, 100, 100, 100, SDL_ALPHA_OPAQUE);

  int x_start = balle.x - balle.r;
  int y_start = balle.y - balle.r;

  int x_stop = balle.x + balle.r;
  int y_stop = balle.y + balle.r;

  for (int x = x_start; x <= x_stop; x++) {
    for (int y = y_start; y <= y_stop; y++) {
      if (pow(x - balle.x, 2) + pow(y - balle.y, 2) <= pow(balle.r, 2))
        SDL_RenderPoint(ren, (float)x, (float)y);
    }
  }
}

void BoomSound(App *app) {
  Uint32 len;
  Uint8 *buf;
  SDL_AudioSpec spec;
  SDL_ClearAudioStream(app->stream);
  if (!SDL_LoadWAV("./assets/snore.wav", &spec, &buf, &len)) {
    printf("Cannot load wav file.\n%s\n", SDL_GetError());
  }
  SDL_PutAudioStreamData(app->stream, buf, len);
}

void UpdateBalls(void *app) {

  App *ctx = (App *)app;
  Balle *balles = ctx->balles;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  double dt = (now.tv_sec + now.tv_nsec / 1000000000.0f) -
              (ctx->last.tv_sec + ctx->last.tv_nsec / 1000000000.0f);
  ctx->last = now;

  for (int i = 0; i < ctx->n; i++) {
    float k;
    if (balles[i].yv > 0)
      k = -K_F;
    else
      k = K_F;
    float a = ACCEL + k;
    balles[i].xv += 0;
    balles[i].yv += a * dt;
    balles[i].x += balles[i].xv * dt;
    balles[i].y += balles[i].yv * dt;
    if (balles[i].y >= HEIGTH - (balles[i].r)) {
      balles[i].y = HEIGTH - (balles[i].r);
      balles[i].yv = -balles[i].yv;
      BoomSound(ctx);
    }
    if (balles[i].x >= WIDTH - (balles[i].r)) {
      balles[i].x = WIDTH - (balles[i].r);
      balles[i].xv = -balles[i].xv;
      BoomSound(ctx);
    }
    if (balles[i].x - balles[i].r <= 0) {
      balles[i].x = balles[i].r;
      balles[i].xv = -balles[i].xv;
      BoomSound(ctx);
    }

    DrawBall(ctx->ren, balles[i]);
  }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  srand(time(NULL));
  App *app = malloc(sizeof(*app));
  SDL_SetAppMetadata("Bouncy Balls", "1.0", "org.jospeh.bouncy-balls");

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("BouncyBall", 900, 600, 0, &app->win,
                                   &app->ren)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderVSync(app->ren, 1);

  SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, 48000};
  SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
  SDL_ResumeAudioStreamDevice(stream);
  app->stream = stream;

  app->n = 2;
  app->balles = malloc(sizeof(*app->balles) * app->n);
  Balle balle0 = {WIDTH / 2.0, 50, 100, 800, 100};
  Balle balle1 = {WIDTH / 4.0, 50, 60, 1200, 0};
  app->balles[0] = balle0;
  app->balles[1] = balle1;

  SDL_SetRenderDrawColor(app->ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
  clock_gettime(CLOCK_MONOTONIC, &app->last);

  *appstate = app;

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
  App *app = appstate;
  SDL_SetRenderDrawColor(app->ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->ren);

  UpdateBalls(app);
  clock_gettime(CLOCK_MONOTONIC, &app->last);

  SDL_RenderPresent(app->ren);
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
