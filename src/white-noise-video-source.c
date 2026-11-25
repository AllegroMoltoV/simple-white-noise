/*
Simple White Noise
Copyright (C) 2026 AllegroMoltoV

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "white-noise-video-source.h"

#include <obs-module.h>
#include <plugin-support.h>
#include <graphics/vec2.h>
#include <util/bmem.h>
#include <util/platform.h>

#include <stdint.h>

#define SETTING_USE_CANVAS_SIZE "use_canvas_size"
#define SETTING_WIDTH "width"
#define SETTING_HEIGHT "height"
#define SETTING_NO_SOUND_INFO "no_sound_info"
#define SOURCE_SIZE_MIN 1
#define SOURCE_SIZE_MAX 8192
#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080

// The shader hashes the seed together with pixel positions as 32-bit floats. Keeping the seed below 4096 keeps
// enough fractional precision for the hash.
#define SEED_RANGE 4096.0f

struct white_noise_video {
	gs_effect_t *effect;
	gs_eparam_t *param_size;
	gs_eparam_t *param_seed;
	bool use_canvas_size;
	uint32_t width;
	uint32_t height;
	uint32_t random_state;
	float seed;
};

// xorshift32, as in the audio source
static inline uint32_t next_random(uint32_t *state)
{
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return x;
}

static float next_seed(uint32_t *state)
{
	// The top 24 bits fit a float exactly, so the seed is spread evenly over [0, SEED_RANGE)
	return (float)(next_random(state) >> 8) * (SEED_RANGE / 16777216.0f);
}

static uint32_t clamp_size(long long value)
{
	if (value < SOURCE_SIZE_MIN)
		return SOURCE_SIZE_MIN;
	if (value > SOURCE_SIZE_MAX)
		return SOURCE_SIZE_MAX;
	return (uint32_t)value;
}

static const char *white_noise_video_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("WhiteNoiseVideo");
}

static void white_noise_video_update(void *data, obs_data_t *settings)
{
	struct white_noise_video *wn = data;
	wn->use_canvas_size = obs_data_get_bool(settings, SETTING_USE_CANVAS_SIZE);
	wn->width = clamp_size(obs_data_get_int(settings, SETTING_WIDTH));
	wn->height = clamp_size(obs_data_get_int(settings, SETTING_HEIGHT));
}

static void white_noise_video_destroy(void *data)
{
	struct white_noise_video *wn = data;
	if (!wn)
		return;
	obs_enter_graphics();
	gs_effect_destroy(wn->effect);
	obs_leave_graphics();
	bfree(wn);
}

static void *white_noise_video_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(source);
	struct white_noise_video *wn = bzalloc(sizeof(struct white_noise_video));
	// Different sources get different noise
	wn->random_state = (uint32_t)(os_gettime_ns() ^ (uint64_t)(uintptr_t)wn) | 1u;
	wn->seed = next_seed(&wn->random_state);

	obs_enter_graphics();
	char *path = obs_module_file("white-noise.effect");
	if (path) {
		wn->effect = gs_effect_create_from_file(path, NULL);
		bfree(path);
	}
	if (wn->effect) {
		wn->param_size = gs_effect_get_param_by_name(wn->effect, "size");
		wn->param_seed = gs_effect_get_param_by_name(wn->effect, "seed");
	} else {
		obs_log(LOG_WARNING, "failed to load the effect white-noise.effect");
	}
	obs_leave_graphics();

	white_noise_video_update(wn, settings);
	return wn;
}

static uint32_t white_noise_video_get_width(void *data)
{
	struct white_noise_video *wn = data;
	if (!wn->use_canvas_size)
		return wn->width;
	struct obs_video_info ovi;
	return obs_get_video_info(&ovi) ? ovi.base_width : DEFAULT_WIDTH;
}

static uint32_t white_noise_video_get_height(void *data)
{
	struct white_noise_video *wn = data;
	if (!wn->use_canvas_size)
		return wn->height;
	struct obs_video_info ovi;
	return obs_get_video_info(&ovi) ? ovi.base_height : DEFAULT_HEIGHT;
}

static void white_noise_video_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);
	struct white_noise_video *wn = data;
	// The seed changes only here. video_render can run several times in one frame and then draws the same image.
	wn->seed = next_seed(&wn->random_state);
}

static void white_noise_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	struct white_noise_video *wn = data;
	if (!wn->effect)
		return;

	const uint32_t width = white_noise_video_get_width(wn);
	const uint32_t height = white_noise_video_get_height(wn);
	struct vec2 size;
	vec2_set(&size, (float)width, (float)height);
	gs_effect_set_vec2(wn->param_size, &size);
	gs_effect_set_float(wn->param_seed, wn->seed);

	// The hash value is written as is, without the sRGB conversion of the framebuffer
	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(false);
	while (gs_effect_loop(wn->effect, "Draw"))
		gs_draw_sprite(NULL, 0, width, height);
	gs_enable_framebuffer_srgb(previous);
}

static void white_noise_video_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, SETTING_USE_CANVAS_SIZE, true);
	obs_data_set_default_int(settings, SETTING_WIDTH, DEFAULT_WIDTH);
	obs_data_set_default_int(settings, SETTING_HEIGHT, DEFAULT_HEIGHT);
}

static bool use_canvas_size_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings)
{
	UNUSED_PARAMETER(property);
	const bool custom = !obs_data_get_bool(settings, SETTING_USE_CANVAS_SIZE);
	obs_property_set_visible(obs_properties_get(props, SETTING_WIDTH), custom);
	obs_property_set_visible(obs_properties_get(props, SETTING_HEIGHT), custom);
	return true;
}

static obs_properties_t *white_noise_video_get_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_text(props, SETTING_NO_SOUND_INFO, obs_module_text("NoSoundInfo"), OBS_TEXT_INFO);
	obs_property_t *p = obs_properties_add_bool(props, SETTING_USE_CANVAS_SIZE, obs_module_text("UseCanvasSize"));
	obs_property_set_modified_callback(p, use_canvas_size_modified);
	obs_properties_add_int(props, SETTING_WIDTH, obs_module_text("Width"), SOURCE_SIZE_MIN, SOURCE_SIZE_MAX, 1);
	obs_properties_add_int(props, SETTING_HEIGHT, obs_module_text("Height"), SOURCE_SIZE_MIN, SOURCE_SIZE_MAX, 1);
	return props;
}

struct obs_source_info white_noise_video_source_info = {
	.id = "simple_white_noise_video_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW,
	.icon_type = OBS_ICON_TYPE_COLOR,
	.get_name = white_noise_video_get_name,
	.create = white_noise_video_create,
	.destroy = white_noise_video_destroy,
	.update = white_noise_video_update,
	.get_defaults = white_noise_video_get_defaults,
	.get_properties = white_noise_video_get_properties,
	.video_tick = white_noise_video_tick,
	.video_render = white_noise_video_render,
	.get_width = white_noise_video_get_width,
	.get_height = white_noise_video_get_height,
};
