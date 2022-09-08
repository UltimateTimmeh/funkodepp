// olcShadowCasting2D: Line of Sight or Shadow Casting in 2D, tutorial by javidx9.
#include <iostream>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


struct sEdge
{
  float sx, sy;
  float ex, ey;
};

struct sCell
{
  int edge_id[4];
  bool edge_exist[4];
  bool exist = false;
};

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3


class olcShadowCasting2D : public olc::PixelGameEngine
{
public:
  olcShadowCasting2D()
  {
    sAppName = "ShadowCasting2D";
  }

private:
  sCell *world;
  int nWorldWidth = 40;
  int nWorldHeight = 30;
  int fBlockWidth = 16;

  olc::Sprite *sprLightCast;
  olc::Sprite *buffLightRay;
  olc::Sprite *buffLightTex;

  std::vector<sEdge> vecEdges;

  std::vector<std::tuple<float, float, float>> vecVisibilityPolygonPoints;

  void ConvertTileMapToPolyMap(int sx, int sy, int w, int h, float step, int pitch)
  {
    // Clear the polymap.
    vecEdges.clear();

    for (int x = 0; x < w; x++)
      for (int y = 0; y < h; y++)
        for (int j = 0; j < 4; j++)
        {
          world[(y + sy) * pitch + (x + sx)].edge_exist[j] = false;
          world[(y + sy) * pitch + (x + sx)].edge_id[j] = 0;
        }

    // Iterate through region from top left to bottom right
    for (int x = 1; x < w - 1; x++)
      for (int y = 1; y < h - 1; y++)
      {
        // Create some convenient indices
        int i = (y + sy) * pitch + (x + sx);      // Current Cell.
        int n = (y + sy - 1) * pitch + (x + sx);  // Northern Neighbour.
        int s = (y + sy + 1) * pitch + (x + sx);  // Southern Neighbour.
        int w = (y + sy) * pitch + (x + sx - 1);  // Western Neighbour.
        int e = (y + sy) * pitch + (x + sx + 1);  // Eastern Neighbour.

        // If this cell exists, check if it needs edges.
        if (world[i].exist)
        {
          // If this cell has no western neighbour, it needs a western edge.
          if (!world[w].exist)
          {
            // It can either extend it from its northern neighbor if it has
            // one, or it can start a new one.
            if (world[n].edge_exist[WEST])
            {
              // Northern neighbor has a western edge, so grow it downwards.
              vecEdges[world[n].edge_id[WEST]].ey += step;
              world[i].edge_id[WEST] = world[n].edge_id[WEST];
              world[i].edge_exist[WEST] = true;
            }
            else
            {
              // Northern neighbor does not have one, so create one.
              sEdge edge;
              edge.sx = (sx + x) * step; edge.sy = (sy + y) * step;
              edge.ex = edge.sx; edge.ey = edge.sy + step;

              // Add edge to pool.
              int edge_id = vecEdges.size();
              vecEdges.push_back(edge);

              // Update tile information with edge information.
              world[i].edge_id[WEST] = edge_id;
              world[i].edge_exist[WEST] = true;
            }
          }

          // If this cell has no eastern neighbour, it needs an eastern edge.
          if (!world[e].exist)
          {
            // It can either extend it from its northern neighbor if it has
            // one, or it can start a new one.
            if (world[n].edge_exist[EAST])
            {
              // Northern neighbor has an eastern edge, so grow it downwards.
              vecEdges[world[n].edge_id[EAST]].ey += step;
              world[i].edge_id[EAST] = world[n].edge_id[EAST];
              world[i].edge_exist[EAST] = true;
            }
            else
            {
              // Northern neighbor does not have one, so create one.
              sEdge edge;
              edge.sx = (sx + x + 1) * step; edge.sy = (sy + y) * step;
              edge.ex = edge.sx; edge.ey = edge.sy + step;

              // Add edge to pool.
              int edge_id = vecEdges.size();
              vecEdges.push_back(edge);

              // Update tile information with edge information.
              world[i].edge_id[EAST] = edge_id;
              world[i].edge_exist[EAST] = true;
            }
          }

          // If this cell has no northern neighbour, it needs a northern edge.
          if (!world[n].exist)
          {
            // It can either extend it from its western neighbor if it has
            // one, or it can start a new one.
            if (world[w].edge_exist[NORTH])
            {
              // Western neighbor has a northern edge, so grow it to the right.
              vecEdges[world[w].edge_id[NORTH]].ex += step;
              world[i].edge_id[NORTH] = world[w].edge_id[NORTH];
              world[i].edge_exist[NORTH] = true;
            }
            else
            {
              // Western neighbor does not have one, so create one.
              sEdge edge;
              edge.sx = (sx + x) * step; edge.sy = (sy + y) * step;
              edge.ex = edge.sx + step; edge.ey = edge.sy;

              // Add edge to pool.
              int edge_id = vecEdges.size();
              vecEdges.push_back(edge);

              // Update tile information with edge information.
              world[i].edge_id[NORTH] = edge_id;
              world[i].edge_exist[NORTH] = true;
            }
          }

          // If this cell has no southern neighbour, it needs a southern edge.
          if (!world[s].exist)
          {
            // It can either extend it from its western neighbor if it has
            // one, or it can start a new one.
            if (world[w].edge_exist[SOUTH])
            {
              // Western neighbor has a southern edge, so grow it to the right.
              vecEdges[world[w].edge_id[SOUTH]].ex += step;
              world[i].edge_id[SOUTH] = world[w].edge_id[SOUTH];
              world[i].edge_exist[SOUTH] = true;
            }
            else
            {
              // Western neighbor does not have one, so create one.
              sEdge edge;
              edge.sx = (sx + x) * step; edge.sy = (sy + y + 1) * step;
              edge.ex = edge.sx + step; edge.ey = edge.sy;

              // Add edge to pool.
              int edge_id = vecEdges.size();
              vecEdges.push_back(edge);

              // Update tile information with edge information.
              world[i].edge_id[SOUTH] = edge_id;
              world[i].edge_exist[SOUTH] = true;
            }
          }
        }


      }
  }

