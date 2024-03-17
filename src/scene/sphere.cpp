#include "sphere.h"

#include <cmath>

#include "pathtracer/bsdf.h"
#include "util/sphere_drawing.h"

namespace CGL {
namespace SceneObjects {

bool Sphere::test(const Ray &r, double &t1, double &t2) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection test.
  // Return true if there are intersections and writing the
  // smaller of the two intersection times in t1 and the larger in t2.

  return true;

}

bool Sphere::has_intersection(const Ray &r) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note that you might want to use the the Sphere::test helper here.


  return true;
}

bool Sphere::intersect(const Ray &r, Intersection *i) const {

  // TODO (Part 1.4):
  // Implement ray - sphere intersection.
  // Note again that you might want to use the the Sphere::test helper here.
  // When an intersection takes place, the Intersection data should be updated
  // correspondingly.
        
    float a = dot(r.d, r.d);
    float b = 2 * dot(r.o - o, r.d);
    float c = dot(r.o - o, r.o - o) - this->r * this->r;
    
    float delta = b * b - 4 * a * c;
    
    if (delta < 0) { // No real root
        return false;
    }
    
    float t = (-b - sqrt(delta)) / 2 * a;
    
    if (t < 0 || t < r.min_t || t > r.max_t) {
        return false;
    }
    
    Vector3D hit_point = r.o + r.d * t;
    
    r.max_t = t;
    i->t = t;
    i->n = normal(hit_point);
    i->primitive = this;
    i->bsdf = get_bsdf();
    return true;
}

void Sphere::draw(const Color &c, float alpha) const {
  Misc::draw_sphere_opengl(o, r, c);
}

void Sphere::drawOutline(const Color &c, float alpha) const {
  // Misc::draw_sphere_opengl(o, r, c);
}

} // namespace SceneObjects
} // namespace CGL
