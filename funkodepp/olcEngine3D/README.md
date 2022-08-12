# olcEngine3D

A simple 3D graphics engine in C++, based on the tutorial named "Code-It-Yourself 3D Graphics Engine in C++"
by YouTube content creator javidx9.

## Theory

### Introduction

We want to look at an object which can be arbitrarily positioned and oriented anywhere in the world
with a camera which can likewise be positioned and oriented anywhere else in the world, and project
what the camera sees onto a computer screen.

Achieving this goal requires a lot of math, most of which can be presented in the form of multiplying
a transformation matrix with a vector to obtain the coordinates of that vector in a transformed space.

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
\mathbf{x_l} = \begin{bmatrix} \mathbf{x_l}_x \cr \mathbf{x_l}_y \end{bmatrix},
\mathbf{y_l} = \begin{bmatrix} \mathbf{y_l}_x \cr \mathbf{y_l}_y \end{bmatrix}$$

We can draw this configuration as follows:

> **Warning**
>
> Add figure

Based on this figure we can see that, relative to world space, the point $P_w$ can be written as
the following sum of vectors:

$$P_w = O_l + {P_l}_x \cdot \mathbf{x_l} + {P_l}_y \cdot \mathbf{y_l}$$

When we decompose this into its individual $x$ and $y$ components:

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
