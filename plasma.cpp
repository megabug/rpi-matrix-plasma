// vim: sw=2 ts=2 et

#include <led-matrix.h>

#include <array>
#include <chrono>
#include <cmath>
#include <csignal>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "fast_hsv2rgb.h"

static constexpr auto PANEL_CENTER_X = 64;
static constexpr auto PANEL_CENTER_Y = 32;

static constexpr auto RADIUS_SCALE = 3.5;
static constexpr auto ANGLE_DELTA_SCALE = 0.0001;
static constexpr auto SHIFT_DELTA = 0.01;

static constexpr auto GRADIENT_ZOOM = 2500;

static constexpr auto COLORS_LERP_SIZE = 256;
static constexpr auto HUES_PALETTE_SIZE = 2048;

class Point {
  const double radius;
  const double center_x;
  const double center_y;
  const double angle_delta;

  double angle;

 public:
  Point(const double radius, const double center_x, const double center_y,
        const double angle_delta)
      : radius(radius),
        center_x(center_x),
        center_y(center_y),
        angle_delta(angle_delta) {
    std::random_device rng_device;
    std::mt19937 rng(rng_device());
    angle =
        std::uniform_int_distribution<std::mt19937::result_type>(0, 1000)(rng);
  }

  inline double x() const {
    return PANEL_CENTER_X + center_x + std::cos(angle) * radius * RADIUS_SCALE;
  }

  inline double y() const {
    return PANEL_CENTER_Y + center_y + std::sin(angle) * radius * RADIUS_SCALE;
  }

  inline void tick() { angle += angle_delta * ANGLE_DELTA_SCALE; }
};

struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

static std::array<Point, 8> points = {
    Point{16.3, -16.1, 8.7, 3}, {23.0, 5.6, -6.5, -5},   {40.8, 23.4, 14.0, 7},
    {8.2, -24.1, -2.9, -11},    {34.3, -4.1, -16.1, 13}, {13.5, 12.1, 7.4, -17},
    {12.1, -2.3, 26.2, 19},     {42.1, -44.2, 5.3, -23}};

static auto shift = 0.0;

static std::vector<RGB> palette;

static void tick() {
  for (auto &point : points) {
    point.tick();
  }

  shift += SHIFT_DELTA;
}

static void render(rgb_matrix::Canvas &canvas) {
  for (auto y = 0; y < canvas.height(); ++y) {
    for (auto x = 0; x < canvas.width(); ++x) {
      auto value = shift;
      for (const auto &point : points) {
        const auto rx = point.x() - x;
        const auto ry = point.y() - y;
        value += std::cos((rx * rx + ry * ry) / GRADIENT_ZOOM);
      }

      const auto &c = palette[static_cast<size_t>((std::cos(value) + 1) / 2 *
                                                  palette.size())];
      canvas.SetPixel(x, y, c.r, c.g, c.b);
    }
  }
}

int main(int argc, char *argv[]) {
  const auto usage = [&argv]() {
    std::cerr << "Usage: " << argv[0]
              << " (palette FILENAME|colors R G B R G B [(R G B)...]|hues)"
              << std::endl;
  };

  if (argc < 1 + 1) {
    usage();
    return EXIT_FAILURE;
  }

  const std::string mode = argv[1];
  if (mode == "palette") {
    if (argc < 1 + 2) {
      usage();
      return EXIT_FAILURE;
    }

    std::ifstream file(argv[2]);
    if (!file.is_open()) {
      std::cerr << "Can't open file " << argv[2] << std::endl;
      return EXIT_FAILURE;
    }

    for (;;) {
      unsigned int r, g, b;
      file >> r >> g >> b;
      if (file.eof()) {
        break;
      }

      palette.push_back(RGB{static_cast<uint8_t>(r), static_cast<uint8_t>(g),
                            static_cast<uint8_t>(b)});
    }

    file.close();
  } else if (mode == "colors") {
    if (argc < 1 + 7 || (argc - 2) % 3) {
      usage();
      return EXIT_FAILURE;
    }

    RGB a = RGB{static_cast<uint8_t>(std::atoi(argv[2])),
                static_cast<uint8_t>(std::atoi(argv[3])),
                static_cast<uint8_t>(std::atoi(argv[4]))};
    RGB b;

    for (auto i = 5; i < argc; i += 3) {
      b = RGB{static_cast<uint8_t>(std::atoi(argv[i])),
              static_cast<uint8_t>(std::atoi(argv[i + 1])),
              static_cast<uint8_t>(std::atoi(argv[i + 2]))};

      for (auto j = 0; j < COLORS_LERP_SIZE; ++j) {
        palette.push_back(
            RGB{static_cast<uint8_t>(a.r + static_cast<double>(b.r - a.r) * j /
                                               COLORS_LERP_SIZE),
                static_cast<uint8_t>(a.g + static_cast<double>(b.g - a.g) * j /
                                               COLORS_LERP_SIZE),
                static_cast<uint8_t>(a.b + static_cast<double>(b.b - a.b) * j /
                                               COLORS_LERP_SIZE)});
      }

      a = b;
    }

    palette.push_back(b);
  } else if (mode == "hues") {
    for (auto i = 0; i < HUES_PALETTE_SIZE; ++i) {
      RGB rgb;
      fast_hsv2rgb_32bit(static_cast<uint16_t>(static_cast<double>(i) /
                                               HUES_PALETTE_SIZE * HSV_HUE_MAX),
                         255, 255, &rgb.r, &rgb.g, &rgb.b);
      palette.push_back(rgb);
    }
  } else {
    usage();
    return EXIT_FAILURE;
  }

  rgb_matrix::RGBMatrix::Options options;
  options.rows = 64;
  options.cols = 64;
  options.chain_length = 2;

  rgb_matrix::RuntimeOptions runtime_options;
  runtime_options.gpio_slowdown = 4;

  rgb_matrix::Canvas *const canvas =
      rgb_matrix::RGBMatrix::CreateFromOptions(options, runtime_options);
  if (!canvas) {
    return EXIT_FAILURE;
  }

  static volatile auto interrupted = false;
  std::signal(SIGTERM, [](int) { interrupted = true; });
  std::signal(SIGINT, [](int) { interrupted = true; });

  while (!interrupted) {
    tick();
    render(*canvas);

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  return EXIT_SUCCESS;
}
