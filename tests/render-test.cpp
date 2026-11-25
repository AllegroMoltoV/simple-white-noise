// Render test for the white noise video source.
// It creates the source the way OBS does when a user adds it, renders it, and checks the pixels.
//
// Usage: render-test <plugin module> <plugin data folder> <graphics module> <libobs data folder> <scenario>
// Scenarios:
//   noise-stats         The output is opaque gray noise with uniformly distributed levels and no pattern between
//                       neighboring pixels.
//   changes-each-frame  The noise of one frame is unrelated to the noise a few frames later.
//   custom-size         The source matches the canvas size by default and uses the width and height settings when
//                       "Match canvas size" is off.
// Exit code: 0 when all checks pass, 1 when a check fails, 2 when the test cannot start.
//
// Thresholds: the canvas has 160 x 120 = 19200 pixels. For independent uniform levels 0 to 255, the mean is 127.5
// with a standard error of 73.9 / sqrt(19200) = 0.53, and a correlation has a standard error of about
// 1 / sqrt(19200) = 0.007. The tolerances below are several times larger, so random noise does not fail them, while
// a broken hash with stripes or a repeated pattern does.

#include "render-fixture.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <set>
#include <sstream>

namespace {

using namespace render_test;

constexpr const char *kSourceId = "simple_white_noise_video_source";

constexpr uint32_t kCanvasWidth = 160;
constexpr uint32_t kCanvasHeight = 120;

// Standard deviation of uniform integer levels 0 to 255
const double kUniformStdDev = std::sqrt((256.0 * 256.0 - 1.0) / 12.0);

// Gray levels of the red channel, row by row
std::vector<double> levels(const Image &image)
{
	std::vector<double> out;
	out.reserve(static_cast<size_t>(image.width) * image.height);
	for (uint32_t y = 0; y < image.height; ++y) {
		for (uint32_t x = 0; x < image.width; ++x) {
			out.push_back(image.at(x, y)[0]);
		}
	}
	return out;
}

double correlation(const std::vector<double> &a, const std::vector<double> &b)
{
	const size_t n = std::min(a.size(), b.size());
	double mean_a = 0.0, mean_b = 0.0;
	for (size_t i = 0; i < n; ++i) {
		mean_a += a[i];
		mean_b += b[i];
	}
	mean_a /= double(n);
	mean_b /= double(n);
	double ab = 0.0, aa = 0.0, bb = 0.0;
	for (size_t i = 0; i < n; ++i) {
		ab += (a[i] - mean_a) * (b[i] - mean_b);
		aa += (a[i] - mean_a) * (a[i] - mean_a);
		bb += (b[i] - mean_b) * (b[i] - mean_b);
	}
	return (aa > 0.0 && bb > 0.0) ? ab / std::sqrt(aa * bb) : 1.0;
}

std::string format(const char *label, double value)
{
	std::ostringstream s;
	s << label << ' ' << value;
	return s.str();
}

obs_source_t *create_noise_source(obs_data_t *settings = nullptr)
{
	return obs_source_create(kSourceId, "white noise video", settings, nullptr);
}

// Renders twice and keeps the second image. On macOS, the first image read back in a process can keep earlier
// content, and the other platforms do not mind the extra render.
bool render_settled(obs_source_t *source, Image &image)
{
	Image warmup;
	return render(source, warmup) && render(source, image);
}

void scenario_noise_stats(Checker &c)
{
	obs_source_t *source = create_noise_source();
	Image image;
	if (!source) {
		c.expect(false, "failed to create the source");
	} else if (!wait_frames(2) || !render_settled(source, image)) {
		c.expect(false, "failed to render the source");
	} else {
		save_image("noise-stats", image);
		c.expect(image.width == kCanvasWidth && image.height == kCanvasHeight,
			 "output size is not the canvas size");

		bool opaque = true, gray = true;
		for (size_t i = 0; i + 3 < image.rgba.size(); i += 4) {
			opaque = opaque && image.rgba[i + 3] == 255;
			gray = gray && std::abs(image.rgba[i] - image.rgba[i + 1]) <= 1 &&
			       std::abs(image.rgba[i] - image.rgba[i + 2]) <= 1;
		}
		c.expect(opaque, "the output is not opaque");
		c.expect(gray, "the output is not gray");

		const std::vector<double> v = levels(image);
		double mean = 0.0;
		for (double x : v) {
			mean += x;
		}
		mean /= double(v.size());
		double variance = 0.0;
		for (double x : v) {
			variance += (x - mean) * (x - mean);
		}
		const double stddev = std::sqrt(variance / double(v.size()));
		c.expect(std::fabs(mean - 127.5) <= 5.0, format("mean level is not near 127.5:", mean));
		c.expect(std::fabs(stddev - kUniformStdDev) <= kUniformStdDev * 0.1,
			 format("standard deviation of the levels is not near 73.9:", stddev));
		const std::set<double> distinct(v.begin(), v.end());
		c.expect(distinct.size() >= 200, format("too few distinct levels:", double(distinct.size())));

		// Stripes, grids, and repeated tiles show up as correlation between neighboring pixels
		std::vector<double> left, right, upper, lower;
		for (uint32_t y = 0; y < image.height; ++y) {
			for (uint32_t x = 0; x < image.width; ++x) {
				const double level = v[static_cast<size_t>(y) * image.width + x];
				if (x + 1 < image.width) {
					left.push_back(level);
					right.push_back(v[static_cast<size_t>(y) * image.width + x + 1]);
				}
				if (y + 1 < image.height) {
					upper.push_back(level);
					lower.push_back(v[static_cast<size_t>(y + 1) * image.width + x]);
				}
			}
		}
		const double horizontal = correlation(left, right);
		const double vertical = correlation(upper, lower);
		c.expect(std::fabs(horizontal) < 0.05, format("horizontal neighbors are correlated:", horizontal));
		c.expect(std::fabs(vertical) < 0.05, format("vertical neighbors are correlated:", vertical));
	}
	obs_source_release(source);
}

void scenario_changes_each_frame(Checker &c)
{
	obs_source_t *source = create_noise_source();
	Image first, later;
	if (!source) {
		c.expect(false, "failed to create the source");
	} else if (!wait_frames(2) || !render_settled(source, first)) {
		c.expect(false, "failed to render the first frame");
	} else if (!wait_frames(3) || !render(source, later)) {
		c.expect(false, "failed to render the later frame");
	} else {
		save_image("changes-each-frame-first", first);
		save_image("changes-each-frame-later", later);
		const double r = correlation(levels(first), levels(later));
		c.expect(std::fabs(r) < 0.05, format("the later frame is correlated with the first frame:", r));
	}
	obs_source_release(source);
}

void scenario_custom_size(Checker &c)
{
	obs_source_t *source = create_noise_source();
	if (!source) {
		c.expect(false, "failed to create the source");
		return;
	}
	c.expect(obs_source_get_width(source) == kCanvasWidth && obs_source_get_height(source) == kCanvasHeight,
		 "the default size is not the canvas size");

	obs_data_t *settings = obs_data_create();
	obs_data_set_bool(settings, "use_canvas_size", false);
	obs_data_set_int(settings, "width", 50);
	obs_data_set_int(settings, "height", 30);
	obs_source_update(source, settings);
	obs_data_release(settings);

	Image image;
	if (!wait_frames(2) || !render_settled(source, image)) {
		c.expect(false, "failed to render the source");
	} else {
		save_image("custom-size", image);
		c.expect(obs_source_get_width(source) == 50 && obs_source_get_height(source) == 30,
			 "the source does not use the width and height settings");
		c.expect(image.width == 50 && image.height == 30, "the output is not 50 x 30");
		bool opaque = !image.rgba.empty();
		for (size_t i = 3; i < image.rgba.size(); i += 4) {
			opaque = opaque && image.rgba[i] == 255;
		}
		c.expect(opaque, "the custom size output is not opaque");
	}
	obs_source_release(source);
}

struct Scenario {
	const char *name;
	std::function<void(Checker &)> run;
};

const Scenario scenarios[] = {
	{"noise-stats", scenario_noise_stats},
	{"changes-each-frame", scenario_changes_each_frame},
	{"custom-size", scenario_custom_size},
};

} // namespace

int main(int argc, char **argv)
{
	if (argc != 6) {
		std::cerr << "usage: render-test <plugin> <plugin data> <graphics module> <libobs data> <scenario>\n";
		return 2;
	}
	const Scenario *scenario = nullptr;
	for (const Scenario &s : scenarios) {
		if (std::strcmp(argv[5], s.name) == 0) {
			scenario = &s;
		}
	}
	if (!scenario) {
		std::cerr << "unknown scenario: " << argv[5] << '\n';
		return 2;
	}
	if (!obs_test::start(argv[3], argv[4], kCanvasWidth, kCanvasHeight)) {
		return 2;
	}
	int result = 2;
	if (obs_test::load_plugin(argv[1], argv[2])) {
		Checker checker;
		scenario->run(checker);
		result = checker.failures == 0 ? 0 : 1;
		if (result != 0) {
			std::cerr << checker.failures << " check(s) failed in " << scenario->name << '\n';
		} else {
			std::cout << "PASS " << scenario->name << '\n';
		}
	}
	obs_shutdown();
	return result;
}