  void CalculateVisibilityPolygon(float ox, float oy, float radius)
  {
    // Get rid of existing polygon
    vecVisibilityPolygonPoints.clear();

    // For each edge in poly map.
    for (auto &e1 : vecEdges)
    {
      // Take the start point, then the end point (we could use a pool of
      // non-duplicated points here, it would be more optimal).
      for (int i = 0; i < 2; i++)
      {
        float rdx, rdy;
        rdx = (i == 0 ? e1.sx : e1.ex) - ox;
        rdy = (i == 0 ? e1.sy : e1.ey) - oy;

        float base_ang = atan2f(rdy, rdx);

        float ang = 0;

        // For each point, cast three rays, one directly at the point
        // and one a little bit to either side.
        for (int j = 0; j < 3; j++)
        {
          if (j == 0) ang = base_ang - 0.0001f;
          if (j == 1) ang = base_ang;
          if (j == 2) ang = base_ang + 0.0001f;

          // Create ray along angle for required distance.
          rdx = radius * cosf(ang);
          rdy = radius * sinf(ang);

          float min_t1 = INFINITY;
          float min_px = 0, min_py = 0, min_ang = 0;

          bool bValid = false;

          // Check for ray intersection with all edges.
          for (auto &e2 : vecEdges)
          {
            // Create line segment vector.
            float sdx = e2.ex - e2.sx;
            float sdy = e2.ey - e2.sy;

            if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
            {
              // t2 is normalized distance from line segment start to interseection point.
              float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx);
              // t1 is normalized distance from source along ray to interseection point.
              float t1 = (e2.sx + sdx * t2 - ox) / rdx;

              // If intersect point exists along ray, and along line
              // segment, then intersection point is valid.
              if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
              {
                // Check if this intersection point is closest to source. If
                // it is, then store this point and reject others.
                if (t1 < min_t1)
                {
                  min_t1 = t1;
                  min_px = ox + rdx * t1;
                  min_py = oy + rdy * t1;
                  min_ang = atan2f(min_py - oy, min_px - ox);
                  bValid = true;
                }
              }
            }
          }

          // Add intersection point to visibility polygon perimeter.
          if (bValid)
            vecVisibilityPolygonPoints.push_back({ min_ang, min_px, min_py });

        }
      }
    }

    // Sort perimeter points by angle from source. This will allow
    // us to draw a triangle fan.
    sort(
         vecVisibilityPolygonPoints.begin(),
         vecVisibilityPolygonPoints.end(),
         [&](const std::tuple<float, float, float> &t1, const std::tuple<float, float, float> &t2)
         {
          return std::get<0>(t1) < std::get<0>(t2);
         });

  }

