#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

numthreads = np.array([2, 4, 6, 8, 10, 12, 14, 16])

total_time_serial = 253.321
frame_time_serial = 9.531

total_time_omp = np.array([150.229, 85.085, 64.483, 59.873, 55.114, 51.883, 48.867, 46.285])
frame_time_omp = np.array([5.409, 2.829, 2.013, 1.825, 1.689, 1.503, 1.383, 1.285])
total_time_omp_scalability = total_time_serial / total_time_omp
total_time_omp_eficiency = total_time_omp_scalability / numthreads

total_time_mpi = np.array([134.264, 72.764, 53.565, 51.979, 50.596, 48.102, 46.723, 44.793])
frame_time_mpi = np.array([4.792, 2.350, 1.578, 1.414, 1.334, 1.298, 1.272, 1.151])
total_time_mpi_scalability = total_time_serial / total_time_mpi
total_time_mpi_eficiency = total_time_mpi_scalability / numthreads

total_time_pthreads = np.array([149.626, 81.725, 58.117, 48.046, 46.852, 43.075, 41.266, 40.074])
frame_time_pthreads = np.array([5.359, 2.644, 1.716, 1.427, 1.391, 1.359, 1.282, 1.196])
total_time_pthreads_scalability = total_time_serial / total_time_pthreads
total_time_pthreads_eficiency = total_time_pthreads_scalability / numthreads

TYPE_NONE = 0
TYPE_SCALABILITY = 1
TYPE_EFICIENCY = 2
TYPE_FRAME_TIME = 3

TITLE_OMP = 'OpenMP'
TITLE_MPI = 'Open MPI'
TITLE_PTHREADS = 'pthreads'

SUBTITLE_TOTAL_TIME = 'Timp Total de Executie'
SUBTITLE_SCALABILITY = 'Scalabilitatea'
SUBTITLE_EFICIENCY = 'Eficienta'
SUBTITLE_FRAME_TIME = 'Timp de Executie pe Frame'

plots = [
    [total_time_omp, TITLE_OMP, TYPE_NONE, 'omp.png'],
    [total_time_mpi, TITLE_MPI, TYPE_NONE, 'mpi.png'],
    [total_time_pthreads, TITLE_PTHREADS, TYPE_NONE, 'pthreads.png'],

    [total_time_omp_scalability, TITLE_OMP, TYPE_SCALABILITY, 'omp_scalability.png'],
    [total_time_mpi_scalability, TITLE_MPI, TYPE_SCALABILITY, 'mpi_scalability.png'],
    [total_time_pthreads_scalability, TITLE_PTHREADS, TYPE_SCALABILITY, 'pthreads_scalability.png'],

    [total_time_omp_eficiency, TITLE_OMP, TYPE_EFICIENCY, 'omp_eficiency.png'],
    [total_time_mpi_eficiency, TITLE_MPI, TYPE_EFICIENCY, 'mpi_eficiency.png'],
    [total_time_pthreads_eficiency, TITLE_PTHREADS, TYPE_EFICIENCY, 'pthreads_eficiency.png'],

    [frame_time_omp, TITLE_OMP, TYPE_FRAME_TIME, 'omp_frame_time.png'],
    [frame_time_mpi, TITLE_MPI, TYPE_FRAME_TIME, 'mpi_frame_time.png'],
    [frame_time_pthreads, TITLE_PTHREADS, TYPE_FRAME_TIME, 'pthreads_frame_time.png']
]

def create_plot(fig_index, time, title, type, filename):
    fig = plt.figure(fig_index)
    ax = fig.add_subplot(111)

    plt.plot(numthreads, time)

    for i, xy in enumerate(zip(numthreads, time)):
        ax.annotate('%.3f' % time[i], xy=xy, textcoords='data')

    plt.xlabel('Numar thread-uri')
    if type == TYPE_NONE:
        plt.ylabel('Timp (s)')
        plt.title(title + ' - ' + SUBTITLE_TOTAL_TIME)
    elif type == TYPE_SCALABILITY:
        plt.ylabel('S(n) = T(1) / T(n)')
        plt.title(title + ' - ' + SUBTITLE_SCALABILITY)
    elif type == TYPE_EFICIENCY:
        plt.ylabel('E(n) = S(n) / n')
        plt.title(title + ' - ' + SUBTITLE_EFICIENCY)
    elif type == TYPE_FRAME_TIME:
        plt.ylabel('Timp (s)')
        plt.title(title + ' - ' + SUBTITLE_FRAME_TIME)

    plt.grid(True)
    plt.savefig(filename)

def main():
    for i, plot in enumerate(plots):
        create_plot(i+1, plot[0], plot[1], plot[2], plot[3])
    else:
        fig = plt.figure(i+2)
        ax = fig.add_subplot(111)

        omp = plt.plot(numthreads, total_time_omp, label='OpenMP')
        plt.hold(True)
        mpi = plt.plot(numthreads, total_time_mpi, label='Open MPI')
        pthreads = plt.plot(numthreads, total_time_pthreads, label='pthreads')

        plt.xlabel('Numar thread-uri')
        plt.ylabel('Timp (s)')
        plt.legend()
        plt.title('Comparatie')


        plt.savefig("comparision.png")

if __name__ == '__main__':
    main()
