#include "../include/audio.h"
#include <cstring>

Hyper_BandCHIP::Audio::Audio() : attack_time(0.005), decay_time(0.005), sustain_level(1.0), release_time(0.200), current_state_time(0.0), start_level(0.0), current_level(0.0), frequency(220.0), current_period(0.0), egs_accumulator(0.0), EGS(EnvelopeGeneratorState::Off)
{
	egs_tp = high_resolution_clock::now();
	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = 48000;
	desired.channels = 2;
	desired.samples = 512;
	desired.format = AUDIO_S32;
	desired.callback = &AudioHandler;
	desired.userdata = this;
	AudioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_spec, 0);
	SDL_PauseAudioDevice(AudioDevice, 0);
}

Hyper_BandCHIP::Audio::~Audio()
{
	SDL_PauseAudioDevice(AudioDevice, 1);
	SDL_CloseAudioDevice(AudioDevice);
}

void Hyper_BandCHIP::Audio::SetEnvelopeGeneratorState(EnvelopeGeneratorState NewState)
{
	EGS = NewState;
	current_state_time = 0.0;
	start_level = current_level;
}

Hyper_BandCHIP::EnvelopeGeneratorState Hyper_BandCHIP::Audio::GetEnvelopeGeneratorState() const
{
	return EGS;
}

void Hyper_BandCHIP::Audio::AudioHandler(void *userdata, Uint8 *stream, int len)
{
	high_resolution_clock::time_point start_tp = high_resolution_clock::now();
	Audio *CurrentAudio = static_cast<Audio *>(userdata);
	struct AudioChannelData
	{
		int left;
		int right;
	};
	for (int i = 0; i < len / sizeof(AudioChannelData); ++i)
	{
		AudioChannelData CurrentAudioData;
		double value = 0.0;
		if (CurrentAudio->current_period < 0.5 / CurrentAudio->frequency)
		{
			value = 1.0;
		}
		else
		{
			value = -1.0;
		}
		value *= (0.3 * CurrentAudio->current_level);
		if (value >= 0.0)
		{
			int i_value = static_cast<int>(2147483647.0 * value);
			CurrentAudioData = { i_value, i_value };
		}
		else
		{
			int i_value = static_cast<int>(2147483648.0 * value);
			CurrentAudioData = { i_value, i_value };
		}
		memcpy(&stream[i * sizeof(AudioChannelData)], &CurrentAudioData, sizeof(AudioChannelData));
		CurrentAudio->current_period += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
		if (CurrentAudio->current_period >= 1.0 / CurrentAudio->frequency)
		{
			CurrentAudio->current_period -= 1.0 / CurrentAudio->frequency;
		}
		switch (CurrentAudio->EGS)
		{
			case EnvelopeGeneratorState::Off:
			{
				if (CurrentAudio->current_level != 0.0)
				{
					CurrentAudio->current_level = 0.0;
				}
				break;
			}
			case EnvelopeGeneratorState::Attack:
			{
				bool auto_change = false;
				if (CurrentAudio->attack_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level + ((1.0 - CurrentAudio->start_level) * (CurrentAudio->current_state_time / CurrentAudio->attack_time));
				}
				else
				{
					CurrentAudio->current_level = 1.0;
					auto_change = true;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->attack_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Decay);
				}
				if (!auto_change)
				{
					break;
				}
			}
			case EnvelopeGeneratorState::Decay:
			{
				if (CurrentAudio->decay_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level - ((CurrentAudio->start_level - CurrentAudio->sustain_level) * (CurrentAudio->current_state_time / CurrentAudio->decay_time));
				}
				else
				{
					CurrentAudio->current_level = CurrentAudio->sustain_level;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->decay_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Sustain);
				}
				break;
			}
			case EnvelopeGeneratorState::Sustain:
			{
				if (CurrentAudio->current_level != CurrentAudio->sustain_level)
				{
					CurrentAudio->current_level = CurrentAudio->sustain_level;
				}
				break;
			}
			case EnvelopeGeneratorState::Release:
			{
				if (CurrentAudio->release_time > 0.0)
				{
					CurrentAudio->current_state_time += 1.0 / static_cast<double>(CurrentAudio->audio_spec.freq);
					CurrentAudio->current_level = CurrentAudio->start_level - (CurrentAudio->start_level * (CurrentAudio->current_state_time / CurrentAudio->release_time));
				}
				else
				{
					CurrentAudio->current_level = 0.0;
				}
				if (CurrentAudio->current_state_time >= CurrentAudio->release_time)
				{
					CurrentAudio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Off);
				}
				break;
			}
		}
	}
}
