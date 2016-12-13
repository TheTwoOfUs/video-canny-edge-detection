#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libde/de.h"
#include "utils.h"

#define MAX_BRIGHTNESS 255
#define CANNY_LOWER 45
#define CANNY_UPPER 50
#define CANNY_SIGMA 1.0

/* Use short int instead unsigned char so that we can store negative values. */
typedef short int pixel_t;

/*
 * If normalize is true, then map pixels to range 0 -> MAX_BRIGHTNESS.
 */
static void
convolution(const pixel_t *in,
            pixel_t       *out,
            const float   *kernel,
            const int      nx,
            const int      ny,
            const int      kn,
            const bool     normalize)
{
  const int khalf = kn / 2;
  float min = 0.5;
  float max = 254.5;
  float pixel = 0.0;
  size_t c = 0;
  int m, n, i, j;

  assert(kn % 2 == 1);
  assert(nx > kn && ny > kn);

  for (m = khalf; m < nx - khalf; m++) {
    for (n = khalf; n < ny - khalf; n++) {
      pixel = c = 0;

      for (j = -khalf; j <= khalf; j++)
        for (i = -khalf; i <= khalf; i++)
          pixel += in[(n - j) * nx + m - i] * kernel[c++];

      if (normalize == true)
        pixel = MAX_BRIGHTNESS * (pixel - min) / (max - min);

      out[n * nx + m] = (pixel_t) pixel;
    }
  }
}

/*
 * gaussianFilter: http://www.songho.ca/dsp/cannyedge/cannyedge.html
 * Determine the size of kernel (odd #)
 * 0.0 <= sigma < 0.5 : 3
 * 0.5 <= sigma < 1.0 : 5
 * 1.0 <= sigma < 1.5 : 7
 * 1.5 <= sigma < 2.0 : 9
 * 2.0 <= sigma < 2.5 : 11
 * 2.5 <= sigma < 3.0 : 13 ...
 * kernel size = 2 * int(2 * sigma) + 3;
 */
static void
gaussian_filter(const pixel_t *in,
                pixel_t       *out,
                const int      nx,
                const int      ny,
                const float    sigma)
{
  const int n = 2 * (int) (2 * sigma) + 3;
  const float mean = (float) floor(n / 2.0);
  float kernel[n * n];
  int i, j;
  size_t c = 0;

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      kernel[c++] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) + pow((j - mean) / sigma, 2.0))) / (2 * M_PI * sigma * sigma);
  }

  convolution(in, out, kernel, nx, ny, n, true);
}

/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */

