#include "pathtracer.h"

#include "pathtracer/noise.h"
#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"
#include "vector3D.h"

#include <algorithm>
#define msg(s) cerr << "[PathTracer] " << s << endl;

using namespace CGL::SceneObjects;

namespace CGL {

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
  PerlinNoise noise;
  P_absorb = 0.1;
  P_scatter = 0.3;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling,
  // you may find the "glow" around the light source is gone. This is totally
  // fine: the area lights in importance sampling has directionality, however in
  // hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out(0, 0, 0);

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  for (int i = 0; i < num_samples; i++) {
    Vector3D sample_direction = hemisphereSampler->get_sample();
    Vector3D sample_dir_obj = o2w * sample_direction;

    Vector3D offset_hit_p = hit_p + isect.n * EPS_D;
    Ray sample_ray = Ray(offset_hit_p, sample_dir_obj);
    sample_ray.min_t = EPS_F;
    sample_ray.max_t = std::numeric_limits<float>::infinity();

    Intersection sample_isect;

    if (bvh->intersect(sample_ray, &sample_isect)) {

      Vector3D sample_ray_L = sample_isect.bsdf->get_emission();
      float cosine = dot(isect.n, sample_dir_obj);
      Vector3D bsdf_value = isect.bsdf->f(w_out, sample_direction);

      float pdf = 1.0 / (2 * M_PI);
      L_out += sample_ray_L * bsdf_value * cosine / pdf;
    }
  }

  if (num_samples != 0) {
    return L_out / num_samples;
  }

