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

We have an object to which a local coordinate system is attached. The origin is at the center of the
object, the local x-axis points forwards, the local y-axis points to the left and the local z-axis
points up. We know the coordinates of that object's vertices in local space. Suppose that one of those
vertices has the following local coordinates:

$$\begin{bmatrix}a & b \cr c & d\end{bmatrix}$$

The object can be positioned and oriented in world space by stating the coordinates of the local
coordinate system's origin and axes in world space.
