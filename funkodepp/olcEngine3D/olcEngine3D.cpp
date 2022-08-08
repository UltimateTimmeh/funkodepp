// olcEngine3D: Code-It-Yourself 3D Graphics Engine in C++, tutorial by javidx9.
//
// In this version I've derived the projection matrix myself such that it natively projects
// objects from camera space - where the camera has a local right-handed coordinate system with
// the X-axis pointing forward, the Y-axis pointing left and the Z-axis pointing up - to a
// normalized projection space with the width-vector pointing right and the height-vector pointing
// down (such that they align with the axes of the 2D screen space), and the depth-vector pointing
// forward.
//
// Visible objects fall within the bounding box of the 3D normalized projection space, which has
// corner nodes (-1, -1, 0) [i.e. top left front] and (1, 1, 1) [i.e. bottom right back].
//
// I also added a few extra things of my own:
//
//   - Controls along all six degrees of freedom, i.e. the three translational and three
//     rotational degrees of freedom), binding the keys with a main focus on comfortable
//     airplane-like controls.
//   - Coloring of the map depending on the height of the centroid of each triangle. The
//     lowest region is colored green to resemble grass, the middle region is colored gray
//     to resemble mountain sides and the highest region is colored white to resemble snow.
//   - A day/night cycle, in which the direction of the lighting rotates around the map
//     and the shading of the map's triangles depends on the "time of day".
//
// Some fun ideas I thought of that may be added in a later version:
//
//   - A visible sun which rotates around the map together with the currently invisible
//     light source. The triangles of the sun's mesh could also change color throughout the
//     day, with a white-ish yellow at noon and a mild orange at dusk/dawn.
//     It could perhaps rotate around the origin of the camera space in a plane perpendicular
//     to the plane in which the world's light source vector rotates to give the impression
//     that it is infinitely far away, and could be made to always face the player, such that it
//     appears spherical even though it is just a flat disk.
//     Some additional changes may have to be made to make sure the sun's illumination is
//     independent of the world's illumination.
//   - Randomly generated stars in a sphere around the player. The stars could be randomly
//     generated when the application is started. The stars should not be 3D objects drawn
//     at a great distance, but pixels drawn on a fictional 2D sphere around the world.
//     I'm thinking every star could be represented by a pair of angles, or maybe a vector relative
//     to world space would be easier, but I'm unsure how to translate that to the drawing of
//     pixels depending on the player's orientation. Probably the star vectors would have to be
//     transformed to camera space, and from there to normalized projected space and maybe
//     even screen space. Or maybe the angle of the vector with the camera space's XY and XZ
//     planes can be compared to the camera's FOV to determine whether a star is visible and
//     where it should be drawn. Either way, the effect should be that the stars only "move"
//     when the player performs a rotational movement, to give the impression that they are
//     infinitely far away.
//     Additionally, the stars should only be visible at night. During the day their color
//     should be either the same as the sky, and at night they should be white.
//   - Use the accompanied "axes.obj" model to give the player a sense of their orientation
//     relative to the world. This could perhaps be done by having the model stay in the
//     same position in camera space, but with its axes aligned with the axes of world space.
//     This may also require implementing some form of order in which objects are drawn,
//     separate from the current triangle depth sorting.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include <math.h>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

// Structs
struct vec3d
{
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float w = 1.0f;
};

struct mat4x4
{
  float m[4][4] = { 0.0f };
};

struct coordsys
{
  vec3d o;
  vec3d u;
  vec3d v;
  vec3d w;
};

struct triangle
{
  vec3d p[3];
  olc::Pixel fillColor;
  olc::Pixel wireColor;
};

struct mesh
{
  std::vector<triangle> tris;

  bool LoadFromObjectFile(std::string sFilename)
  {
    std::ifstream f(sFilename);
    if (!f.is_open())
      return false;

    // Local cache of verts
    std::vector<vec3d> verts;

    while (!f.eof())
    {
      char line[128];
      f.getline(line, 128);

      std::stringstream s;
      s << line;

      char junk;

      if (line[0] == 'v')
      {
        vec3d v;
        s >> junk >> v.x >> v.y >> v.z;
        verts.push_back(v);
      }

      if (line[0] == 'f')
      {
        int f[3];
        s >> junk >> f[0] >> f[1] >> f[2];
        tris.push_back({ verts[f[0]-1], verts[f[1]-1], verts[f[2]-1] });
      }
    }
    return true;
  }
};


// 3D vector operations
void Vec3d_Print(vec3d &v)
{
  std::cout << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w << std::endl;
}

