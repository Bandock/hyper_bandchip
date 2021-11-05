#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <chrono>
#include <array>
#include <cstring>
#include <cmath>
#include <SDL.h>

using namespace std::chrono;

namespace Hyper_BandCHIP
{
	enum class EnvelopeGeneratorState { Off, Attack, Decay, Sustain, Release };

	class Audio
	{
		public:
			Audio();
			~Audio();
			void SetEnvelopeGeneratorState(EnvelopeGeneratorState NewState);
			EnvelopeGeneratorState GetEnvelopeGeneratorState() const;
			static void AudioHandler(void *userdata, Uint8 *stream, int len);
		private:
			double attack_time;
			double decay_time;
			double sustain_level;
			double release_time;
			double current_state_time;
			double start_level;
			double current_level;
			double frequency;
			double current_period;
			high_resolution_clock::time_point egs_tp;
			double egs_accumulator;
			EnvelopeGeneratorState EGS;
			SDL_AudioSpec audio_spec;
			SDL_AudioDeviceID AudioDevice;
	};

	template <unsigned char channels>
	class SampleAudio
	{
		public:
			SampleAudio() : playback_rate(4000.0), pause(true)
			{
				memset(audio_buffer.data(), 0x00, 16 * channels);
				for (unsigned char i = 0; i < channels; ++i)
				{
					audio_channel_output_toggle[i] = true;
					pos[i] = 0;
					b_offset[i] = 0;
					time[i] = 0.0;
				}
				SDL_AudioSpec desired;
				desired.freq = 96000;
				desired.channels = channels;
				desired.samples = 1600;
				desired.format = AUDIO_S16;
				desired.callback = &AudioHandler;
				desired.userdata = this;
				AudioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_spec, 0);
				SDL_PauseAudioDevice(AudioDevice, 0);
			}
			~SampleAudio()
			{
				SDL_PauseAudioDevice(AudioDevice, 1);
				SDL_CloseAudioDevice(AudioDevice);
			}
			void InitializeAudioBuffer()
			{
				memset(audio_buffer.data(), 0x00, 16 * channels);
			}
			void Reset()
			{
				for (unsigned char i = 0; i < channels; ++i)
				{
					pos[i] = 0;
					b_offset[i] = 0;
					time[i] = 0.0;
				}
			}
			void SetPlaybackRate(unsigned char rate)
			{
				playback_rate = 4000.0 * std::pow(2.0, ((static_cast<double>(rate) - 64.0) / 48.0));
			}
			void CopyToAudioBuffer(const unsigned char *memory, unsigned short start_address, unsigned char channel)
			{
				if (channel < channels)
				{
					for (unsigned char i = 0; i < 16; ++i)
					{
						audio_buffer[(channel * 16) + i] = memory[static_cast<unsigned short>(start_address + i)];
					}
				}
			}
			void PauseAudio(bool pause)
			{
				if (this->pause != pause)
				{
					this->pause = pause;
				}
			}
			bool IsPaused() const
			{
				return pause;
			}
			static void AudioHandler(void *userdata, Uint8 *stream, int len)
			{
				SampleAudio<channels> *Audio = static_cast<SampleAudio<channels> *>(userdata);
				for (int i = 0; i < len / sizeof(short); ++i)
				{
					unsigned char current_channel = static_cast<unsigned char>(i % channels);
					if (Audio->audio_channel_output_toggle[current_channel] && !Audio->pause)
					{
						short value = 0;
						if (Audio->audio_buffer[Audio->pos[current_channel]] & (0x80 >> (Audio->b_offset[current_channel])))
						{
							value = static_cast<short>(32767.0 * 0.20);
						}
						else
						{
							value = 0;
						}
						memcpy(&stream[i * sizeof(short)], &value, sizeof(short));
						Audio->time[current_channel] += 1.0 / static_cast<double>(Audio->audio_spec.freq);
						while (Audio->time[current_channel] >= 1.0 / Audio->playback_rate)
						{
							Audio->time[current_channel] -= 1.0 / Audio->playback_rate;
							++Audio->b_offset[current_channel];
							if (Audio->b_offset[current_channel] > 7)
							{
								Audio->b_offset[current_channel] = 0;
								if (Audio->pos[current_channel] < 15)
								{
									++Audio->pos[current_channel];
								}
								else
								{
									Audio->pos[current_channel] = 0;
								}
							}
						}
					}
					else
					{
						memset(&stream[i * sizeof(short)], 0, sizeof(short));
					}
				}
			}
		private:
			std::array<unsigned char, 16 * channels> audio_buffer;
			std::array<bool, channels> audio_channel_output_toggle;
			std::array<int, channels> pos;
			std::array<unsigned char, channels> b_offset;
			double playback_rate;
			std::array<double, channels> time;
			bool pause;
			SDL_AudioSpec audio_spec;
			SDL_AudioDeviceID AudioDevice;
	};
}

#endif
