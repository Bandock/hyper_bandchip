#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <chrono>
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
}

#endif
