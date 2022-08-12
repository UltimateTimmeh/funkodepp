# olcEngine3D

A simple 3D graphics engine in C++, based on the tutorial named "Code-It-Yourself 3D Graphics Engine in C++"
by YouTube content creator javidx9.

## Theory

### Introduction

We want to look at an object which can be arbitrarily positioned and oriented anywhere in world space
with a camera which can likewise be positioned and oriented anywhere else in world space, and project
what the camera sees onto a computer screen.

Achieving this goal requires a lot of math, most of which can be presented in the form of multiplying
a transformation matrix with a vector to obtain the coordinates of that vector in a transformed space.

### From local space to world space

Suppose that we have an object to which a local coordinate system is attached. The origin is at the
center of the object, the local x-axis points forwards, the local y-axis points to the left and the
local z-axis points up. When we know the coordinates of that object's vertices relative to that local coordinate
system, then  we say that "we have the coordinates in local space".

For simplicity's sake we will start deriving the equations to tranform the coordinates from local
space to world space in 2D. At the end, we can easily expand the derived equations in 3D simply
by adding an additional axis.

We start with an object in 2D, and we knkow that one of that object's vertices has the following
coordinates in local space:

$$P'=\begin{bmatrix}x' \cr y'\end{bmatrix}$$