public:
  bool OnUserCreate() override
  {
    world = new sCell[nWorldWidth * nWorldHeight];

    // Add a boundary to the world.
    for (int x = 1; x < (nWorldWidth - 1); x++)
    {
      world[1 * nWorldWidth + x].exist = true;
      world[(nWorldHeight - 2) * nWorldWidth + x].exist = true;
    }
    for (int y = 1; y < (nWorldHeight - 1); y++)
    {
      world[y * nWorldWidth + 1].exist = true;
      world[y * nWorldWidth + (nWorldWidth - 2)].exist = true;
    }

    // Calculate initial poly map.
    ConvertTileMapToPolyMap(0, 0, nWorldWidth, nWorldHeight, fBlockWidth, nWorldWidth);

    // Load light source sprite and buffers.
    sprLightCast = new olc::Sprite("light_cast.png");
    buffLightRay = new olc::Sprite(ScreenWidth(), ScreenHeight());
    buffLightTex = new olc::Sprite(ScreenWidth(), ScreenHeight());

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    float fSourceX = GetMouseX();
    float fSourceY = GetMouseY();

    // Set tile map blocks to on or off.
    if (GetMouse(0).bReleased)
    {
      // i = y * width + x
      int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
      world[i].exist = !world[i].exist;
      ConvertTileMapToPolyMap(0, 0, nWorldWidth, nWorldHeight, fBlockWidth, nWorldWidth);
    }

    // Update visibility polygon perimeter if right mouse button held.
    if (GetMouse(1).bHeld)
    {
      CalculateVisibilityPolygon(fSourceX, fSourceY, 1000.0f);
    }

    // Drawing.
    SetDrawTarget(nullptr);
    Clear(olc::BLACK);

    int nRaysCast = vecVisibilityPolygonPoints.size();

    auto it = unique(
        vecVisibilityPolygonPoints.begin(),
        vecVisibilityPolygonPoints.end(),
        [&](const std::tuple<float, float, float> &t1, const std::tuple<float, float, float> &t2)
        {
          return fabs(std::get<1>(t1) - std::get<1>(t2)) < 0.1f && fabs(std::get<2>(t1) - std::get<2>(t2)) < 0.1f;
        });
    vecVisibilityPolygonPoints.resize(distance(vecVisibilityPolygonPoints.begin(), it));

    int nRaysCast2 = vecVisibilityPolygonPoints.size();
    DrawString(4, 4, "Rays Cast: " + std::to_string(nRaysCast) + ", Rays Drawn: " + std::to_string(nRaysCast2));

    // If drawing rays, set an offscreen texture as our target buffer.
    if (GetMouse(1).bHeld && vecVisibilityPolygonPoints.size() > 1)
    {
      // Clear offscreen buffer for sprite.
      SetDrawTarget(buffLightTex);
      Clear(olc::BLACK);

      // Draw "Radial Light" sprite to offscreen buffer, centered around
      // source location (the mouse coordinates, buffer is 512x512).
      DrawSprite(fSourceX - 255, fSourceY - 255, sprLightCast);

      // Clear offscreen buffer for rays.
      SetDrawTarget(buffLightRay);
      Clear(olc::BLANK);

      // Draw each triangle in fan.
      for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++)
      {
        FillTriangle(
          fSourceX,
          fSourceY,

          std::get<1>(vecVisibilityPolygonPoints[i]),
          std::get<2>(vecVisibilityPolygonPoints[i]),

          std::get<1>(vecVisibilityPolygonPoints[i + 1]),
          std::get<2>(vecVisibilityPolygonPoints[i + 1])
        );
      }

      FillTriangle(
        fSourceX,
        fSourceY,

        std::get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
        std::get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),

        std::get<1>(vecVisibilityPolygonPoints[0]),
        std::get<2>(vecVisibilityPolygonPoints[0])
      );

      // Wherever rays exist in ray sprite, copy over radial light sprite.
      SetDrawTarget(nullptr);
      for (int x = 0; x < ScreenWidth(); x++)
        for (int y = 0; y < ScreenHeight(); y++)
          if (buffLightRay->GetPixel(x, y).r > 0)
            Draw(x, y, buffLightTex->GetPixel(x, y));
    }

    // Draw blocks from tile map.
    for (int x = 0; x < nWorldWidth; x++)
      for (int y = 0; y < nWorldHeight; y++)
      {
        if (world[y * nWorldWidth + x].exist)
          FillRect(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, olc::BLUE);
      }

    // Draw edges from poly map.
    for (auto &e : vecEdges)
    {
      DrawLine(e.sx, e.sy, e.ex, e.ey);
      FillCircle(e.sx, e.sy, 3, olc::RED);
      FillCircle(e.ex, e.ey, 3, olc::RED);
    }
    return true;
  }
};


int main ()
{
  olcShadowCasting2D demo;
  if (demo.Construct(640, 480, 1, 1))
    demo.Start();
  return 0;
}
