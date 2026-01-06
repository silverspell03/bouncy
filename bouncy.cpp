#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <stddef.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>

#define ACCEL 5000
#define WIDTH 900
#define HEIGTH 600
#define K_F 0
#define CHANNELS 2
#define SAMPLE_RATE 44100
#define AUDIO_FORMAT SDL_AUDIO_S16


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
  std::chrono::steady_clock::time_point last;
  double accumulator;   // seconds
  double fixed_dt;      // seconds per physics step
  double sim_time;      // total simulated time
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

void BoomSound(App *app)
{
  if (!app) return;
  if (!app->stream)
  {
    printf("Pas pu trouver stream de l'app\n%s", SDL_GetError());
    return;
  }
  SDL_ClearAudioStream(app->stream);

  Uint8 *wav_buf = NULL;
  Uint32 wav_len = 0;
  SDL_AudioSpec wav_spec;
  
  const SDL_AudioSpec target = {AUDIO_FORMAT, CHANNELS, SAMPLE_RATE};

  if (!SDL_LoadWAV("assets/snore.wav", &wav_spec, &wav_buf, &wav_len))
  {
    printf("Cannot load wav file.\n%s\n", SDL_GetError());
  }

  const SDL_AudioSpec src_spec = wav_spec;
  const Uint8 *src_buf = wav_buf;
  const Uint32 src_len = wav_len;

  Uint8 *con_buf = NULL;
  int con_len = 0;


  if (!SDL_ConvertAudioSamples(&src_spec, src_buf, src_len, &target, &con_buf, &con_len))
  {
    printf("Erreur conversion\n%s", SDL_GetError());
    return;
  }
  if (!SDL_PutAudioStreamData(app->stream, con_buf, con_len))
  {
    printf("Error putting data on stream\n%s", SDL_GetError());
    return;
  }
}

void UpdateBallsPhysics(App *app, double dt)
{

  App *ctx = (App *)app;
  Balle *balles = ctx->balles;
  for (int i = 0; i < ctx->n; i++) 
  {
    float kx, ky;
    if (balles[i].yv > 0)
      ky = -K_F;
    else 
      ky = K_F;
    if (balles[i].xv > 0)
      kx = K_F;
    else 
      kx = -K_F;

    float ay = ACCEL + ky;
    balles[i].xv += kx * dt;
    balles[i].yv += ay * dt;
    balles[i].x += balles[i].xv * dt;
    balles[i].y += balles[i].yv * dt;
    if (balles[i].y >= HEIGTH - (balles[i].r))
    {
      balles[i].y = HEIGTH - (balles[i].r);
      balles[i].yv = -balles[i].yv;
      BoomSound(ctx);
    }
    if (balles[i].x >= WIDTH - (balles[i].r))
    {
      balles[i].x = WIDTH - (balles[i].r);
      balles[i].xv = -balles[i].xv;
      BoomSound(ctx);
    }
    if (balles[i].x - balles[i].r <= 0)
    {
      balles[i].x = balles[i].r;
      balles[i].xv = -balles[i].xv;
      BoomSound(ctx);
    }
  }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  using clock = std::chrono::steady_clock;

  srand(time(NULL));
  App *app = (App *)malloc(sizeof(*app));
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

  SDL_AudioSpec spec = {AUDIO_FORMAT, CHANNELS, SAMPLE_RATE};
  SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
  SDL_ResumeAudioStreamDevice(stream);
  if (stream == NULL) 
  {
    printf("%s\n",SDL_GetError());
    return SDL_APP_FAILURE;
  }
  app->stream = stream;

  app->n = 2;
  app->balles = (Balle*)malloc(sizeof(*app->balles) * app->n);
  Balle balle0 = {WIDTH / 2.0, 50, 100, 800, 100};
  Balle balle1 = {WIDTH / 4.0, 50, 60, 1200, 0};
  app->balles[0] = balle0;
  app->balles[1] = balle1;
  
  SDL_SetRenderDrawColor(app->ren, 0, 0, 0, SDL_ALPHA_OPAQUE);

  app->fixed_dt = 1.0 / 60.0; // 60 Hz physics
  app->accumulator = 0.0;
  app->sim_time = 0.0;
  app->last = clock::now();

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
  using clock = std::chrono::steady_clock;
  using sec   = std::chrono::duration<double>;

  App *app = (App*)appstate;

  auto now = clock::now();
  sec frame_elapsed = now - app->last;
  app->last = now;
  double frame_dt = frame_elapsed.count();
  
  const double max_frame_dt = 0.25;

  if (frame_dt > max_frame_dt) frame_dt = max_frame_dt;

  SDL_SetRenderDrawColor(app->ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(app->ren);

  app->accumulator += frame_dt;
  while (app->accumulator >= app->fixed_dt)
  {
    UpdateBallsPhysics(app, app->fixed_dt);
    app->accumulator -= app->fixed_dt;
    app->sim_time += app->fixed_dt;
  }

  for (int i = 0; i < app->n; ++i)
  {
    DrawBall(app->ren, app->balles[i]);
  }

  SDL_RenderPresent(app->ren);
  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
