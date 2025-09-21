#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define NUM_SPHERES (sizeof(scene) / sizeof(Sphere))

typedef struct {
  double x, y, z;
} Vec3;
typedef struct {
  Vec3 origin, direction;
} Ray;
typedef struct {
  Vec3 center;
  double radius;
  Vec3 color;
  Vec3 emission;
} Sphere;
Vec3 vec_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 vec_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 vec_mul_scalar(Vec3 v, double s) {
  return (Vec3){v.x * s, v.y * s, v.z * s};
}
Vec3 vec_mul(Vec3 a, Vec3 b) { return (Vec3){a.x * b.x, a.y * b.y, a.z * b.z}; }
double vec_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 vec_normalize(Vec3 v) {
  double mag = sqrt(vec_dot(v, v));
  return vec_mul_scalar(v, 1.0 / mag);
}
Sphere scene[] = {
    {{50.0, 72.0, 81.6}, 5.0, {0.0, 0.0, 0.0}, {12.0, 12.0, 12.0}},
    {{50.0, -1000.0, 81.6}, 1000.0, {0.75, 0.75, 0.75}, {0.0, 0.0, 0.0}},
    {{27.0, 32.5, 47.0}, 16.5, {0.75, 0.25, 0.25}, {0.0, 0.0, 0.0}},
    {{73.0, 32.5, 78.0}, 16.5, {0.25, 0.25, 0.75}, {0.0, 0.0, 0.0}},
};

double intersect(Ray r, int *id) {
  double min_t = 1e20;
  *id = -1;
  for (int i = 0; i < NUM_SPHERES; i++) {
    Vec3 op = vec_sub(scene[i].center, r.origin);
    double b = vec_dot(op, r.direction);
    double det = b * b - vec_dot(op, op) + scene[i].radius * scene[i].radius;
    if (det < 0)
      continue;
    det = sqrt(det);
    double t = b - det;
    if (t > 1e-4 && t < min_t) {
      min_t = t;
      *id = i;
    }
    t = b + det;
    if (t > 1e-4 && t < min_t) {
      min_t = t;
      *id = i;
    }
  }
  return min_t < 1e20 ? min_t : 0;
}

Vec3 trace(Ray r, int depth, unsigned int *seed) {
  int id;
  double t = intersect(r, &id);
  if (t == 0)
    return (Vec3){0, 0, 0};
  const Sphere *obj = &scene[id];
  Vec3 hit_point = vec_add(r.origin, vec_mul_scalar(r.direction, t));
  Vec3 normal = vec_normalize(vec_sub(hit_point, obj->center));
  if (depth > 5)
    return obj->emission;
  double r1 = 2 * M_PI * ((double)rand_r(seed) / RAND_MAX);
  double r2 = ((double)rand_r(seed) / RAND_MAX);
  double r2s = sqrt(r2);
  Vec3 w = normal;
  Vec3 u = vec_normalize(vec_dot(w, (Vec3){0, 0, 1}) > 0.1 ? (Vec3){0, 1, 0}
                                                           : (Vec3){1, 0, 0});
  Vec3 v = (Vec3){w.y * u.z - w.z * u.y, w.z * u.x - w.x * u.z,
                  w.x * u.y - w.y * u.x};
  Vec3 new_dir =
      vec_normalize(vec_add(vec_mul_scalar(u, cos(r1) * r2s),
                            vec_add(vec_mul_scalar(v, sin(r1) * r2s),
                                    vec_mul_scalar(w, sqrt(1 - r2)))));
  Vec3 raytrace = trace((Ray){hit_point, new_dir}, depth + 1, seed);
  return vec_add(obj->emission, vec_mul(obj->color, raytrace));
}

int main(int argc, char **argv) {
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  int samples = atoi(argv[3]);
  char *path = argv[4];

  Vec3 camera_direction = {0, -0.042612, -1};
  Ray camera = {{50, 52, 295.6}, vec_normalize(camera_direction)};
  Vec3 cx = {width * 0.5135 / height, 0, 0};
  Vec3 cy = vec_normalize(
      (Vec3){cx.z * camera.direction.y - cx.y * camera.direction.z,
             cx.x * camera.direction.z - cx.z * camera.direction.x,
             cx.y * camera.direction.x - cx.x * camera.direction.y});
  cy = vec_mul_scalar(cy, -0.5135);
  Vec3 *image = (Vec3 *)calloc(sizeof(Vec3), width * height);
  printf("Rendering a %d x %d image with %d samples per pixel...\n", width,
         height, samples);
  double before = omp_get_wtime();

#pragma omp parallel
#pragma omp single
  {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        unsigned int seed =
            (unsigned int)(omp_get_wtime() * 1000) + omp_get_thread_num();
        int index = (height - 1 - y) * width + x;

#pragma omp task
        {
          for (int s = 0; s < samples; s++) {
            double r1 = 2 * ((double)rand_r(&seed) / RAND_MAX) - 1;
            double r2 = 2 * ((double)rand_r(&seed) / RAND_MAX) - 1;
            Vec3 offset =
                vec_add(vec_mul_scalar(cx, (r1 + x - width / 2.0) / width),
                        vec_mul_scalar(cy, (r2 + y - height / 2.0) / height));
            Ray ray = {vec_add(camera.origin, offset),
                       vec_normalize(vec_add(camera.direction, offset))};
            image[index] = vec_add(image[index], trace(ray, 0, &seed));
          }
          image[index] = vec_mul_scalar(image[index], 1.0 / samples);
        }
      }
    }
  }
#pragma omp taskwait
  double after = omp_get_wtime();
  printf("\nDone rendering. Time: %lf seconds.\n", after - before);
  printf("Saving image to render.png...\n");
  const int channels = 3;
  unsigned char *png_data = (unsigned char *)malloc(width * height * channels);

#pragma omp parallel for
  for (int i = 0; i < width * height; i++) {
    int r = (int)(pow(fmin(1.0, image[i].x), 1 / 2.2) * 255 + 0.5);
    int g = (int)(pow(fmin(1.0, image[i].y), 1 / 2.2) * 255 + 0.5);
    int b = (int)(pow(fmin(1.0, image[i].z), 1 / 2.2) * 255 + 0.5);
    png_data[i * channels + 0] = r;
    png_data[i * channels + 1] = g;
    png_data[i * channels + 2] = b;
  }
  int success = stbi_write_png(path, width, height, channels, png_data,
                               width * channels);
  if (success) {
    printf("Image saved successfully.\n");
  } else {
    printf("Error saving image.\n");
  }
  free(image);
  free(png_data);
  return 0;
}