static uint8_t *
canny_edge_detection(const uint8_t *in,
                     const int      width,
                     const int      height,
                     const int      t1,
                     const int      t2,
                     const float    sigma)
{
  int i, j, k, nedges;
  int *edges;
  size_t t = 1;
  uint8_t *retval;

  const float Gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
  const float Gy[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

  pixel_t *G = calloc(width * height * sizeof(pixel_t), 1);
  DIE(G == NULL, "calloc");

  pixel_t *after_Gx = calloc(width * height * sizeof(pixel_t), 1);
  DIE(after_Gx == NULL, "calloc");

  pixel_t *after_Gy = calloc(width * height * sizeof(pixel_t), 1);
  DIE(after_Gy == NULL, "calloc");

  pixel_t *nms = calloc(width * height * sizeof(pixel_t), 1);
  DIE(nms == NULL, "calloc");

  pixel_t *out = malloc(width * height * sizeof(pixel_t));
  DIE(out == NULL, "malloc");

  pixel_t *pixels = malloc(width * height * sizeof(pixel_t));
  DIE(pixels == NULL, "malloc");

  /* Convert to pixel_t. */
  for (i = 0; i < width * height; i++) {
    pixels[i] = (pixel_t)in[i];
  }

  gaussian_filter(pixels, out, width, height, sigma);

  convolution(out, after_Gx, Gx, width, height, 3, false);

  convolution(out, after_Gy, Gy, width, height, 3, false);

  for (i = 1; i < width - 1; i++) {
    for (j = 1; j < height - 1; j++) {
      const int c = i + width * j;
      G[c] = (pixel_t)hypot(after_Gx[c], after_Gy[c]);
    }
  }

  /* Non-maximum suppression, straightforward implementation. */
  for (i = 1; i < width - 1; i++) {
    for (j = 1; j < height - 1; j++) {
      const int c = i + width * j;
      const int nn = c - width;
      const int ss = c + width;
      const int ww = c + 1;
      const int ee = c - 1;
      const int nw = nn + 1;
      const int ne = nn - 1;
      const int sw = ss + 1;
      const int se = ss - 1;
      const float dir = (float) (fmod(atan2(after_Gy[c], after_Gx[c]) + M_PI, M_PI) / M_PI) * 8;

      if (((dir <= 1 || dir > 7) && G[c] > G[ee] && G[c] > G[ww]) || // 0 deg
          ((dir > 1 && dir <= 3) && G[c] > G[nw] && G[c] > G[se]) || // 45 deg
          ((dir > 3 && dir <= 5) && G[c] > G[nn] && G[c] > G[ss]) || // 90 deg
          ((dir > 5 && dir <= 7) && G[c] > G[ne] && G[c] > G[sw]))   // 135 deg
        nms[c] = G[c];
      else
        nms[c] = 0;
    }
  }

  /* Reuse the array used as a stack, width * height / 2 elements should be enough. */
  edges = (int *) after_Gy;
  memset(out, 0, sizeof(pixel_t) * width * height);
  memset(edges, 0, sizeof(pixel_t) * width * height);

  /* Tracing edges with hysteresis. Non-recursive implementation. */
  for (j = 1; j < height - 1; j++) {
    for (i = 1; i < width - 1; i++) {
      /* Trace edges. */
      if (nms[t] >= t2 && out[t] == 0) {
        out[t] = MAX_BRIGHTNESS;
        nedges = 1;
        edges[0] = t;

        do {
          nedges--;
          const int e = edges[nedges];

          int nbs[8]; // neighbours
          nbs[0] = e - width;     // nn
          nbs[1] = e + width;     // ss
          nbs[2] = e + 1;      // ww
          nbs[3] = e - 1;      // ee
          nbs[4] = nbs[0] + 1; // nw
          nbs[5] = nbs[0] - 1; // ne
          nbs[6] = nbs[1] + 1; // sw
          nbs[7] = nbs[1] - 1; // se

          for (k = 0; k < 8; k++) {
            if (nms[nbs[k]] >= t1 && out[nbs[k]] == 0) {
              out[nbs[k]] = MAX_BRIGHTNESS;
              edges[nedges] = nbs[k];
              nedges++;
            }
          }
        } while (nedges > 0);
      }
      t++;
    }
  }

  retval = malloc(width * height * sizeof(uint8_t));
  DIE(retval == NULL, "malloc");

  /* Convert back to uint8_t */
  for (i = 0; i < width * height; i++) {
    retval[i] = (uint8_t)out[i];
  }

  free(after_Gx);
  free(after_Gy);
  free(G);
  free(nms);
  free(pixels);
  free(out);

  return retval;
}

static void
print_usage(const char *argv0)
{
  fprintf(stderr, "Usage: %s <IN.mpg> [OUT.mpg]\n", argv0);
  fprintf(stderr, "Required arguments:\n"
                  "  <IN.mpg>\tthe input video file\n"
                  "Optional arguments:\n"
                  "  [OUT.mpg]\tthe output video file\n");
}

int main(int argc, char **argv)
{
  const char *file_in;
  const char *file_out;

  DeContext *context;
  DeFrame *frame = NULL;
  int got_frame = 0;

  struct timespec start, end;
  double time_per_frame, computational_time = 0;

  if (argc < 2 || argc > 3) {
    print_usage(argv[0]);
    exit(1);
  }

  file_in = argv[1];
  file_out = argc == 3 ? argv[2] : "out.mpg";

  context = de_context_create(file_in);
  de_context_prepare_encoding(context, file_out);

  do {
    frame = de_context_get_next_frame(context, &got_frame);

    if (got_frame == -1)
      break;

    if (got_frame && frame) {
      DIE(clock_gettime(CLOCK_MONOTONIC, &start) == -1, "clock_gettime");
      frame->frame->data[0] = canny_edge_detection(frame->data, frame->width, frame->height,
                                                   CANNY_LOWER, CANNY_UPPER, CANNY_SIGMA);
      DIE(clock_gettime(CLOCK_MONOTONIC, &end) == -1, "clock_gettime");

      time_per_frame = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
      printf("Time per frame: %lf\n", time_per_frame);
      computational_time += time_per_frame;

      de_context_set_next_frame(context, frame);
    }
  } while (1);

  de_context_end_encoding(context);

  printf("Computational time: %lf\n", computational_time);

  return 0;
}
