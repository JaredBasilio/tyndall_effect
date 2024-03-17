#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CGL {

bool BBox::intersect(const Ray& r, double& t0, double& t1) const {

  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bouding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.
    
    Vector3D dist_min = this->min - r.o;
    Vector3D dist_max = this->max - r.o;
    
    float t_near[3], t_far[3];
    float intersect_t_min = -std::numeric_limits<float>::max();
    float intersect_t_max = std::numeric_limits<float>::max();
    
    for (int i = 0; i < 3; ++i) {
        t_near[i] = std::min(dist_min[i] / r.d[i], dist_max[i] / r.d[i]);
        t_far[i] = std::max(dist_min[i] / r.d[i], dist_max[i] / r.d[i]);

        intersect_t_min = std::max(intersect_t_min, t_near[i]);
        intersect_t_max = std::min(intersect_t_max, t_far[i]);
    }
    
    t0 = intersect_t_min;
    t1 = intersect_t_max;
        
    if (intersect_t_min > intersect_t_max) {
        return false;
    }
    
    if (intersect_t_min > r.max_t || intersect_t_max < r.min_t) {
        return false;
    }
    
    return true;

}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();

}

std::ostream& operator<<(std::ostream& os, const BBox& b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
