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
