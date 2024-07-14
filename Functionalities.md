# Application Functionalities

![Snímka obrazovky (319)](https://github.com/user-attachments/assets/771552ad-3379-450b-a0ef-ec8af626422c)

## Drawing Shapes:
- *Line* defined by two points.
- *Rectangle* defined by two points.
- *Polygon* defined by n points.
- *Circle* defined by two points.
- *Bézier curve* defined by n points.

## Rendering Order:
- Each newly drawn shape is rendered 'above' the previous one (each shape has a depth value for the Z-buffer).

## Shape Filling:
- Depending on UI choice, shapes can be **filled** or **unfilled** (applies only to closed polygons).

## Color Settings:
- The *boundary* and *fill* color are set globally but can be changed individually for each shape after drawing.

## Shape List:
- After drawing, shapes are added to a list of shapes.
- The *list of drawn shapes* is visible to the user in the UI as a **list of layers**, with one object per layer.

## Layer Selection:
- After selecting a *layer*, the user can:
  - **Move**, **scale**, and **rotate**(defining an angle is necessary) the shape.
  - Define a different *depth value* for the **Z-buffer** (move the shape *above/below* others).

## Saving and Loading:
- The current state of the program (shapes in layers, their colors, depth values, and canvas size) can be saved in the *.csv format*.
- The saved file can be loaded to continue working with the program.

## Rasterization Algorithms Used:
- Bresenham's algorithm for rasterizing all line segments.
- Bresenham's algorithm for rasterizing circles.
- Scan-line algorithm for filling shapes.
- Cyrus-Beck and Sutherland-Hodgman algorithms for clipping (clipping circles is not required).
- Z-buffer algorithm for visibility resolution.
