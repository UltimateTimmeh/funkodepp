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

For simplicity's sake we will start deriving the equations to tranform the coordinates from local space
to world space in 2D. At the end, we can easily expand the derived equations to 3D simply by adding an
additional axis to the coordinate systems and coordinate to the vertices.

So we start with an object in 2D. Suppose we kow that one of that object's vertices has the following
coordinates in local space:

$$P_l=\begin{bmatrix}x_l \cr y_l\end{bmatrix}$$

In world space, that same vertex has the following coordinates:

$$P=\begin{bmatrix}x \cr y\end{bmatrix}$$

We can unambiguously define the position and orientation of the local coordinate system in world space
by representing its origin as a point in world space and its axes as vectors in world space. In other
words: we know the coordinates of the local coordinate system's origin $O$ (point) and axes (vectors)
in world space:

$$O_l = \begin{bmatrix} {O_l}_x \cr {O_l}_y \end{bmatrix},
\mathbf{x_l}=\begin{bmatrix}\mathbf{x_l}_x \cr \mathbf{x_l}_y\end{bmatrix},
\mathbf{y_l}=\begin{bmatrix}\mathbf{y_l}_x \cr \mathbf{y_l}_y\end{bmatrix}$$

We can draw this configuration as follows:

> **Note**
> Add figure

