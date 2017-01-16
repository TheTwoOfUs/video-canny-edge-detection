#!/usr/bin/python

import sys
from PIL import Image

images_omp = map(Image.open, ['omp.png', 'omp_scalability.png', 'omp_efficiency.png', 'omp_frame_time.png'])
images_mpi = map(Image.open, ['mpi.png', 'mpi_scalability.png', 'mpi_efficiency.png', 'mpi_frame_time.png'])
images_pthreads = map(Image.open, ['pthreads.png', 'pthreads_scalability.png', 'pthreads_efficiency.png', 'pthreads_frame_time.png'])
images_mpi_omp = map(Image.open, ['mpi_omp.png', 'mpi_omp_scalability.png', 'mpi_omp_efficiency.png', 'mpi_omp_frame_time.png'])

def combine(images, filename):
    widths, heights = zip(*(i.size for i in images))

    total_width = sum(widths) / 2
    max_height = max(heights) * 2

    new_im = Image.new('RGB', (total_width, max_height))

    x_offset = 0
    y_offset = 0
    for i, im in enumerate(images):
        new_im.paste(im, (x_offset, y_offset))
        x_offset += im.size[0]

        if i % 2 == 1:
            x_offset = 0
            y_offset += im.size[1]

    new_im.save(filename)

def main():
    combine(images_omp, 'omp_combined.png')
    combine(images_mpi, 'mpi_combined.png')
    combine(images_pthreads, 'pthreads_combined.png')
    combine(images_mpi_omp, 'mpi_omp_combined.png')

if __name__ == '__main__':
    main()
