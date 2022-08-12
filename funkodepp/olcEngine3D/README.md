# olcEngine3D

## Introduction

A simple 3D graphics engine in C++, based on the tutorial named "Code-It-Yourself 3D Graphics Engine in C++"
by YouTube content creator javidx9.

In the tutorial, javidx9 starts off deriving the necessary equations for projecting and transforming
the coordinates of the visualized object's vertices by defining a left-handed coordinate system. Although
this appears to be the standard for 3D computer graphics (and after having thought about it a bit more,
I can see why), for me this is already not an intuitive way to start thinking about this problem, since
I've worked with right-handed coordinate systems where the z-axis represents "up" practically my entire
professional life.

After following the tutorial to the letter and making some trial-and-error adjustments to at least make
the world space coordinate system appear right-handed with an upwards-pointing z-axis, I managed to make
it work as I would intuitively expect but I didn't know *why* it worked. So I decided to derive all the
equations myself, starting from the assumption that the coordinate systems in local, world, camera and
projected space are all right-handed. In the first three of those spaces, the x-axis points forwards, the
y-axis points to the left and the z-axis points up.

On top of that I wanted to add the concept of using local coordinate systems and coordinate system
transformations to transform the vertices of an object from one space to another. I think this is a very
intuitive method for positioning and orienting objects in your world without having to worry about the
combination and order of the individual rotations and translations necessary to achieve the desired result.
Especially when using 3D models, which are usually created and exported with their vertices relative to a
local coordinate system, I think this method is very useful.

In the end I managed to do derive all the necessary equations, but not before filling several A4 sheets
with mathematical equations and hand-written examples to test the results. In order to not lose all this
precious theory when I inevitably trash the paper once I'm sick of having to deal with it lying on my desk,
I decided to first transfer all the knowledge contained on its surface within the README of this project.
The result is the "Theory" section below. I hope that I have presented the concepts and maths in a way that
is understandable to those who wish to learn about the basic projection algorithms of a 3D graphics engine
when starting from right-handed coordinate systems.

If you have any questions or see an error in what I have written down below, feel free to contact me.

## Theory

### Goal

We want to look at an object which can be arbitrarily positioned and oriented anywhere in the world
with a camera which can likewise be positioned and oriented anywhere else in the world, and project
what the camera sees onto a computer screen.

Achieving this goal requires a lot of math, most of which can be presented in the form of multiplying
a transformation matrix with a vector to obtain the coordinates of that vector relative to another space.

### From local space to world space

Suppose that we have an object to which a local coordinate system is attached. The origin $O$ of that
local coordinate system is at the center of the object, the local x-axis $\mathbf{x_l}$ points forwards,
the local y-axis $\mathbf{y_l}$ points to the left and the local z-axis $\mathbf{x_l}$ points up. When
we know the coordinates of that object's vertices relative to that local coordinate system, then we say
that we know the coordinates in "local space". When we know the coordinates of that object's vertices
relative to the world's coordinate system, then we say that we know the coordinates in "world space".
But how do we transform the coordinates from local space to world space?

For simplicity's sake we will start with deriving the equations to tranform the coordinates from local
space to world space in 2D. Afterwards we can easily expand the derived equations to 3D simply by adding
an additional axis to the coordinate systems and by adding an extra coordinate to the vertices.

We start with an object in 2D, defind by a set of vertices which are connected by lines. Suppose we kow
that one of the object's vertices has the following coordinates in local space:

$$P_l = \begin{bmatrix} {P_l}_x \cr {P_l}_y \end{bmatrix}$$

In world space, that same vertex has the following unknown coordinates:

$$P_w = \begin{bmatrix} {P_w}_x \cr {P_w}_y \end{bmatrix}$$

We can unambiguously define the position and orientation of the local coordinate system in world space
by representing its origin as a point in world space and its axes as vectors in world space. In other
words, we also know the coordinates of the local coordinate system's origin $O_l$ (point) and axes
$\mathbf{x_l}$ and $\mathbf{y_l}$ (vectors) relative to world space:

$$O_l = \begin{bmatrix} {O_l}_x \cr {O_l}_y \end{bmatrix},
\mathbf{x_l} = \begin{bmatrix} {x_l}_x \cr {x_l}_y \end{bmatrix},
\mathbf{y_l} = \begin{bmatrix} {y_l}_x \cr {y_l}_y \end{bmatrix}$$

We can draw this configuration as follows:

> **Warning**
>
> Add figure

Based on this figure we can see that, relative to world space, the point $P_w$ can be written as
the following sum of vectors:

$$P_w = O_l + {P_l}_x \cdot \mathbf{x_l} + {P_l}_y \cdot \mathbf{y_l}$$

When we decompose this into its individual $x$ and $y$ components:

$${P_w}_x = {P_l}_x \cdot {x_l}_x + {P_l}_y \cdot {y_l}_x + {O_l}_x$$

$${P_w}_y = {P_l}_x \cdot {x_l}_y + {P_l}_y \cdot {y_l}_y + {O_l}_y$$

Then we see that we can write these equations as a multiplication of a matrix with a vector:

> **Note**
>
> We've added a fictional third component to each vector, which is necessary for the matrix
> multiplication to make sense. In 2D, the value of this fictional third vector component is always
> $1$.

The matrix in the equation above is the transformation matrix for transforming coordinates from local
space to world space. If you look at the upper left 2x2 section of the matrix, you will see that it
simply contains the components of the local coordinate system's x-axis relative to world space in its
first column and the components of the local coordinate system's y-axis relative to world space in its
second column. This is the rotational component of the transformation matrix. Likewise, the top two
rows of the last column contain the components of the local coordinate system's origin relative to
world space. This is the translational component of the transformation matrx. The bottom row always
consists of zeros except the last column, which is always $1$.

As you can see, this matrix is very simple to compose if you know how the local coordinate system is
defined relative to world space. In other words, once you know how you want to position and orient the
object in the world, you can easily transform its coordinates from local space to world space. All you
need is a few points and/or vectors in world space to define the origin and axes of the local coordinate
system attached to the object. Using local coordinate systems attached to each of your objects, you
can intuitively position, orient and move each of them relative to the world and to each other.

To get the equivalent equation in 3D, we can simply extend the vectors and transformation matrix with
an additional coordinate and axis as follows:
