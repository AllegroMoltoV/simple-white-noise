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

#include "white-noise-source.h"

#include <obs-module.h>
#include <plugin-support.h>
#include <util/bmem.h>
#include <util/platform.h>
#include <util/threading.h>
#include <util/util_uint64.h>

#include <errno.h>
#include <math.h>
#include <stdint.h>

#define SETTING_LEVEL "level_db"
#define LEVEL_MIN_DB -60.0
#define LEVEL_MAX_DB 0.0
#define LEVEL_DEFAULT_DB -20.0

// The source always generates 48 kHz stereo. OBS resamples it when its own output uses another sample rate or
// speaker layout. Each chunk is 10 ms long.
#define SAMPLE_RATE 48000
#define CHUNK_FRAMES 480
#define CHANNELS 2

// The amplitude is shared between the settings thread and the generator thread. OBS offers atomics for long values
// only, so the amplitude is stored in millionths.
#define AMPLITUDE_SCALE 1000000.0

struct white_noise {
	obs_source_t *source;
	pthread_t thread;
	os_event_t *stop_event;
	bool thread_created;
	volatile long amplitude_millionths;
	uint32_t seed;
};

// xorshift32. Fast, allocation free, and good enough for audible noise.
static inline uint32_t next_random(uint32_t *state)
{
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return x;
}

// Maps a random value to a uniform sample in [-1, 1).
static inline float random_sample(uint32_t *state)
{
	return (float)(int32_t)next_random(state) * (1.0f / 2147483648.0f);
}

static long level_db_to_amplitude_millionths(double level_db)
{
	if (level_db < LEVEL_MIN_DB)
		level_db = LEVEL_MIN_DB;
	if (level_db > LEVEL_MAX_DB)
		level_db = LEVEL_MAX_DB;
	return (long)(pow(10.0, level_db / 20.0) * AMPLITUDE_SCALE + 0.5);
}

static void *noise_thread(void *data)
{
	struct white_noise *wn = data;
	float samples[CHANNELS][CHUNK_FRAMES];
	uint32_t state = wn->seed;
	uint64_t start_ns = os_gettime_ns();
	uint64_t frames_sent = 0;

	os_set_thread_name("simple-white-noise");

	while (os_event_try(wn->stop_event) == EAGAIN) {
		const float amplitude =
			(float)((double)os_atomic_load_long(&wn->amplitude_millionths) / AMPLITUDE_SCALE);

		for (size_t ch = 0; ch < CHANNELS; ch++) {
			for (size_t i = 0; i < CHUNK_FRAMES; i++)
				samples[ch][i] = amplitude * random_sample(&state);
		}

		struct obs_source_audio audio = {0};
		for (size_t ch = 0; ch < CHANNELS; ch++)
			audio.data[ch] = (const uint8_t *)samples[ch];
		audio.frames = CHUNK_FRAMES;
		audio.speakers = SPEAKERS_STEREO;
		audio.format = AUDIO_FORMAT_FLOAT_PLANAR;
		audio.samples_per_sec = SAMPLE_RATE;
		audio.timestamp = start_ns + util_mul_div64(frames_sent, 1000000000ULL, SAMPLE_RATE);
		obs_source_output_audio(wn->source, &audio);

		frames_sent += CHUNK_FRAMES;
		os_sleepto_ns(start_ns + util_mul_div64(frames_sent, 1000000000ULL, SAMPLE_RATE));
	}

	return NULL;
}

static const char *white_noise_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("WhiteNoise");
}

static void white_noise_update(void *data, obs_data_t *settings)
{
	struct white_noise *wn = data;
	os_atomic_set_long(&wn->amplitude_millionths,
			   level_db_to_amplitude_millionths(obs_data_get_double(settings, SETTING_LEVEL)));
}

static void white_noise_destroy(void *data)
{
	struct white_noise *wn = data;
	if (!wn)
		return;

	if (wn->thread_created) {
		os_event_signal(wn->stop_event);
		pthread_join(wn->thread, NULL);
	}
	if (wn->stop_event)
		os_event_destroy(wn->stop_event);
	bfree(wn);
}

static void *white_noise_create(obs_data_t *settings, obs_source_t *source)
{
	struct white_noise *wn = bzalloc(sizeof(struct white_noise));
	wn->source = source;
	// Different sources get different noise, so that two copies do not add up to the same signal
	wn->seed = (uint32_t)(os_gettime_ns() ^ (uint64_t)(uintptr_t)wn) | 1u;
	white_noise_update(wn, settings);

	if (os_event_init(&wn->stop_event, OS_EVENT_TYPE_MANUAL) != 0) {
		obs_log(LOG_ERROR, "failed to create the stop event");
		white_noise_destroy(wn);
		return NULL;
	}
	if (pthread_create(&wn->thread, NULL, noise_thread, wn) != 0) {
		obs_log(LOG_ERROR, "failed to start the noise thread");
		white_noise_destroy(wn);
		return NULL;
	}
	wn->thread_created = true;
	return wn;
}

static void white_noise_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, SETTING_LEVEL, LEVEL_DEFAULT_DB);
}

static obs_properties_t *white_noise_get_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_property_t *level = obs_properties_add_float_slider(props, SETTING_LEVEL, obs_module_text("Level"),
								LEVEL_MIN_DB, LEVEL_MAX_DB, 0.1);
	obs_property_float_set_suffix(level, " dB");
	return props;
}

struct obs_source_info white_noise_source_info = {
	.id = "simple_white_noise_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_AUDIO,
	.icon_type = OBS_ICON_TYPE_AUDIO_INPUT,
	.get_name = white_noise_get_name,
	.create = white_noise_create,
	.destroy = white_noise_destroy,
	.update = white_noise_update,
	.get_defaults = white_noise_get_defaults,
	.get_properties = white_noise_get_properties,
};
