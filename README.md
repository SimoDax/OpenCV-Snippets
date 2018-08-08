# OpenCV-Snippets
A collection of OpenCV projects I made

To compile these solutions you need to tell the linker where the OpenCV libraries are located, so I uploaded my configuration file opencv_debug.props. Note that I use OpenCV 3.4.0, **linked statically**

Stuff in here:
<br>

+ **ShapeCV:** Heuristic edge-based object detection, no AI/machine learning involved. <br><br> How it works: computes the gradient magnitude for every pixel of the image (that is, how 'fast' the color changes just before and after that pixel). The gradient is then thresholded to get a binary image of edges (This is called [Canny edge detection](http://opencv-python-tutroals.readthedocs.io/en/latest/py_tutorials/py_imgproc/py_canny/py_canny.html)), which are then 'read' from the image and stored in arrays of pixels. The relevant ones are drawn in a new image (contours) and then converted to poly lines using the [Ramer-Douglas-Peucker algorithm](http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm). The various zones delimited by the contours are then flood-filled to highlight the different objects in the image.