  return Vector3D(0.5, 0.5, 0.5);
}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out(0, 0, 0);

  // TODO (Part 3): Write your sampling loop here
  // TODO BEFORE YOU BEGIN
  // UPDATE `est_radiance_global_illumination` to return direct lighting instead
  // of normal shading

  for (size_t i = 0; i < scene->lights.size(); i++) {
    Vector3D L_out_per_light(0, 0, 0);

    int num_sample_per_light = 1;
    if (!scene->lights[i]->is_delta_light()) {
      num_sample_per_light = ns_area_light;
    }

    for (int j = 0; j < num_sample_per_light; j++) {
      Vector3D wi;
      double distToLight, pdf;

      Vector3D L = scene->lights[i]->sample_L(hit_p, &wi, &distToLight, &pdf);

      if (pdf <= 0)
        continue;

      Vector3D sample_dir_obj = wi;

      Vector3D offset_hit_p = hit_p + isect.n * EPS_D;
      Ray sample_ray = Ray(hit_p, sample_dir_obj, 1);
      sample_ray.min_t = EPS_F;
      sample_ray.max_t = std::numeric_limits<float>::infinity();

      Intersection sample_isect;

      float cosine = dot(isect.n, sample_dir_obj);
      if (cosine > 0) { // Negative cosine value means no light
        if (!bvh->intersect(sample_ray, &sample_isect)) {
          Vector3D bsdf_value = isect.bsdf->f(w_out, wi);
          L_out_per_light += L * bsdf_value * cosine / pdf;
        } else {
          Vector3D sample_ray_L = sample_isect.bsdf->get_emission();
          Vector3D bsdf_value = isect.bsdf->f(w_out, wi);

          L_out_per_light += sample_ray_L * bsdf_value * cosine / pdf;
        }
      }
    }

    L_out += L_out_per_light / num_sample_per_light;
  }

  if (scene->lights.size() != 0) {
    return L_out;
  }

  return Vector3D(0.5, 0.5, 0.5);
}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
  // TODO: Part 3, Task 2
  // Returns the light that results from no bounces of light
  if (isect.bsdf) {
    return isect.bsdf->get_emission();
  }
  return Vector3D(0, 0, 0);
}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`

  return estimate_direct_lighting_importance(r, isect);
}

Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
                                                  const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  Vector3D L_out(0, 0, 0);

  bool stop_roulette = coin_flip(0);

  if (r.depth == 1 || stop_roulette) {
    return estimate_direct_lighting_importance(r, isect);
  }

  if (isAccumBounces) {
    L_out = estimate_direct_lighting_importance(r, isect);
  }

  Vector3D wi;
  double pdf;

  Vector3D bsdf_val = isect.bsdf->sample_f(w_out, &wi, &pdf);

  Vector3D offset_hit_p = hit_p + EPS_D * (o2w * wi);
  Ray sample_ray = Ray(hit_p, o2w * wi);
  sample_ray.depth = r.depth - 1;
  sample_ray.min_t = EPS_F;
  sample_ray.max_t = std::numeric_limits<float>::infinity();

  Intersection sample_isect;

  if (bvh->intersect(sample_ray, &sample_isect)) {

    float cosine = dot(isect.n, o2w * wi);
    if (cosine > 0) {
      // Recursive call to get radiance for the next bounce
      L_out += at_least_one_bounce_radiance(sample_ray, sample_isect) * cosine *
               bsdf_val / pdf;
    }
  }

  return L_out;
}

Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  if (!bvh->intersect(r, &isect))
    return envLight ? envLight->sample_dir(r) : L_out;

  // L_out = (isect.t == INF_D) ? debug_shading(r.d) : normal_shading(isect.n);

  double interval = 0.05; // when homogeneous this was  double interval = 0.005;
  Vector3D marchDirection = r.d.unit();
  double steps = isect.t / 0.05;
  double beta = 0.2;
  double red_transimission_rate = 0.1;
  double attenuation = 1;
  int hit_count = 0;
  bool hit = false;
  double avg_density = 0;
  Vector3D L_light;
  double distToLight, pdf;
  double tau = 0.5;
  for (double i = 0; i < isect.t; i += interval) {

    Vector3D sample_pos = r.o + i * marchDirection;
    //  std::cout << "y _position: " << sample_pos.y << std::endl;
    if (sample_pos.y <= 0.75) {
      continue; // only one upper half area to be cloud
    }
    double density = (noise.eval(sample_pos * 2.5) + 1);
    // std::cout << "density: " << density << std::endl;
    double hit_prob = std::max((1 / density) / 10000, 0.0);
    // std::cout << "Hit probability: " << hit_prob << std::endl;
    if (coin_flip(hit_prob)) {
      hit = true;
      hit_count += 1;
      isect.t = i;
      isect.bsdf = new FogBSDF(0.2);

      Vector3D hit_p = r.o + r.d * isect.t;
      // assume there is only one light, no need loop
      // for (int j = 0; j < scene->lights.size(); j++) {
      //   Vector3D L_out_light(0, 0, 0);
      Vector3D wi;

        L_light = scene->lights[0]->sample_L(hit_p, &wi, &distToLight, &pdf); //wi is in world space, unit vector of hit point towards light source
        double sample_rate = 0.05;
        for (double t = 0; t <= distToLight; t+=sample_rate){
        double density = (noise.eval(sample_pos * 2.5) + 1);
        avg_density += density;
        avg_density = avg_density / steps;
        tau = avg_density*50; // try to make tau close to 1
        // std::cout << "tau: " << tau << std::endl;
        attenuation = std::exp(-beta * std::pow(distToLight*tau + 0.5, 4) - 0.7);
        }
        // std::cout << "distToLight1: " << distToLight << std::endl;
        // std::exp(-beta * std::pow(distToLight + 0.7, 9) - 0.7);
      // }
      break;
    }
  }

  // std::cout << "attenuation: " << attenuation << std::endl;
  // std::cout << "beta: " << beta << std::endl;
  // std::cout << "distToLight: " << distToLight << std::endl;
  attenuation = std::min(std::max(attenuation, 0.0), 1.0);
  auto red = L_light.x;
  red += hit_count * red_transimission_rate * L_light.x;
  auto green = L_light.y;
  auto blue = L_light.z;
  Vector3D rayleigh_scattering_color = Vector3D(red * 0.83, green, blue * 1.2);
  L_out += rayleigh_scattering_color * attenuation;
  Vector3D ambient_light(0.1, 0.1, 0.1); // Low intensity ambient light

  // Compute lighting with potentially modified intersection due to fog
  L_out += zero_bounce_radiance(r, isect) +
           at_least_one_bounce_radiance(r, isect) + ambient_light;

  // L_out = one_bounce_radiance(r, isect) + zero_bounce_radiance(r, isect);
  // L_out = zero_bounce_radiance(r, isect) + at_least_one_bounce_radiance(r,
  // isect);

  return L_out;
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  int num_samples = samplesPerBatch; // total samples to evaluate
  Vector2D origin = Vector2D(x, y);  // bottom left corner of the pixel

  float width = sampleBuffer.w;
  float height = sampleBuffer.h;

  Vector3D color_acc = Vector3D(0.0, 0.0, 0.0);
  float s1 = 0;
  float s2 = 0;

  float i_val = 1;
  float avg = 0;
  while (num_samples <= ns_aa) {
    for (int i = 0; i < samplesPerBatch; i++) {
      Vector2D sample = gridSampler->get_sample();
      Ray ray =
          camera->generate_ray((x + sample.x) / width, (y + sample.y) / height);
      ray.depth = max_ray_depth;
      Vector3D sample_color = est_radiance_global_illumination(ray);
      color_acc += sample_color;

      float illum = sample_color.illum();
      s1 += illum;
      s2 += illum * illum;
    }

    avg = s1 / num_samples;
    float sigma = (s2 - (s1 * s1 / num_samples)) / (num_samples - 1);
    i_val = 1.96 * sqrt(sigma) / sqrt(num_samples);

    num_samples += samplesPerBatch;

    // if (i_val <= 0.05 * avg || num_samples > ns_aa) {
    //     break;
    // }
  }

  sampleBuffer.update_pixel(color_acc / num_samples, x, y);
  sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;
}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL