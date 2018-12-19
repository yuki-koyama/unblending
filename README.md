# Unblending

Unblending is a C++ library for decomposing a target image into a set of semi-transparent layers associated with *advanced color-blend modes* (e.g., "multiply" and "color-dodge"). Output layers can be imported to Adobe Photoshop, Adobe After Effects, GIMP, Krita, etc. and are useful for performing complex edits that are otherwise difficult.

![An example of image decomposition. Input image courtesy of David Revoy.](./docs/images/teaser.gif)

- `unblending` is a library that provides the main functionality for the use from other programs.
- `unblending-cli` is an executable with a command line interface to use the main functionality.
- `unblending-gui` is an executable with a graphical user interface to use the main functionality with interactive specification of necessary input parameters.

## Publication

Yuki Koyama and Masataka Goto. 2018. Decomposing Images into Layers with Advanced Color Blending. Comput. Graph. Forum 37, 7, pp.397--407 (2018). (a.k.a. Proceedings of Pacific Graphics 2018) DOI: <https://doi.org/10.1111/cgf.13577>

This library is provided mainly for researchers who want to use or extend the method described in the above paper. We also expect that this library is useful for developers who plan to use the method in their software.

## Dependencies

- NLopt <https://nlopt.readthedocs.io/> (included as gitsubmodule)
- json11 <https://github.com/dropbox/json11> (included as gitsubmodule)
- nlopt-util <https://github.com/yuki-koyama/nlopt-util> (included as gitsubmodule)
- parallel-util <https://github.com/yuki-koyama/parallel-util> (included as gitsubmodule)
- timer <https://github.com/yuki-koyama/timer> (included as gitsubmodule)
- Eigen <http://eigen.tuxfamily.org/>
- Qt5 <https://www.qt.io/>

## Prerequisites

Eigen and Qt5 need to be installed beforehand. macOS users can use `brew` for this purpose: 
```
$ brew install eigen qt5
```

## Build Using CMake

Unblending is organized using `cmake` and is built by the following commands:
```
$ git clone https://github.com/yuki-koyama/unblending.git --recursive
$ mkdir build
$ cd build
$ cmake ../unblending
$ make
```
This process builds a static library (e.g., `libunblending.a`) and a command line interface (CLI) named `unblending-cli`.

The CLI can be used by the following command:
```
$ ./unblending-cli/unblending-cli [-o <path-to-output-dir>] [-w <target-image-width>] <path-to-input-image> <path-to-layer-info>
```
Note that `--help` will generate the following messages:
```
$ ./unblending-cli/unblending-cli --help
 - A command line interface (CLI) for using the ``unblending'' library.
Usage:
  unblending-cli [OPTION...] <input-image-path> <layer-infos-path>

  -o, --outdir arg            Path to the output directory (default: ./out)
  -w, --width arg             Target width (pixels) of the output image
                              (default: original resolution)
  -h, --help                  Print help
  -e, --explicit-mode-names   Append blend mode names to output image file
                              names
  -v, --verbose-export        Export intermediate files as well as final
                              outcomes
      --input-image-path arg  Path to the input image (png or jpg)
      --layer-infos-path arg  Path to the layer infos (json)
```

The GUI allows you to interactively specify necessary parameters. Currently the GUI is tested on macOS only (pull requests are highly appreciated).

![GUI. Input image courtesy of David Revoy.](./docs/images/gui.png)

## Build and Run Using Docker

If you use `docker`, you can easily build the CLI by `docker build`:
```
$ git clone https://github.com/yuki-koyama/unblending.git --recursive
$ cd unblending
$ docker build -t unblending-cli:latest .
```

Then, you can use the CLI by `docker run`: 
```
$ docker run --rm --volume $(pwd):/tmp -it unblending-cli:latest [-o <path-to-output-dir>] [-w <target-image-width>] <path-to-input-image> <path-to-layer-info>
```

For example, the following command generates results into a sub directory named `output`:
```
$ docker run --rm --volume $(pwd):/tmp -it unblending-cli:latest -o /tmp/output -w 120 /tmp/examples/magic.png /tmp/examples/magic.json
```
Note: this typically takes around 10--30 seconds with consumer-level laptops.

## `LayerInfos` File Structure

### Definition

The file should be written as a JSON file that includes a `LayerInfos` object.

`LayerInfos` is an array of `LayerInfo` objects, where formers correspond to lower layers.

`LayerInfo` is an unordered set of the following key/value pairs:

- "color_model": `ColorModel`
- "comp_op": `CompOp`
- "mode": `Mode`

`ColorModel` is an unordered set of the following key/value pairs:

- "primary_color": an array of three numbers corresponding to R, G, and B values
- "color_variance" : a single number that indicates variance or an array of nine numbers that indicates 3-by-3 covariance matrix

`CompOp` should be one of the followings:

- "source-over"
- "plus"

`Mode` shoulbe be one of the followings:

- "Normal"
- "Multiply"
- "Screen"
- "Overlay"
- "Darken"
- "Lighten"
- "ColorDodge"
- "ColorBurn"
- "HardLight"
- "SoftLight"
- "Difference"
- "Exclusion"
- "LinearDodge"

### Example

Here is an example of `input_layer_info` file (a JSON file):
```
[
  {
    "color_model": {
      "color_variance": 0.10,
      "primary_color": [0.131, 0.131, 0.212]
    },
    "comp_op": "source-over",
    "mode": "Normal"
  },
  {
    "color_model": {
      "color_variance": 0.015,
      "primary_color": [1.000, 0.566, 1.000]
    },
    "comp_op": "source-over",
    "mode": "HardLight"
  },
  {
    "color_model": {
      "color_variance": 0.10,
      "primary_color": [0.283, 0.121, 0.131]
    },
    "comp_op": "source-over",
    "mode": "Multiply"
  }
]
```

## Licensing

Unblending is dual-licensed; You may use this software under either *LGPLv3* or *our commercial license*. See the `LICENSE` files for details.

## Contributing

Pull requests are highly welcome. Please be aware that any contribution to this repository will be licensed under the above license condition.

## Authors

- Yuki Koyama
- Masataka Goto

## Copyright

Copyright (c) 2018 National Institute of Advanced Industrial Science and Technology (AIST) - <koyama.y@aist.go.jp>
