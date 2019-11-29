# Data Compression. LZW v. Huffman Coding

## Prerequisites

This project uses advanced concepts of modern C++ programming. The structure is maintained using the
Bazel build system. Some of the functinality is being actively tested using unit tests written with
support of the Google Test.

The development environment for this project runs Arch Linux with the following software versions:

| Product | Version |
|---------|---------|
| `git`   | `v2.24` |
| `bazel` | `1.2.0` |
| `gcc`   | `9.2`   |

## Structure

The project is aimed to support extensibility. It is composed of a utilities library, `//lib/utils`
which provides high-level components for byte/unaligned storage. The code makes heavy usage of
templates in order to support true type safety among all the code components and as much static
analysis.

The compression algorithms are inside the `//lib/compression` library. The entry point of this
library is the `compression::Compressor<Algo>` template class. It uses policy templates to expose
a common interface for all the implemented algorithms: LZW and Huffman Coding.

The application provides CLI applications for compressing/decompressing files in the `//demo`
component:

    $ bazel-bin/demo/lzw_<compress|decompress> <input_file> <output_file>
    $ bazel-bin/demo/huffman_<encode|decode> <input_file> <output_file>

## Build and Run

In order to build and run the application, one must retrieve the entire source and run the following
bash commands:

```bash
$ bazel build //demo:all
```

To compile with optimizations, the flag `--cxxopts="-O2"` must be appended to the build command.

This will compile and create the CLI utilities mentioned above.

In order to run the available unit tests, one must run the following command:

```bash
$ bazel test //...
```

After having the CLI utilities built, one can run the `check_<algorithm>.sh` scripts. The scripts
compress and decompress the file given as argument. They display the compression rate and other
useful information (such as the time taken by the algorithm) and check the integrity of the
decompressed file against the original file. Note that the compression and decompression are
performed on temporary files. These temporary files are displayed at the beginning of the script and
they are not removed once the check script ends, for debugging purposes.

## Metrics and Further Improvements

Advanced benchmarks, compression statistics, memory footprints of the algorithms shall be provided
at the next Stage of the project, included in the final documentation. 
