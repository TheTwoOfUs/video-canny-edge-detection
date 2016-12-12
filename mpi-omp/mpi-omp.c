#include <assert.h>
#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libde/de.h"
#include "mpi.h"
#include "utils.h"

#define MAX_BRIGHTNESS 255
#define TAG_WORK       42
#define TAG_SIZE       43
#define BUFFSIZE       16777216 // 16 MiB

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
            const bool     normalize,
            const int      nthreads)
{
  const int khalf = kn / 2;
  float min = 0.5;
  float max = 254.5;
  float pixel = 0.0;
  size_t c = 0;
  int m, n, i, j;

  assert(kn % 2 == 1);
  assert(nx > kn && ny > kn);

  #pragma omp parallel for private(m, n, pixel, c, i, j) shared(out, min, max) num_threads(nthreads) collapse(2)
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
                const float    sigma,
                const int      nthreads)
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

  convolution(in, out, kernel, nx, ny, n, true, nthreads);
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
                     const float    sigma,
                     const int      nthreads)
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

  gaussian_filter(pixels, out, width, height, sigma, nthreads);

  convolution(out, after_Gx, Gx, width, height, 3, false, nthreads);

  convolution(out, after_Gy, Gy, width, height, 3, false, nthreads);

  #pragma omp parallel for private(i, j) shared(G) num_threads(nthreads) collapse(2)
  for (i = 1; i < width - 1; i++) {
    for (j = 1; j < height - 1; j++) {
      const int c = i + width * j;
      G[c] = (pixel_t)hypot(after_Gx[c], after_Gy[c]);
    }
  }

  /* Non-maximum suppression, straightforward implementation. */
  #pragma omp parallel for private(i, j) shared(nms) num_threads(nthreads) collapse(2)
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
  fprintf(stderr, "Usage: mpirun -np <NUM> %s <IN_FILE> [OUT_FILE]\n", argv0);
  fprintf(stderr, "Required arguments:\n"
                  "  <IN_FILE>\tthe input video file\n"
                  "Optional arguments:\n"
                  "  [OUT_FILE]\t\tthe output video file\n");
}

static bool
is_power_of_two(const int x)
{
  return ((x != 0) && (x != 1) && !(x & (x - 1)));
}

static bool
is_perfect_square(const int x)
{
  int root = sqrt(x);

  return root * root == x;
}

static void
frame_get_block(const uint8_t *frame,
                const int      width,
                const int      y_id,
                const int      x_id,
                const int      y_step,
                const int      x_step,
                uint8_t       *block)
{
  int i, j, k = 0;

  for (i = y_id * y_step; i < (y_id + 1) * y_step; i++) {
    for (j = x_id * x_step; j < (x_id + 1) * x_step; j++)
      block[k++] = frame[i * width + j];
  }
}

static void
frame_set_block(uint8_t       *frame,
                const int      width,
                const int      y_id,
                const int      x_id,
                const int      y_step,
                const int      x_step,
                const uint8_t *block)
{
  int i, j, k = 0;

  for (i = y_id * y_step; i < (y_id + 1) * y_step; i++) {
    for (j = x_id * x_step; j < (x_id + 1) * x_step; j++)
      frame[i * width + j] = block[k++];
  }
}

int main(int argc, char **argv)
{
  const char *file_in;
  const char *file_out;

  int num_tasks, rank;
  int num_workers, master_id;
  MPI_Status status;

  uint8_t *buffer;
  int size_buffer[2];

  struct timespec start, end;
  double time_per_frame, computational_time = 0;

  if (argc < 2 || argc > 3) {
    print_usage (argv[0]);
    exit (1);
  }

  file_in = argv[1];
  file_out = argc == 3 ? argv[2] : "out.mpg";

  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &num_tasks);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

  master_id = num_workers = num_tasks - 1;
  assert(is_power_of_two(num_workers));

  buffer = calloc(BUFFSIZE, sizeof(uint8_t));

  if (rank == master_id) {
    DeContext *context;
    DeFrame *frame = NULL;
    int got_frame = 0;
    int x_blocks, y_blocks;
    int x_step, y_step;
    int i, j, worker_id;

    context = de_context_create (file_in);
    de_context_prepare_encoding (context, file_out);

    do {
      frame = de_context_get_next_frame (context, &got_frame);

      if (got_frame == -1) {
        size_buffer[0] = size_buffer[1] = 0;

        for (i = 0; i < num_workers; i++)
          MPI_Send (size_buffer, 2, MPI_INT, i, TAG_SIZE, MPI_COMM_WORLD);

        break;
      }

      if (got_frame && frame) {
        DIE(clock_gettime (CLOCK_MONOTONIC, &start) == -1, "clock_gettime");

        if (is_perfect_square(num_workers)) {
          x_blocks = y_blocks = (int)sqrt(num_workers);
        } else {
          x_blocks = 2;
          y_blocks = num_workers / 2;
        }

        x_step = frame->width / x_blocks;
        y_step = frame->height / y_blocks;

        size_buffer[0] = y_step;
        size_buffer[1] = x_step;

        /* Split the frame in blocks and send one block to each worker. */
        worker_id = 0;
        for (i = 0; i < y_blocks; i++) {
          for (j = 0; j < x_blocks; j++) {
            /* Send the block width and height. */
            MPI_Send(size_buffer, 2, MPI_INT, worker_id, TAG_SIZE, MPI_COMM_WORLD);

            /* Send the actual block. */
            frame_get_block(frame->data, frame->width, i, j, y_step, x_step, buffer);
            MPI_Send(buffer, x_step * y_step, MPI_UNSIGNED_CHAR, worker_id++, TAG_WORK, MPI_COMM_WORLD);
          }
        }

        /* Receive the computed blocks from workers. */
        for (i = 0; i < num_workers; i++) {
          MPI_Recv(buffer, x_step * y_step, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, TAG_WORK, MPI_COMM_WORLD, &status);
          worker_id = status.MPI_SOURCE;
          frame_set_block(frame->frame->data[0], frame->width, worker_id / x_blocks, worker_id % x_blocks, y_step, x_step, buffer);
        }

        DIE(clock_gettime (CLOCK_MONOTONIC, &end) == -1, "clock_gettime");

        time_per_frame = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
        printf("[%d] Time per frame: %lf\n", rank, time_per_frame);
        computational_time += time_per_frame;

        de_context_set_next_frame (context, frame);
      }
    } while (1);

    printf("[%d] Computational time: %lf\n", rank, computational_time);

    de_context_end_encoding (context);
  } else {
    int block_width, block_height;

    /* Receive frame blocks from master and apply canny edge detection on them. */
    while (true) {
      /* Receive the block's width and height. */
      MPI_Recv(size_buffer, 2, MPI_INT, master_id, TAG_SIZE, MPI_COMM_WORLD, &status);

      block_height = size_buffer[0];
      block_width = size_buffer[1];

      /* Job is done. */
      if (block_height == 0 && block_width == 0)
        break;

      /* Receive the actual block. */
      MPI_Recv(buffer, block_width * block_height, MPI_UNSIGNED_CHAR, master_id, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      /* Apply canny edge detection. */
      uint8_t *jarjar = canny_edge_detection(buffer, block_width, block_height, 45, 50, 1.0f, num_workers);

      /* Send the block back to master. */
      MPI_Send(jarjar, block_width * block_height, MPI_UNSIGNED_CHAR, master_id, TAG_WORK, MPI_COMM_WORLD);
    }
  }

  free(buffer);

  MPI_Finalize ();

  return 0;
}