vec3d Vec3d_Add(vec3d &v1, vec3d &v2)
{
  return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3d Vec3d_Sub(vec3d &v1, vec3d &v2)
{
  return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3d Vec3d_Mul(vec3d &vi, float s)
{
  vec3d vo;
  vo.x = vi.x * s; vo.y = vi.y * s; vo.z = vi.z * s; vo.w = vi.w * s;
  return vo;
}

vec3d Vec3d_Div(vec3d &vi, float s)
{
  vec3d vo;
  vo.x = vi.x / s; vo.y = vi.y / s; vo.z = vi.z / s; vo.w = vi.w / s;
  return vo;
}

float Vec3d_DotProduct(vec3d &v1, vec3d &v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vec3d_Length(vec3d &v)
{
  return sqrtf(Vec3d_DotProduct(v, v));
}

vec3d Vec3d_Normalize(vec3d &v)
{
  float l = Vec3d_Length(v);
  return { v.x / l, v.y / l, v.z / l };
}

vec3d Vec3d_CrossProduct(vec3d &v1, vec3d &v2)
{
  vec3d v;
  v.x = v1.y * v2.z - v1.z * v2.y;
  v.y = v1.z * v2.x - v1.x * v2.z;
  v.z = v1.x * v2.y - v1.y * v2.x;
  return v;
}

vec3d Vec3d_ApplyTransform(vec3d &vi, mat4x4 &m)
{
  vec3d vo;
  vo.x = m.m[0][0] * vi.x + m.m[0][1] * vi.y + m.m[0][2] * vi.z + m.m[0][3] * vi.w;
  vo.y = m.m[1][0] * vi.x + m.m[1][1] * vi.y + m.m[1][2] * vi.z + m.m[1][3] * vi.w;
  vo.z = m.m[2][0] * vi.x + m.m[2][1] * vi.y + m.m[2][2] * vi.z + m.m[2][3] * vi.w;
  vo.w = m.m[3][0] * vi.x + m.m[3][1] * vi.y + m.m[3][2] * vi.z + m.m[3][3] * vi.w;
  return vo;
}

vec3d Vec3d_WhereLineIntersectsPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd)
{
  plane_n = Vec3d_Normalize(plane_n);
  float plane_d = -Vec3d_DotProduct(plane_n, plane_p);
  float ad = Vec3d_DotProduct(lineStart, plane_n);
  float bd = Vec3d_DotProduct(lineEnd, plane_n);
  float t = (-plane_d - ad) / (bd - ad);
  vec3d lineStartToEnd = Vec3d_Sub(lineEnd, lineStart);
  vec3d lineStartToIntersect = Vec3d_Mul(lineStartToEnd, t);
  return Vec3d_Add(lineStart, lineStartToIntersect);
}


// 4x4 matrix operations
void Mat4x4_Print(mat4x4 &m)
{
  std::cout << m.m[0][0] << ' ' << m.m[0][1] << ' ' << m.m[0][2] << ' ' << m.m[0][3] << '\n'
            << m.m[1][0] << ' ' << m.m[1][1] << ' ' << m.m[1][2] << ' ' << m.m[1][3] << '\n'
            << m.m[2][0] << ' ' << m.m[2][1] << ' ' << m.m[2][2] << ' ' << m.m[2][3] << '\n'
            << m.m[3][0] << ' ' << m.m[3][1] << ' ' << m.m[3][2] << ' ' << m.m[3][3] << std::endl;
}

mat4x4 Mat4x4_MakeTranslation(float x, float y, float z)
{
  mat4x4 matrix;
  matrix.m[0][0] = 1.0f;
  matrix.m[0][3] = x;
  matrix.m[1][1] = 1.0f;
  matrix.m[1][3] = y;
  matrix.m[2][2] = 1.0f;
  matrix.m[2][3] = z;
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeRotationX(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = 1.0f;
  matrix.m[1][1] = cosf(fAngleRad);
  matrix.m[1][2] = -sinf(fAngleRad);
  matrix.m[2][1] = sinf(fAngleRad);
  matrix.m[2][2] = cosf(fAngleRad);
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeRotationY(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = cosf(fAngleRad);
  matrix.m[0][2] = sinf(fAngleRad);
  matrix.m[1][1] = 1.0f;
  matrix.m[2][0] = -sinf(fAngleRad);
  matrix.m[2][2] = cosf(fAngleRad);
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeRotationZ(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = cosf(fAngleRad);
  matrix.m[0][1] = -sinf(fAngleRad);
  matrix.m[1][0] = sinf(fAngleRad);
  matrix.m[1][1] = cosf(fAngleRad);
  matrix.m[2][2] = 1.0f;
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeRotationArbitraryAxis(float fAngle, vec3d &axis)
{
  // See 'https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle'
  mat4x4 matrix;
  matrix.m[0][0] = cosf(fAngle) + axis.x * axis.x * (1 - cosf(fAngle));
  matrix.m[0][1] = axis.x * axis.y * (1 - cosf(fAngle)) - axis.z * sinf(fAngle);
  matrix.m[0][2] = axis.x * axis.z * (1 - cosf(fAngle)) + axis.y * sinf(fAngle);
  matrix.m[1][0] = axis.y * axis.x * (1 - cosf(fAngle)) + axis.z * sinf(fAngle);
  matrix.m[1][1] = cosf(fAngle) + axis.y * axis.y * (1 - cosf(fAngle));
  matrix.m[1][2] = axis.y * axis.z * (1 - cosf(fAngle)) - axis.x * sinf(fAngle);
  matrix.m[2][0] = axis.z * axis.x * (1 - cosf(fAngle)) - axis.y * sinf(fAngle);
  matrix.m[2][1] = axis.z * axis.y * (1 - cosf(fAngle)) + axis.x * sinf(fAngle);
  matrix.m[2][2] = cosf(fAngle) + axis.z * axis.z * (1 - cosf(fAngle));
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeCameraProjection(float fFovDeg, float fAspectRatio, float fNear, float fFar)
{
  float fTanHalfFov = tanf(0.5f * fFovDeg * 3.141592f / 180.0f);
  mat4x4 matrix;
  matrix.m[0][1] = -1.0f / fTanHalfFov;
  matrix.m[1][2] = -fAspectRatio / fTanHalfFov;
  matrix.m[2][0] = fFar / (fFar - fNear);
  matrix.m[2][3] = - fFar * fNear / (fFar - fNear);
  matrix.m[3][0] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MakeScreenTransform(float fScreenWidth, float fScreenHeight)
{
  mat4x4 matrix;
  matrix.m[0][0] = 0.5f * fScreenWidth;
  matrix.m[0][3] = 0.5f * fScreenWidth;
  matrix.m[1][1] = 0.5f * fScreenHeight;
  matrix.m[1][3] = 0.5f * fScreenHeight;
  matrix.m[2][2] = 1.0f;
  matrix.m[3][3] = 1.0f;
  return matrix;
}

mat4x4 Mat4x4_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
{
  mat4x4 matrix;
  for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++)
      matrix.m[r][c] = (m1.m[r][0] * m2.m[0][c] +
                        m1.m[r][1] * m2.m[1][c] +
                        m1.m[r][2] * m2.m[2][c] +
                        m1.m[r][3] * m2.m[3][c]);
  return matrix;
}

mat4x4 Mat4x4_ConcatenateTransformations(mat4x4 &m1, mat4x4 &m2)
{
  // This is a convencience function to intuitively concatenate multiple transformation matrices.
  // It allows the user to provide the transformations in the order in which they should be
  // executed (first, then second), because the actual multiplication has to happen the other way
  // around (second x first).
  return Mat4x4_MultiplyMatrix(m2, m1);
}

mat4x4 Mat4x4_MakeToCsTransform(coordsys &cs)
{
  mat4x4 matrix;
  matrix.m[0][0] = cs.u.x; matrix.m[0][1] = cs.u.y; matrix.m[0][2] = cs.u.z;
  matrix.m[1][0] = cs.v.x; matrix.m[1][1] = cs.v.y; matrix.m[1][2] = cs.v.z;
  matrix.m[2][0] = cs.w.x; matrix.m[2][1] = cs.w.y; matrix.m[2][2] = cs.w.z;
  matrix.m[0][3] = -Vec3d_DotProduct(cs.u, cs.o);
  matrix.m[1][3] = -Vec3d_DotProduct(cs.v, cs.o);
  matrix.m[2][3] = -Vec3d_DotProduct(cs.w, cs.o);
  matrix.m[3][3] = 1.0f;
  return matrix;
}


// Coordinate system operations
void CoordSys_Print(coordsys &cs)
{
  std::cout << "cs.o: "; Vec3d_Print(cs.o);
  std::cout << "cs.u: "; Vec3d_Print(cs.u);
  std::cout << "cs.v: "; Vec3d_Print(cs.v);
  std::cout << "cs.w: "; Vec3d_Print(cs.w);
}

coordsys CoordSys_LookAt(vec3d &position, vec3d &target, vec3d &up)
{
  coordsys cs;
  cs.o = { position.x, position.y, position.z };
  // Create new forward vector u.
  cs.u = Vec3d_Sub(target, position);
  cs.u = Vec3d_Normalize(cs.u);
  // Create new left vector v.
  cs.v = Vec3d_CrossProduct(up, cs.u);
  cs.v = Vec3d_Normalize(cs.v);
  // Create updated up vector w.
  cs.w = Vec3d_CrossProduct(cs.u, cs.v);
  cs.w = Vec3d_Normalize(cs.w); // Shouldn't really be necessary since u and v are already normalized.
  return cs;
}

void CoordSys_TranslateUVW(coordsys &cs, float u, float v, float w)
{
  vec3d vTranslateU = Vec3d_Mul(cs.u, u);
  vec3d vTranslateV = Vec3d_Mul(cs.v, v);
  vec3d vTranslateW = Vec3d_Mul(cs.w, w);
  cs.o = Vec3d_Add(cs.o, vTranslateU);
  cs.o = Vec3d_Add(cs.o, vTranslateV);
  cs.o = Vec3d_Add(cs.o, vTranslateW);
}

void CoordSys_RotateU(coordsys &cs, float fAngle)
{
  mat4x4 matRotation = Mat4x4_MakeRotationArbitraryAxis(fAngle, cs.u);
  cs.v = Vec3d_ApplyTransform(cs.v, matRotation);
  cs.w = Vec3d_ApplyTransform(cs.w, matRotation);
}

void CoordSys_RotateV(coordsys &cs, float fAngle)
{
  mat4x4 matRotation = Mat4x4_MakeRotationArbitraryAxis(fAngle, cs.v);
  cs.u = Vec3d_ApplyTransform(cs.u, matRotation);
  cs.w = Vec3d_ApplyTransform(cs.w, matRotation);
}

void CoordSys_RotateW(coordsys &cs, float fAngle)
{
  mat4x4 matRotation = Mat4x4_MakeRotationArbitraryAxis(fAngle, cs.w);
  cs.u = Vec3d_ApplyTransform(cs.u, matRotation);
  cs.v = Vec3d_ApplyTransform(cs.v, matRotation);
}


// Triangle operations
vec3d Triangle_Centroid(triangle &tri)
{
  vec3d centroid;
  centroid.x = (tri.p[0].x + tri.p[1].x + tri.p[2].x) / 3.0f;
  centroid.y = (tri.p[0].y + tri.p[1].y + tri.p[2].y) / 3.0f;
  centroid.z = (tri.p[0].z + tri.p[1].z + tri.p[2].z) / 3.0f;
  return centroid;
}

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
{
  // Returns how many triangles resulted from the clipping (can be 0, 1 or 2).
  // Clipped triangles are stored in the provided references to output triangles.

  // Make sure the provided plane normal is indeed normal.
  plane_n = Vec3d_Normalize(plane_n);

  // Return signed distance from a point to the plane.
  auto dist = [&](vec3d &p)
  {
    vec3d n = Vec3d_Normalize(p);
    return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vec3d_DotProduct(plane_n, plane_p));
  };

  // Create two temporary storage arrays to classify points as being on either side of the plane.
  // If the distance from the point to the plane is positive, it is considered to be "inside".
  vec3d* inside_points[3]; int nInsidePointCount = 0;
  vec3d* outside_points[3]; int nOutsidePointCount = 0;

  // Get the signed distance of each point in the triangle to the plane.
  float d0 = dist(in_tri.p[0]);
  float d1 = dist(in_tri.p[1]);
  float d2 = dist(in_tri.p[2]);

  if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
  else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
  if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
  else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
  if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
  else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

  // Now classify the triangle points, and break the input triangle into
  // smaller output triangles if required. There are four possible outcomes.

  if (nInsidePointCount == 0)
  {
    // All points lie on the outside of the plane, so clip the whole triangle.
    // It ceases to exist.

    return 0;  // No returned triangles are valid.
  }

  if (nInsidePointCount == 3)
  {
    // All points lie on the inside of the plane, so do nothing and allow
    // the whole triangle to simply pass through.
    out_tri1 = in_tri;

    return 1;  // Just the one returned original triangle is valid.
  }

  if (nInsidePointCount == 1 && nOutsidePointCount == 2)
  {
    // The triangle should be clipped. As two points lie outside the plane,
    // the triangle simply becomes a single smaller triangle.

    // Copy triangle appearance info to the new triangle.
    out_tri1.fillColor = in_tri.fillColor;
    out_tri1.wireColor = in_tri.wireColor;

    // The inside point is valid, so keep that.
    out_tri1.p[0] = *inside_points[0];

    // The two new points are at the location where the original sides of
    // the triangle intersect with the plane.
    out_tri1.p[1] = Vec3d_WhereLineIntersectsPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
    out_tri1.p[2] = Vec3d_WhereLineIntersectsPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

    // Here we could make sure that the new triangle's normal is the same as
    // the original triangle's normal and flip points around if needed, but
    // we currently don't need the normal of the triangle after clipping,
    // so we're not going to do that.

    return 1;  // Return the newly formed single triangle.
  }

  if (nInsidePointCount == 2 && nOutsidePointCount == 1)
  {
    // The triangle should be clipped. As two points lie inside the plane,
    // the clipped triangle becomes a "quad". Fortunately, we can represent
    // a quad with two new triangles.

    // Copy triangle appearance info to the new triangles.
    out_tri1.fillColor = in_tri.fillColor;
    out_tri1.wireColor = in_tri.wireColor;

    out_tri2.fillColor = in_tri.fillColor;
    out_tri2.wireColor = in_tri.wireColor;

    // The first triangle consists of the two inside points and a new point
    // determined by the location where one side of the triangle intersects
    // with the plane.
    out_tri1.p[0] = *inside_points[0];
    out_tri1.p[1] = *inside_points[1];
    out_tri1.p[2] = Vec3d_WhereLineIntersectsPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

    // The second triangle is composed of one of the inside points, a new
    // point determined by the intersection of the other side of the triangle,
    // and the newly created point above.
    out_tri2.p[0] = *inside_points[1];
    out_tri2.p[1] = out_tri1.p[2];
    out_tri2.p[2] = Vec3d_WhereLineIntersectsPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

    return 2;  // Return two newly formed triangles which together form a quad.
  }

  return -1;  // Should never happen, but added to prevent a compile warning.

}


// The 3D graphics engine class.
class olcEngine3D : public olc::PixelGameEngine
{
public:
  olcEngine3D()
  {
    sAppName = "3D Demo";
  }

private:
  mesh meshLocal;  // The drawn object in local space.
  float meshDeltaTheta;  // Setting for how fast the mesh should rotate.
  float meshCurrentTheta; // Used to keep track of the mesh's current rotation angle, updated at every frame.
  vec3d meshTranslation;  // Used to keep track of the mesh's current translation.

  coordsys csCamera;  // Used to keep track of the current position and orientation of the camera.
  float fFovDeg, fNear, fFar, fAspectRatio;  // Camera settings.

  mat4x4 matCameraToProjected;  // Matrix to transform from camera space to normalized projection space.
  mat4x4 matProjectedToScreen;  // Matrix to transform from normalized projection space to screen space.

  olc::Pixel colorDay, colorNight, colorSky, colorGrass, colorMountain, colorSnow;
  vec3d lightDirection;  // Direction of the light, we assume the source is infinitely far away.

public:
  bool OnUserCreate() override
  {
    // Initialize a mesh in local space.
    // meshLocal.tris = {  // Unit cube centered on the origin.
    //   // SOUTH
    //   { -0.5f,  0.5f, -0.5f, 1.0f,    -0.5f, -0.5f, -0.5f, 1.0f,    -0.5f, -0.5f,  0.5f, 1.0f },
    //   { -0.5f,  0.5f, -0.5f, 1.0f,    -0.5f, -0.5f,  0.5f, 1.0f,    -0.5f,  0.5f,  0.5f, 1.0f },
    //   // EAST
    //   { -0.5f, -0.5f, -0.5f, 1.0f,     0.5f, -0.5f, -0.5f, 1.0f,     0.5f, -0.5f,  0.5f, 1.0f },
    //   { -0.5f, -0.5f, -0.5f, 1.0f,     0.5f, -0.5f,  0.5f, 1.0f,    -0.5f, -0.5f,  0.5f, 1.0f },
    //   // NORTH
    //   {  0.5f, -0.5f, -0.5f, 1.0f,     0.5f,  0.5f, -0.5f, 1.0f,     0.5f,  0.5f,  0.5f, 1.0f },
    //   {  0.5f, -0.5f, -0.5f, 1.0f,     0.5f,  0.5f,  0.5f, 1.0f,     0.5f, -0.5f,  0.5f, 1.0f },
    //   // WEST
    //   {  0.5f,  0.5f, -0.5f, 1.0f,    -0.5f,  0.5f, -0.5f, 1.0f,    -0.5f,  0.5f,  0.5f, 1.0f },
    //   {  0.5f,  0.5f, -0.5f, 1.0f,    -0.5f,  0.5f,  0.5f, 1.0f,     0.5f,  0.5f,  0.5f, 1.0f },
    //   // TOP
    //   { -0.5f,  0.5f,  0.5f, 1.0f,    -0.5f, -0.5f,  0.5f, 1.0f,     0.5f, -0.5f,  0.5f, 1.0f },
    //   { -0.5f,  0.5f,  0.5f, 1.0f,     0.5f, -0.5f,  0.5f, 1.0f,     0.5f,  0.5f,  0.5f, 1.0f },
    //   // BOTTOM
    //   { -0.5f, -0.5f, -0.5f, 1.0f,    -0.5f,  0.5f, -0.5f, 1.0f,     0.5f,  0.5f, -0.5f, 1.0f },
    //   { -0.5f, -0.5f, -0.5f, 1.0f,     0.5f,  0.5f, -0.5f, 1.0f,     0.5f, -0.5f, -0.5f, 1.0f },
    // }; meshTranslation = { 0.0f, 0.0f, 0.0f }; meshDeltaTheta = 0.4f;
    // meshLocal.LoadFromObjectFile("axes.obj"); meshTranslation = { 0.0f, 0.0f, 0.0f }; meshDeltaTheta = 0.0f;
    // meshLocal.LoadFromObjectFile("teapot.obj"); meshTranslation = { 0.0f, 0.0f, 0.0f }; meshDeltaTheta = 0.0f;
    meshLocal.LoadFromObjectFile("mountains.obj"); meshTranslation = { 0.0f, 0.0f, 0.0f }; meshDeltaTheta = 0.0f;
    meshCurrentTheta = 0.0f;
    std::cout << "Loaded " << meshLocal.tris.size() << " triangles." << std::endl;

    // Initial camera coordinate system. Updated with user input.
    vec3d vCameraPosition = { 0.0f, -17.5f, -15.0f };
    vec3d vCameraTarget = { 1.0f, -17.5f, -15.0f };
    vec3d vCameraUp = { 0.0f, 0.0f, 1.0f };
    csCamera = CoordSys_LookAt(vCameraPosition, vCameraTarget, vCameraUp);

    // Camera settings.
    fFovDeg = 60.0f; fNear = 0.1f; fFar = 1000.0f;
    fAspectRatio = (float)ScreenWidth() / (float)ScreenHeight();

    // Camera projection matrix. Needs to be calculated only once.
    matCameraToProjected = Mat4x4_MakeCameraProjection(fFovDeg, fAspectRatio, fNear, fFar);

    // Projection matrix from normalized projection space to screen space. Needs to be calculated only once.
    matProjectedToScreen = Mat4x4_MakeScreenTransform((float)ScreenWidth(), (float)ScreenHeight());

    // Initial direction of the light.
    lightDirection = { 0.0f, 0.0f, -1.0f };
    lightDirection = Vec3d_Normalize(lightDirection);

    // Set colors.
    colorDay.r = 135; colorDay.g = 206; colorDay.b = 235;
    colorNight.r = 0; colorNight.g = 0; colorNight.b = 49;
    colorSky.r = 0; colorSky.g = 0; colorSky.b = 0;
    colorGrass.r = 126; colorGrass.g = 200; colorGrass.b = 80;
    colorMountain.r = 127; colorMountain.g = 131; colorMountain.b = 134;
    colorSnow.r = 255; colorSnow.g = 255; colorSnow.b = 255;

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Process user input.
    // Translational degrees of freedom
    if (GetKey(olc::Key::W).bHeld)  // Forward
    {
      CoordSys_TranslateUVW(csCamera, 8.0f * fElapsedTime, 0.0f, 0.0f);
    }
    if (GetKey(olc::Key::S).bHeld)  // Backward
    {
      CoordSys_TranslateUVW(csCamera, -8.0f * fElapsedTime, 0.0f, 0.0f);
    }
    if (GetKey(olc::Key::Q).bHeld)  // Left
    {
      CoordSys_TranslateUVW(csCamera, 0.0f, 8.0f * fElapsedTime, 0.0f);
    }
    if (GetKey(olc::Key::E).bHeld)  // Right
    {
      CoordSys_TranslateUVW(csCamera, 0.0f, -8.0f * fElapsedTime, 0.0f);
    }
    if (GetKey(olc::Key::SPACE).bHeld)  // Up
    {
      CoordSys_TranslateUVW(csCamera, 0.0f, 0.0f, 8.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::SHIFT).bHeld)  // Down
    {
      CoordSys_TranslateUVW(csCamera, 0.0f, 0.0f, -8.0f * fElapsedTime);
    }
    // Rotational degrees of freedom
    if (GetKey(olc::Key::RIGHT).bHeld)  // Roll right
    {
      CoordSys_RotateU(csCamera, 1.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::LEFT).bHeld)  // Roll left
    {
      CoordSys_RotateU(csCamera, -1.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::UP).bHeld)  // Pitch down
    {
      CoordSys_RotateV(csCamera, 1.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::DOWN).bHeld)  // Pitch up
    {
      CoordSys_RotateV(csCamera, -1.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::A).bHeld)  // Yaw left
    {
      CoordSys_RotateW(csCamera, 1.0f * fElapsedTime);
    }
    if (GetKey(olc::Key::D).bHeld)  // Yaw right
    {
      CoordSys_RotateW(csCamera, -1.0f * fElapsedTime);
    }
    // Some useful debugging keys.
    if (GetKey(olc::Key::P).bPressed)  // Print camera info.
    {
      std::cout << "===" << '\n';
      std::cout << "Camera position: "; Vec3d_Print(csCamera.o);
      std::cout << "Camera forward : "; Vec3d_Print(csCamera.u);
      std::cout << "Camera up      : "; Vec3d_Print(csCamera.w);
    }

    // The world-to-camera transformation matrix is re-calculated every frame because
    // the coordinate system from which it is derived might have changed due to user input.
    mat4x4 matWorldToCamera = Mat4x4_MakeToCsTransform(csCamera);

    // Update the direction of light to make it seem as if time passes.
    mat4x4 matLightRot = Mat4x4_MakeRotationX(0.25f * fElapsedTime);
    lightDirection = Vec3d_ApplyTransform(lightDirection, matLightRot);
    vec3d vMidday = { 0.0f, 0.0f, -1.0f };

    // Set the color of the sky based on the light direction to simulate night/day.
    float dp = Vec3d_DotProduct(lightDirection, vMidday);  // dp is between -1 and 1.
    float dpNormalized = 0.5f * (dp + 1.0f);  // dpNormalized is between 0 and 1.
    colorSky.r = colorNight.r + dpNormalized * (colorDay.r - colorNight.r);
    colorSky.g = colorNight.g + dpNormalized * (colorDay.g - colorNight.g);
    colorSky.b = colorNight.b + dpNormalized * (colorDay.b - colorNight.b);

    // Update mesh rotation angle to have it rotate, and calculate its world transformation matrix.
    meshCurrentTheta += meshDeltaTheta * fElapsedTime;
    mat4x4 matRotX = Mat4x4_MakeRotationX(meshCurrentTheta);
    mat4x4 matRotY = Mat4x4_MakeRotationY(3.141592f*meshCurrentTheta);
    mat4x4 matRotZ = Mat4x4_MakeRotationZ(1.414214f*meshCurrentTheta);
    mat4x4 matTrl = Mat4x4_MakeTranslation(meshTranslation.x, meshTranslation.y, meshTranslation.z);
    mat4x4 matRotXY = Mat4x4_ConcatenateTransformations(matRotX, matRotY);
    mat4x4 matRotXYZ = Mat4x4_ConcatenateTransformations(matRotXY, matRotZ);
    mat4x4 matWorld = Mat4x4_ConcatenateTransformations(matRotXYZ, matTrl);

    // Decide which triangles to rasterize.
    std::vector<triangle> vecTrianglesToRasterize;
    for (auto triLocal : meshLocal.tris)
    {
      // Transform the triangle from local space to world space.
      // This is done here instead of in OnUserCreate because we want the mesh to move instead of
      // the camera. Once the camera can move, the mesh can be stationary and needs to be positioned
      // somewhere in the world only once in OnUserCreate.
      triangle triWorld;
      triWorld.p[0] = Vec3d_ApplyTransform(triLocal.p[0], matWorld);
      triWorld.p[1] = Vec3d_ApplyTransform(triLocal.p[1], matWorld);
      triWorld.p[2] = Vec3d_ApplyTransform(triLocal.p[2], matWorld);

      // Use cross product to get the triangle's normal.
      vec3d v1, v2, normal;
      v1 = Vec3d_Sub(triWorld.p[1], triWorld.p[0]);
      v2 = Vec3d_Sub(triWorld.p[2], triWorld.p[0]);
      normal = Vec3d_CrossProduct(v1, v2);
      normal = Vec3d_Normalize(normal);

      // Ray from the triangle to the camera.
      vec3d vCameraRay = Vec3d_Sub(csCamera.o, triWorld.p[0]);

      // Only continue if the triangle is visible.
      if (Vec3d_DotProduct(normal, vCameraRay) > 0.0f)
      {
        // Set initial triangle color.
        triWorld.fillColor = { colorGrass.r, colorGrass.g, colorGrass.b };
        if (Triangle_Centroid(triWorld).z > -10.0f)
          triWorld.fillColor = { colorMountain.r, colorMountain.g, colorMountain.b };
        if (Triangle_Centroid(triWorld).z > 5.0f)
          triWorld.fillColor = { colorSnow.r, colorSnow.g, colorSnow.b };

        // Apply illumination
        // The less similarity between the triangle normal and the light direction, the more
        // that triangle faces the light source and is illuminated.
        float dp = Vec3d_DotProduct(lightDirection, normal);  // dp is between -1 and 1.
        float dpNormalized = 0.5f * (1.0f - dp);  // dpNormalized is between 0 and 1.
        triWorld.fillColor.r *= dpNormalized;
        triWorld.fillColor.g *= dpNormalized;
        triWorld.fillColor.b *= dpNormalized;
        triWorld.wireColor = (dpNormalized >= 0.5f) ? olc::BLACK : olc::WHITE;

        // Transform the triangle from world space to camera space.
        triangle triCamera;
        triCamera.p[0] = Vec3d_ApplyTransform(triWorld.p[0], matWorldToCamera);
        triCamera.p[1] = Vec3d_ApplyTransform(triWorld.p[1], matWorldToCamera);
        triCamera.p[2] = Vec3d_ApplyTransform(triWorld.p[2], matWorldToCamera);
        triCamera.fillColor = triWorld.fillColor;
        triCamera.wireColor = triWorld.wireColor;

        // Only continue if at least one of the triangle's points is ahead, but not too far ahead.
        if ((triCamera.p[0].x < fFar || triCamera.p[1].x < fFar || triCamera.p[2].x < fFar) &&
            (triCamera.p[0].x > fNear || triCamera.p[1].x > fNear || triCamera.p[2].x > fNear))
        {
          // Clip triangles against the near plane in the normalized projection space.
          // We clip with this plane here because, once projected, we lose the ability
          // to use the depth to properly determine whether a triangle is in front of
          // or behind the near plane. Also see theory concerning hyperbolic relationship
          // between a point's X-value in camera space and the projected depth value.
          // The clipping could form two additional triangles.
          int nClippedTriangles = 0;
          triangle clipped[2];
          vec3d front_p = { fNear, 0.0f, 0.0f };
          vec3d front_n = { 1.0f, 0.0f, 0.0f };
          nClippedTriangles = Triangle_ClipAgainstPlane(front_p, front_n, triCamera, clipped[0], clipped[1]);
          for (int n = 0; n < nClippedTriangles; ++n)
          {
            // Transform the clipped triangle from camera space to normalized projection space.
            triangle triProjectedTimesX, triProjected;
            triProjectedTimesX.p[0] = Vec3d_ApplyTransform(clipped[n].p[0], matCameraToProjected);
            triProjectedTimesX.p[1] = Vec3d_ApplyTransform(clipped[n].p[1], matCameraToProjected);
            triProjectedTimesX.p[2] = Vec3d_ApplyTransform(clipped[n].p[2], matCameraToProjected);
            triProjected.p[0] = Vec3d_Div(triProjectedTimesX.p[0], triProjectedTimesX.p[0].w);
            triProjected.p[1] = Vec3d_Div(triProjectedTimesX.p[1], triProjectedTimesX.p[1].w);
            triProjected.p[2] = Vec3d_Div(triProjectedTimesX.p[2], triProjectedTimesX.p[2].w);
            triProjected.fillColor = clipped[n].fillColor;
            triProjected.wireColor = clipped[n].wireColor;

            // Transform the clipped triangle from normalized projection space to screen space.
            triangle triScreen;
            triScreen.p[0] = Vec3d_ApplyTransform(triProjected.p[0], matProjectedToScreen);
            triScreen.p[1] = Vec3d_ApplyTransform(triProjected.p[1], matProjectedToScreen);
            triScreen.p[2] = Vec3d_ApplyTransform(triProjected.p[2], matProjectedToScreen);
            triScreen.fillColor = triProjected.fillColor;
            triScreen.wireColor = triProjected.wireColor;

            // Store triangle for sorting.
            vecTrianglesToRasterize.push_back(triScreen);
          }
        }
      }
    }

    // Perform further clipping of triangles that need to be rasterized.
    std::vector<triangle> vecClippedTrianglesToRasterize;
    for (auto &triToClip : vecTrianglesToRasterize)
    {
      // Clip triangles against the remaining planes.
      // Currently these are only the screen edges, but in the future we may wany to also
      // clip against the far plane to improve performance. Probably not though, it wouldn't
      // look too great (i.e. much worse than pop-in).
      // This could yield a bunch of triangles.
      vec3d plane_ps[5];
      vec3d plane_ns[5];
      plane_ps[0] = { 0.0f, 0.0f, 0.0f }; plane_ns[0] = { 1.0f, 0.0f, 0.0f };  // Left
      plane_ps[1] = { (float)ScreenWidth(), (float)ScreenHeight(), 1.0f }; plane_ns[1] = { -1.0f, 0.0f, 0.0f };  // Right
      plane_ps[2] = plane_ps[0]; plane_ns[2] = { 0.0f, 1.0f, 0.0f };  // Top
      plane_ps[3] = plane_ps[1]; plane_ns[3] = { 0.0f, -1.0f, 0.0f };  // Bottom
      plane_ps[4] = plane_ps[1]; plane_ns[4] = { 0.0f, 0.0f, -1.0f };  // Back

      triangle clipped[2];
      std::list<triangle> listTriangles;
      listTriangles.push_back(triToClip);
      int nNewTriangles = 1;
      for (int p = 0; p <= 4; ++p)
      {
        int nTrisToAdd = 0;
        while (nNewTriangles > 0)
        {
          triangle test = listTriangles.front();
          listTriangles.pop_front();
          nNewTriangles--;

          nTrisToAdd = Triangle_ClipAgainstPlane(plane_ps[p], plane_ns[p], test, clipped[0], clipped[1]);
          for (int w = 0; w < nTrisToAdd; ++w)
            listTriangles.push_back(clipped[w]);

        }
        nNewTriangles = listTriangles.size();
      }

      for (auto &t : listTriangles)
        vecClippedTrianglesToRasterize.push_back(t);

    }

    // Sort the triangles from back to front.
    // We compare the z-value of the triangle's centroid.
    // The z-value here is the normalized projected depth.
    sort(vecClippedTrianglesToRasterize.begin(), vecClippedTrianglesToRasterize.end(), [](triangle &t1, triangle &t2)
    {
      float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
      float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
      return z1 > z2;
    });

    // Clear the screen.
    Clear(colorSky);

    // Rasterize the sorted triangles.
    for (auto &triToRasterize : vecClippedTrianglesToRasterize)
    {
        FillTriangle(triToRasterize.p[0].x, triToRasterize.p[0].y,
                     triToRasterize.p[1].x, triToRasterize.p[1].y,
                     triToRasterize.p[2].x, triToRasterize.p[2].y,
                     triToRasterize.fillColor);
        // DrawTriangle(triToRasterize.p[0].x, triToRasterize.p[0].y,
        //              triToRasterize.p[1].x, triToRasterize.p[1].y,
        //              triToRasterize.p[2].x, triToRasterize.p[2].y,
        //              triToRasterize.wireColor);
    }
    return true;
  }
};


void testProjectionMatrix()
{
  // TESTING THE PROJECTION MATRIX
  // Fictional screen and camera settings.
  float fScreenWidth = 100, fScreenHeight = 100;
  float fFovDeg = 90.0f, fAspectRatio = fScreenWidth / fScreenHeight, fNear = 0.1f, fFar = 20.0f;

  // Test input, choose one.
  float fTanHalfFov = tanf(0.5f * fFovDeg * 3.141592f / 180.0f);
  vec3d vCamera = { fNear, 0.0f, 0.0f };  // Dead center front
  // vec3d vCamera = { fFar, 0.0f, 0.0f };  // Dead center back
  // vec3d vCamera = { fNear, fNear * fTanHalfFov, fNear / fAspectRatio * fTanHalfFov };  // Top Left Front
  // vec3d vCamera = { fNear, fNear * fTanHalfFov, -fNear / fAspectRatio * fTanHalfFov };  // Bottom Left Front
  // vec3d vCamera = { fNear, -fNear * fTanHalfFov, fNear / fAspectRatio * fTanHalfFov };  // Top Right Front
  // vec3d vCamera = { fNear, -fNear * fTanHalfFov, -fNear / fAspectRatio * fTanHalfFov };  // Bottom Right Front
  // vec3d vCamera = { fFar, fFar * fTanHalfFov, fFar / fAspectRatio * fTanHalfFov };  // Top Left Back
  // vec3d vCamera = { fFar, fFar * fTanHalfFov, -fFar / fAspectRatio * fTanHalfFov };  // Bottom Left Back
  // vec3d vCamera = { fFar, -fFar * fTanHalfFov, fFar / fAspectRatio * fTanHalfFov };  // Top Right Back
  // vec3d vCamera = { fFar, -fFar * fTanHalfFov, -fFar / fAspectRatio * fTanHalfFov };  // Bottom Right Back

  // Transformation from 3D camera space to 3D normalized projected space.
  mat4x4 matCameraToProjected = Mat4x4_MakeCameraProjection(fFovDeg, fAspectRatio, fNear, fFar);
  vec3d vProjectedTimesX = Vec3d_ApplyTransform(vCamera, matCameraToProjected);
  vec3d vProjected = Vec3d_Div(vProjectedTimesX, vProjectedTimesX.w);

  // Transformation from 3D normalized projected space to 2D screen space.
  mat4x4 matProjectedToScreen = Mat4x4_MakeScreenTransform(fScreenWidth, fScreenHeight);
  vec3d vScreen = Vec3d_ApplyTransform(vProjected, matProjectedToScreen);

  std::cout << "vCamera: "; Vec3d_Print(vCamera);
  std::cout << "vScreen: "; Vec3d_Print(vScreen);
}


int main()
{
  // testProjectionMatrix();
  olcEngine3D demo;
  if (demo.Construct(640, 480, 1, 1))
    demo.Start();
  return 0;
}
