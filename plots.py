#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

numthreads = np.array([1, 2, 4, 6, 8, 10, 12, 14, 16])

total_time_serial = 941.751
frame_time_serial = 7.037056

total_time_omp = np.array([941.751, 520.624, 315.977, 258.002, 237.726, 201.365, 189.464, 180.272, 213.566])
frame_time_omp = np.array([7.037056, 3.702026, 2.065648, 1.553605, 1.426003, 1.105929, 0.998395, 0.903751, 0.955066])
total_time_omp_scalability = total_time_serial / total_time_omp
total_time_omp_efficiency = total_time_omp_scalability / numthreads

total_time_mpi = np.array([941.751, 517.055, 360.12, 244.75, 200.419, 170.603, 157.034, 146.127, 158.261])
frame_time_mpi = np.array([7.037056, 3.628033, 2.256552, 1.382628, 1.151234, 0.815097, 0.707006, 0.618487, 0.636365])
total_time_mpi_scalability = total_time_serial / total_time_mpi
total_time_mpi_efficiency = total_time_mpi_scalability / numthreads

total_time_pthreads = np.array([941.751, 571.262, 309.834, 225.784, 188.145, 168.175, 151.594, 147.202, 136.023])
frame_time_pthreads = np.array([7.037056, 3.943, 1.925, 1.293, 0.998, 0.829, 0.709, 0.649, 0.588])
total_time_pthreads_scalability = total_time_serial / total_time_pthreads
total_time_pthreads_efficiency = total_time_pthreads_scalability / numthreads

total_time_mpi_omp = np.array([941.751, 627.573, 338.469, 288.16, 281.583, 286.568, 298.561, 313.241, 338.822])
frame_time_mpi_omp = np.array([7.037, 4.070, 1.795, 1.345, 1.244, 1.210, 1.264, 1.361, 1.439])
total_time_mpi_omp_scalability = total_time_serial / total_time_mpi_omp
total_time_mpi_omp_efficiency = total_time_mpi_omp_scalability / numthreads

TYPE_NONE = 0
TYPE_SCALABILITY = 1
TYPE_EFFICIENCY = 2
TYPE_FRAME_TIME = 3

TITLE_OMP = 'OpenMP'
TITLE_MPI = 'Open MPI'
TITLE_PTHREADS = 'pthreads'
TITLE_MPI_OMP = 'Open MPI + omp'

SUBTITLE_TOTAL_TIME = 'Timp Total de Executie'
SUBTITLE_SCALABILITY = 'Scalabilitatea'
SUBTITLE_EFFICIENCY = 'Eficienta'
SUBTITLE_FRAME_TIME = 'Timp de Executie pe Frame'

plots = [
    [total_time_omp, TITLE_OMP, TYPE_NONE, 'omp.png'],
    [total_time_mpi, TITLE_MPI, TYPE_NONE, 'mpi.png'],
    [total_time_pthreads, TITLE_PTHREADS, TYPE_NONE, 'pthreads.png'],
    [total_time_mpi_omp, TITLE_MPI_OMP, TYPE_NONE, 'mpi_omp.png'],

    [total_time_omp_scalability, TITLE_OMP, TYPE_SCALABILITY, 'omp_scalability.png'],
    [total_time_mpi_scalability, TITLE_MPI, TYPE_SCALABILITY, 'mpi_scalability.png'],
    [total_time_pthreads_scalability, TITLE_PTHREADS, TYPE_SCALABILITY, 'pthreads_scalability.png'],
    [total_time_mpi_omp_scalability, TITLE_MPI_OMP, TYPE_SCALABILITY, 'mpi_omp_scalability.png'],

    [total_time_omp_efficiency, TITLE_OMP, TYPE_EFFICIENCY, 'omp_efficiency.png'],
    [total_time_mpi_efficiency, TITLE_MPI, TYPE_EFFICIENCY, 'mpi_efficiency.png'],
    [total_time_pthreads_efficiency, TITLE_PTHREADS, TYPE_EFFICIENCY, 'pthreads_efficiency.png'],
    [total_time_mpi_omp_efficiency, TITLE_MPI_OMP, TYPE_EFFICIENCY, 'mpi_omp_efficiency.png'],

    [frame_time_omp, TITLE_OMP, TYPE_FRAME_TIME, 'omp_frame_time.png'],
    [frame_time_mpi, TITLE_MPI, TYPE_FRAME_TIME, 'mpi_frame_time.png'],
    [frame_time_pthreads, TITLE_PTHREADS, TYPE_FRAME_TIME, 'pthreads_frame_time.png'],
    [frame_time_mpi_omp, TITLE_MPI_OMP, TYPE_FRAME_TIME, 'mpi_omp_frame_time.png']
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
    elif type == TYPE_EFFICIENCY:
        plt.ylabel('E(n) = S(n) / n')
        plt.title(title + ' - ' + SUBTITLE_EFFICIENCY)
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
        mpi_omp = plt.plot(numthreads, total_time_mpi_omp, label='mpi-omp')

        plt.xlabel('Numar thread-uri')
        plt.ylabel('Timp (s)')
        plt.legend()
        plt.title('Comparatie')


        plt.savefig("comparision.png")

if __name__ == '__main__':
    main()
