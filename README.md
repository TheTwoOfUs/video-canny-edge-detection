## Canny Edge Detection on Video Files

### Description

Parallel implementation of [Canny edge detection](https://en.wikipedia.org/wiki/Canny_edge_detector) on MPG video files using different C parallel programming models such as [OpenMP](https://en.wikipedia.org/wiki/OpenMP), [Open MPI](https://en.wikipedia.org/wiki/Open_MPI), [Pthreads](https://en.wikipedia.org/wiki/POSIX_Threads). The serial Canny edge detection code was taken from [here](https://rosettacode.org/wiki/Canny_edge_detector#C) and adapted to our needs.

### Dependencies

* ffmpeg
* ffmpeg-devel
* ffmpeg-libs

To install the dependencies on your **Fedora** system:

```
sudo dnf install -y ffmpeg ffmpeg-libs ffmpeg-devel
```

To install the dependencies on `fep.grip.pub.ro`, run `./setup_fep.sh` from the root directory.

### Build & Run

1. Build `libde`:
    ```
    cd libde && make && cd ..
    ```

2. Build and run your desired implementation, e.g. for Pthreads:
    ```
    cd pthreads && make
    ./pthreads <IN.mpg> <NUM> [OUT.mpg]
    ```

Note: use `make fep` instead of `make` in case of building on `fep.grip.pub.ro`.

### Team members
- Ivașcu Gabriel-Cristian
- Radu Iulian-Gabriel
