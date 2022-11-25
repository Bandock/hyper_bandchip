#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <fstream>
#include <array>
#include <vector>
#include <chrono>
#include <random>
#include <variant>
#include <concepts>
#include <thread>
#include <mutex>
#include <atomic>
#include <semaphore>
#include "audio.h"

using namespace std::chrono;
using std::mt19937;
using std::uniform_int_distribution;

namespace Hyper_BandCHIP
{
	template <typename T>
	concept HasCurrentTimeSupport = requires(T obj)
	{
		obj.current_tp;
	};

	template <typename T>
	concept HasResolutionModeSupport = requires(T obj)
	{
		obj.CurrentResolutionMode;
	};

	template <typename T>
	concept HasProgramCounterSupport = requires(T obj)
	{
		obj.PC;
	};

	template <typename T>
	concept HasRegisterSupport = requires(T obj)
	{
		obj.V;
		obj.I;
	};

	template <typename T>
	concept HasTimerSupport = requires(T obj)
	{
		obj.delay_timer;
		obj.sound_timer;
	};

	template <typename T>
	concept HasStackSupport = requires(T obj)
	{
		obj.SP;
		obj.stack;
	};

	template <typename T>
	concept HasMemorySupport = requires(T obj)
	{
		obj.memory;
	};

	template <typename T>
	concept HasDisplaySupport = requires(T obj)
	{
		obj.display;
	};

	template <typename T>
	concept HasDisplaySizeSupport = requires(T obj)
	{
		obj.display_width;
		obj.display_height;
	};

	template <typename T>
	concept HasVIPDisplayInterruptSupport = requires(T obj)
	{
		obj.behavior_data.VIP_Display_Interrupt;
		obj.display_interrupt;
	};

	template <typename T>
	concept HasPlaneSupport = requires(T obj)
	{
		obj.plane;
	};

	template <typename T>
	concept HasVoiceSupport = requires(T obj)
	{
		obj.voice;
	};

	template <typename T>
	concept HasKeyStatusSupport = requires(T obj)
	{
		obj.key_status;
	};

	template <typename T>
	concept HasSyncSupport = requires(T obj)
	{
		obj.RefreshSync;
		obj.DelayTimerSync;
		obj.SoundTimerSync;
	};

	template <typename T>
	concept HasCycleSupport = requires(T obj)
	{
		obj.cycles_per_second;
		obj.cycle_rate;
		obj.cycle_tp;
		obj.cycle_accumulator;
	};

	template <typename T>
	concept HasSynthesizerAudioSupport = requires(T obj)
	{
		obj.audio_system.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
		obj.audio_system.GetEnvelopeGeneratorState();
	};

	template <typename T>
	concept HasSampleAudioSupport = requires(T obj)
	{
		obj.audio_system.InitializeAudioBuffer(0);
		obj.audio_system.Reset(0);
		obj.audio_system.SetChannelOutput(0, 0);
		obj.audio_system.SetPlaybackRate(0, 0);
		obj.audio_system.SetVolume(0, 0);
		obj.audio_system.CopyToAudioBuffer(nullptr, 0, 0);
		obj.audio_system.PauseAudio(true, 0);
		obj.audio_system.IsPaused(0);
	};

	template <typename T>
	concept HasOperationalSupport = requires(T obj)
	{
		obj.operational;
	};

	template <typename T>
	concept HasErrorStateSupport = requires(T obj)
	{
		obj.error_state;
	};

	template <typename T>
	concept HasPauseSupport = requires(T obj)
	{
		obj.pause;
	};

	template <typename T>
	concept HasRendererSupport = requires(T obj)
	{
		obj.DisplayRenderer->WriteToDisplay(nullptr, 0, 0);
		obj.DisplayRenderer->Render();
	};

	template <typename T>
	using InstructionCallback = void (*)(T &, uint16_t);

	class Machine;

	using XOCHIP_Audio = SampleAudio<1, 1>;
	using HyperCHIP64_Audio = SampleAudio<2, 4>;

	enum class MachineCore
	{
		BandCHIP_CHIP8, // BandCHIP CHIP-8 Core
		BandCHIP_SuperCHIP, // BandCHIP SuperCHIP Core
		BandCHIP_XOCHIP, // BandCHIP XO-CHIP Core
		BandCHIP_HyperCHIP64, // BandCHIP HyperCHIP-64 Core
		BandCHIP_Pipelined_CHIP8 // BandCHIP Pipelined CHIP-8 Core
	};

	enum class MachineAudioModel
	{
		Synthesizer,
		Sampled_XOCHIP,
		Sampled_HyperCHIP64
	};

	enum class MachineError
	{
		NoError,
		InvalidInstruction,
		StackOverflow,
		StackUnderflow,
		MachineInstructionsUnsupported
	};

	enum class SuperCHIPVersion
	{
		Fixed_SuperCHIP11, // Emulates SuperCHIP V1.1 with original CHIP-8 behavior. (Default)
		Original_SuperCHIP10, // Emulates the Original SuperCHIP V1.0 behavior. (Possibly needed for games and programs that used this build of SuperCHIP V1.0.)
		SuperCHIP10, // Emulates the SuperCHIP V1.0 behavior. (Likely needed for games and programs that were assembled for this version.)
		SuperCHIP11 // Emulates the original SuperCHIP V1.1 behavior (with CHIP-48 shifting and load/store).
	};

	enum class ResolutionMode
	{
		LoRes, // 64x32
		HiRes // 128x64
	};

	struct RegisterData_8Bit
	{
		unsigned char v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, va, vb, vc, vd, ve, vf;

		unsigned char &operator[](int index)
		{
			switch (index)
			{
				case 0x0: { return v0; }
				case 0x1: { return v1; }
				case 0x2: { return v2; }
				case 0x3: { return v3; }
				case 0x4: { return v4; }
				case 0x5: { return v5; }
				case 0x6: { return v6; }
				case 0x7: { return v7; }
				case 0x8: { return v8; }
				case 0x9: { return v9; }
				case 0xA: { return va; }
				case 0xB: { return vb; }
				case 0xC: { return vc; }
				case 0xD: { return vd; }
				case 0xE: { return ve; }
				case 0xF: { return vf; }
			}
			return v0;
		}
	};

	struct SoundTimerData
	{
		unsigned char st_0, st_1, st_2, st_3;

		unsigned char &operator[](int index)
		{
			switch (index)
			{
				case 0: { return st_0; }
				case 1: { return st_1; }
				case 2: { return st_2; }
				case 3: { return st_3; }
			}
			return st_0;
		}
	};

	struct SoundTimerAccumulatorData
	{
		double st_a0, st_a1, st_a2, st_a3;

		double &operator[](int index)
		{
			switch (index)
			{
				case 0: { return st_a0; }
				case 1: { return st_a1; }
				case 2: { return st_a2; }
				case 3: { return st_a3; }
			}
			return st_a0;
		}
	};

	/*
	template <MachineCore T>
	struct InstructionData
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};
	*/

	struct CHIP8_BehaviorData
	{
		bool CHIP48_Shift;
		bool CHIP48_LoadStore;
		bool VIP_Display_Interrupt;
		bool VIP_Clipping;
		bool VIP_VF_Reset;
	};

	struct SuperCHIP_BehaviorData
	{
		SuperCHIPVersion Version;
	};

	struct XOCHIP_BehaviorData
	{
		bool SuperCHIP_Shift;
		bool SuperCHIP_LoadStore;
		bool Octo_LoResSprite; // When toggled, enables 16x16 in low resolution mode.  Inherited from the Octo interpreter.
	};

	/*
	template <>
	struct InstructionData<MachineCore::BandCHIP_CHIP8>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	template <>
	struct InstructionData<MachineCore::BandCHIP_SuperCHIP>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	template <>
	struct InstructionData<MachineCore::BandCHIP_XOCHIP>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	template <>
	struct InstructionData<MachineCore::BandCHIP_HyperCHIP64>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};
	*/

	struct MachineState
	{
		// unsigned char V[0x10];
		std::array<uint8_t, 16> V;
		unsigned short PC;
		unsigned short I;
		unsigned char DT;
		// unsigned char ST[4];
		SoundTimerData ST;
	};

	struct SyncState
	{
		uint32_t cycle_counter;
		uint32_t cycles_per_frame;
	};

	class Renderer;

	/*
	class Machine
	{
		public:
			Machine(MachineCore Core = MachineCore::BandCHIP_CHIP8, unsigned int cycles_per_second = 600, unsigned int memory_size = 0x1000, unsigned short display_width = 64, unsigned short display_height = 32, Renderer *DisplayRenderer = nullptr);
			~Machine();
			
			template <typename T>
			void StoreBehaviorData(const T *source_behavior_data)
			{
				behavior_data = *source_behavior_data;
			}
			
			inline void SetResolutionMode(ResolutionMode Mode)
			{
				switch (CurrentMachineCore)
				{
					case MachineCore::BandCHIP_SuperCHIP:
					case MachineCore::BandCHIP_XOCHIP:
					case MachineCore::BandCHIP_HyperCHIP64:
					{
						CurrentResolutionMode = Mode;
						break;
					}
				}
			}
			
			inline void InitializeRegisters()
			{
				memset(&V, 0, sizeof(V));
				I = 0;
			}
			
			inline void InitializeTimers()
			{
				delay_timer = 0;
				dt_accumulator = 0.0;
				sound_timer = { 0, 0, 0, 0 };
				st_accumulator = { 0.0, 0.0, 0.0, 0.0 };
			}
			
			inline void InitializeStack()
			{
				SP = 0;
				memset(stack, 0, sizeof(stack));
			}
			
			void InitializeMemory();
			void InitializeVideo();
			
			inline void InitializeAudio()
			{
				switch (CurrentMachineAudioModel)
				{
					case MachineAudioModel::Sampled_XOCHIP:
					{
						XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
						audio->InitializeAudioBuffer();
						audio->Reset();
						audio->SetPlaybackRate(64);
						break;
					}
					case MachineAudioModel::Sampled_HyperCHIP64:
					{
						HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
						voice = 0;
						for (unsigned char v = 0; v < 4; ++v)
						{
							audio->InitializeAudioBuffer(v);
							audio->Reset(v);
							audio->SetChannelOutput(0x03, v);
							audio->SetPlaybackRate(64, v);
							audio->SetVolume(255, v);
						}
						break;
					}
				}
			}
			
			inline void InitializeKeyStatus()
			{
				memset(key_status, 0x00, 0x10);
			}

			inline void CopyDataToInterpreterMemory(const unsigned char *source, unsigned short address, unsigned int size)
			{
				if (source != nullptr)
				{
					if (size > 0 && size <= 0x200 && address < 0x200)
					{
						if (address + size - 1 < 0x200)
						{
							memcpy(&memory[address], source, size);
						}
					}
				}
			}
			
			inline void GetDisplay(unsigned char **display, unsigned short *display_width, unsigned short *display_height)
			{
				if (display != nullptr)
				{
					*display = this->display;
				}
				if (display_width != nullptr)
				{
					*display_width = this->display_width;
				}
				if (display_height != nullptr)
				{
					*display_height = this->display_height;
				}
			}

			inline void SetSync(bool toggle)
			{
				sync = toggle;
				if (sync && cycles_per_second > 0)
				{
					RefreshSync = { 0, cycles_per_second / 60 };
					DelayTimerSync = { 0, cycles_per_second / 60 };
					for (size_t i = 0; i < 4; ++i)
					{
						SoundTimerSync[i] = { 0, cycles_per_second / 60 };
					}
				}
			}

			inline bool GetSync() const
			{
				return sync;
			}
			
			inline void SetCyclesPerSecond(unsigned int cycles_per_second)
			{
				this->cycles_per_second = cycles_per_second;
				this->cycle_rate = (this->cycles_per_second > 0) ? 1.0 / static_cast<double>(this->cycles_per_second) : 0.0;
				if (sync && this->cycles_per_second > 0)
				{
					uint32_t previous_cycles_per_frame = RefreshSync.cycles_per_frame;
					RefreshSync.cycles_per_frame = this->cycles_per_second / 60;
					RefreshSync.cycle_counter = static_cast<double>(RefreshSync.cycle_counter) * (static_cast<double>(RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
					previous_cycles_per_frame = DelayTimerSync.cycles_per_frame;
					DelayTimerSync.cycles_per_frame = this->cycles_per_second / 60;
					DelayTimerSync.cycle_counter = static_cast<double>(DelayTimerSync.cycle_counter) * (static_cast<double>(DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
					for (size_t i = 0; i < 4; ++i)
					{
						previous_cycles_per_frame = SoundTimerSync[i].cycles_per_frame;
						SoundTimerSync[i].cycles_per_frame = this->cycles_per_second / 60;
						SoundTimerSync[i].cycle_counter = static_cast<double>(SoundTimerSync[i].cycle_counter) * (static_cast<double>(SoundTimerSync[i].cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
					}
				}
			}

			inline unsigned int GetCyclesPerFrame() const
			{
				return (cycles_per_second > 0) ? cycles_per_second / 60 : ((cycle_rate > 0.0) ? 1.0 / cycle_rate / 60.0 : 0);
			}
			
			inline void SetDelayTimer(unsigned char delay_timer)
			{
				this->delay_timer = delay_timer;
				if (!sync)
				{
					dt_accumulator = 0.0;
					if (this->delay_timer > 0)
					{
						dt_tp = current_tp;
					}
				}
				else
				{
					DelayTimerSync.cycle_counter = 0;
				}
			}
			
			void SetSoundTimer(unsigned char sound_timer);
			
			inline void SetKeyStatus(unsigned char key, bool pressed)
			{
				if (key <= 0xF)
				{
					key_status[key] = pressed;
				}
			}
			
			inline bool LoadProgram(const unsigned char *source, unsigned short start_address, unsigned int size)
			{
				if (source != nullptr)
				{
					if (size > 0 && size <= memory_size - 0x200)
					{
						this->start_address = start_address;
						PC = this->start_address;
						memcpy(&memory[this->start_address], source, size);
						cycle_accumulator = 0.0;
						if (error_state != MachineError::NoError)
						{
							error_state = MachineError::NoError;
						}
						if (!operational)
						{
							operational = true;
						}
						return true;
					}
				}
				return false;
			}
			
			void PauseProgram(bool pause = true);
			
			inline bool IsPaused() const
			{
				return pause;
			}
			
			inline bool IsOperational() const
			{
				return operational;
			}

			inline MachineCore GetMachineCore() const
			{
				return CurrentMachineCore;
			}
			
			inline MachineError GetErrorState() const
			{
				return error_state;
			}

			inline MachineState GetMachineState() const
			{
				MachineState State;
				memcpy(&State.V, &V, 0x10);
				State.PC = PC;
				State.I = I;
				State.DT = delay_timer;
				memcpy(&State.ST, &sound_timer, 4);
				return State;
			}
			
			inline void SetCurrentTime(const high_resolution_clock::time_point current_tp)
			{
				this->current_tp = current_tp;
			}
			
			inline void ExecuteInstructions()
			{
				duration<double> delta_time = current_tp - cycle_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				cycle_accumulator += delta_time.count();
				cycle_tp = current_tp;
				if (cycles_per_second == 0.0)
				{
					if (cycle_accumulator == 0.0)
					{
						return;
					}
					uint32_t new_cycles_per_second_u32 = 30000.0 / cycle_accumulator;
					if (new_cycles_per_second_u32 >= 60)
					{
						uint32_t excess_cycles = new_cycles_per_second_u32 % 60;
						if (excess_cycles)
						{
							new_cycles_per_second_u32 -= excess_cycles;
						}
						cycle_rate = 1.0 / new_cycles_per_second_u32;
						uint32_t previous_cycles_per_frame = RefreshSync.cycles_per_frame;
						RefreshSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
						RefreshSync.cycle_counter = static_cast<double>(RefreshSync.cycle_counter) * (static_cast<double>(RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
						previous_cycles_per_frame = DelayTimerSync.cycles_per_frame;
						DelayTimerSync.cycles_per_frame = new_cycles_per_second_u32 / 60;
						DelayTimerSync.cycle_counter = static_cast<double>(DelayTimerSync.cycle_counter) * (static_cast<double>(DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
						for (size_t i = 0; i < 4; ++i)
						{
							SyncState &CurrentSoundTimer = SoundTimerSync[i];
							previous_cycles_per_frame = CurrentSoundTimer.cycles_per_frame;
							CurrentSoundTimer.cycles_per_frame = new_cycles_per_second_u32 / 60;
							CurrentSoundTimer.cycle_counter = static_cast<double>(CurrentSoundTimer.cycle_counter) * (static_cast<double>(CurrentSoundTimer.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
						}
					}
					else
					{
						return;
					}
				}
				while (cycle_accumulator >= cycle_rate && !pause)
				{
					cycle_accumulator -= cycle_rate;
					if (sync)
					{
						SyncToCycle();
					}
					switch (CurrentMachineCore)
					{
						case MachineCore::BandCHIP_CHIP8:
						{
							InstructionData<MachineCore::BandCHIP_CHIP8> Instruction;
							Instruction.opcode = (memory[PC] >> 4);
							Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[((PC + 1) & 0xFFF)]);
							Instruction(this);
							const CHIP8_BehaviorData *Behavior = std::get_if<CHIP8_BehaviorData>(&behavior_data);
							if (Behavior->VIP_Display_Interrupt)
							{
								if (display_interrupt)
								{
									display_interrupt = false;
									DisplayRender();
								}
							}
							break;
						}
						case MachineCore::BandCHIP_SuperCHIP:
						{
							InstructionData<MachineCore::BandCHIP_SuperCHIP> Instruction;
							Instruction.opcode = (memory[PC] >> 4);
							Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[((PC + 1) & 0xFFF)]);
							Instruction(this);
							break;
						}
						case MachineCore::BandCHIP_XOCHIP:
						{
							InstructionData<MachineCore::BandCHIP_XOCHIP> Instruction;
							Instruction.opcode = (memory[PC] >> 4);
							Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<unsigned short>(PC + 1)]);
							Instruction(this);
							break;
						}
						case MachineCore::BandCHIP_HyperCHIP64:
						{
							InstructionData<MachineCore::BandCHIP_HyperCHIP64> Instruction;
							Instruction.opcode = (memory[PC] >> 4);
							Instruction.operand = ((memory[PC] & 0x0F) << 8) | (memory[static_cast<unsigned short>(PC + 1)]);
							Instruction(this);
							break;
						}
					}
				}
			}
			
			inline void RunDelayTimer()
			{
				if (delay_timer > 0)
				{
					if (!sync)
					{
						duration<double> delta_time = current_tp - dt_tp;
						if (delta_time.count() > 0.25)
						{
							delta_time = duration<double>(0.25);
						}
						dt_accumulator += delta_time.count();
						dt_tp = current_tp;
						constexpr double dt_rate = (1.0 / 60.0);
						if (dt_accumulator >= dt_rate)
						{
							unsigned char tick_count = static_cast<unsigned char>(dt_accumulator / dt_rate);
							if (delay_timer >= tick_count)
							{
								delay_timer -= tick_count;
							}
							else
							{
								delay_timer = 0;
							}
							if (delay_timer == 0)
							{
								dt_accumulator = 0.0;
							}
							else
							{
								dt_accumulator -= (static_cast<double>(tick_count) / 60.0);
							}
						}
					}
					else
					{
						++DelayTimerSync.cycle_counter;
						if (DelayTimerSync.cycle_counter >= DelayTimerSync.cycles_per_frame)
						{
							DelayTimerSync.cycle_counter = 0;
							--delay_timer;
						}
					}
				}
			}
			
			template <uint8_t voice>
			inline void RunSoundTimer()
			{
				if (sound_timer[voice] > 0)
				{
					if (!sync)
					{
						duration<double> delta_time = current_tp - st_tp[voice];
						if (delta_time.count() > 0.25)
						{
							delta_time = duration<double>(0.25);
						}
						st_accumulator[voice] += delta_time.count();
						st_tp[voice] = current_tp;
						constexpr double st_rate = 1.0 / 60.0;
						if (st_accumulator[voice] >= st_rate)
						{
							unsigned char tick_count = static_cast<unsigned char>(st_accumulator[voice] / st_rate);
							if (sound_timer[voice] >= tick_count)
							{
								sound_timer[voice] -= tick_count;
							}
							else
							{
								sound_timer[voice] = 0;
							}
							if (sound_timer[voice] == 0)
							{
								st_accumulator[voice] = 0.0;
								switch (CurrentMachineAudioModel)
								{
									case MachineAudioModel::Synthesizer:
									{
										Audio *audio = std::get_if<Audio>(&audio_system);
										audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
										break;
									}
									case MachineAudioModel::Sampled_XOCHIP:
									{
										XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
										audio->PauseAudio(true);
										audio->Reset();
										break;
									}
									case MachineAudioModel::Sampled_HyperCHIP64:
									{
										HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
										audio->PauseAudio(true, voice);
										audio->Reset(voice);
										break;
									}
								}
							}
							else
							{
								st_accumulator[voice] -= (static_cast<double>(tick_count) / 60.0);
							}
						}
					}
					else
					{
						++SoundTimerSync[voice].cycle_counter;
						if (SoundTimerSync[voice].cycle_counter >= SoundTimerSync[voice].cycles_per_frame)
						{
							SoundTimerSync[voice].cycle_counter = 0;
							--sound_timer[voice];
							if (sound_timer[voice] == 0)
							{
								switch (CurrentMachineAudioModel)
								{
									case MachineAudioModel::Synthesizer:
									{
										Audio *audio = std::get_if<Audio>(&audio_system);
										audio->SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
										break;
									}
									case MachineAudioModel::Sampled_XOCHIP:
									{
										XOCHIP_Audio *audio = std::get_if<XOCHIP_Audio>(&audio_system);
										audio->PauseAudio(true);
										audio->Reset();
										break;
									}
									case MachineAudioModel::Sampled_HyperCHIP64:
									{
										HyperCHIP64_Audio *audio = std::get_if<HyperCHIP64_Audio>(&audio_system);
										audio->PauseAudio(true, voice);
										audio->Reset(voice);
										break;
									}
								}
							}
						}
					}
				}
			}

			void SyncToCycle();
			void DisplayRender();
			friend struct InstructionData<MachineCore::BandCHIP_CHIP8>;
			friend struct InstructionData<MachineCore::BandCHIP_SuperCHIP>;
			friend struct InstructionData<MachineCore::BandCHIP_XOCHIP>;
			friend struct InstructionData<MachineCore::BandCHIP_HyperCHIP64>;
		private:
			MachineCore CurrentMachineCore;
			MachineAudioModel CurrentMachineAudioModel;
			ResolutionMode CurrentResolutionMode;
			std::filebuf rpl_user_flags_file;
			std::variant<CHIP8_BehaviorData, SuperCHIP_BehaviorData, XOCHIP_BehaviorData> behavior_data;
			std::variant<Audio, XOCHIP_Audio, HyperCHIP64_Audio> audio_system;
			Renderer *DisplayRenderer;
			bool sync;
			bool superscalar_mode;
			bool display_interrupt;
			unsigned int cycles_per_second;
			double cycle_rate;
			unsigned char delay_timer;
			// std::array<unsigned char, 4> sound_timer;
			SoundTimerData sound_timer;
			// unsigned char V[16];
			RegisterData_8Bit V;
			unsigned short start_address;
			unsigned short PC;
			unsigned short I;
			unsigned short SP;
			unsigned short stack[32];
			unsigned char *memory;
			unsigned int memory_size;
			unsigned char *display;
			unsigned short display_width;
			unsigned short display_height;
			unsigned char key_status[0x10];
			unsigned char key_pressed;
			unsigned char plane;
			unsigned char voice;
			mt19937 rng_engine;
			uniform_int_distribution<unsigned char> rng_distrib;
			high_resolution_clock::time_point current_tp;
			high_resolution_clock::time_point cycle_tp;
			double cycle_accumulator;
			high_resolution_clock::time_point dt_tp;
			double dt_accumulator;
			high_resolution_clock::time_point st_tp[4];
			SoundTimerAccumulatorData st_accumulator;
			SyncState RefreshSync;
			SyncState DelayTimerSync;
			SyncState SoundTimerSync[4];
			bool pause;
			bool operational;
			bool wait_for_key_release;
			MachineError error_state;
	};
	*/

	struct ExecutionUnitData
	{
		bool running;
		std::thread ExecutionThread;
		std::atomic<uint32_t> cycles;
	};
	
	template <HasResolutionModeSupport T>
	inline void SetResolutionMode(T &machine, ResolutionMode Mode)
	{
		machine.CurrentResolutionMode = Mode;
	}

	template <HasRegisterSupport T>
	inline void InitializeRegisters(T &machine)
	{
		memset(&machine.V, 0, sizeof(machine.V));
		machine.I = 0;
	}

	template <HasTimerSupport T>
	inline void InitializeTimers(T &machine)
	{
		machine.delay_timer = 0;
		machine.sound_timer = 0;
	}

	template <HasTimerSupport T, typename T2> requires std::same_as<T2, HyperCHIP64_Audio>
	inline void InitializeTimers(T &machine)
	{
		machine.delay_timer = 0;
		for (size_t v = 0; v < 4; ++v)
		{
			machine.sound_timer[v] = 0;
		}
	}

	template <HasStackSupport T>
	inline void InitializeStack(T &machine)
	{
		machine.SP = 0;
		memset(machine.stack.data(), 0, machine.stack.size());
	}

	template <HasMemorySupport T>
	inline void InitializeMemory(T &machine)
	{
		memset(machine.memory.data(), 0, machine.memory.size());
	}

	template <HasDisplaySupport T>
	inline void InitializeVideo(T &machine)
	{
		memset(machine.display.data(), 0, machine.display.size());
	}

	template <HasSampleAudioSupport T, typename T2> requires std::same_as<T2, XOCHIP_Audio>
	inline void InitializeAudio(T &machine)
	{
		machine.audio_system.InitializeAudioBuffer();
		machine.audio_system.Reset();
		machine.audio_system.SetPlaybackRate(64);
	}

	template <typename T, typename T2> requires HasSampleAudioSupport<T> && HasVoiceSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline void InitializeAudio(T &machine)
	{
		machine.voice = 0;
		for (uint8_t v = 0; v < 4; ++v)
		{
			machine.audio_system.InitializeAudioBuffer(v);
			machine.audio_system.Reset(v);
			machine.audio_system.SetChannelOutput(0x03, v);
			machine.audio_system.SetPlaybackRate(64, v);
			machine.audio_system.SetVolume(255, v);
		}
	}

	template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
	inline void InitializeVideo(T &machine)
	{
		memset(machine.display.data(), 0, machine.display.size());
		machine.plane = 0x01;
	}

	template <HasKeyStatusSupport T>
	inline void InitializeKeyStatus(T &machine)
	{
		memset(machine.key_status, 0x00, 0x10);
	}

	template <HasMemorySupport T>
	inline void CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size)
	{
		if (source != nullptr)
		{
			if (size > 0 && size <= 512 && address < 0x200)
			{
				if (address + size - 1 < 0x200)
				{
					memcpy(&machine.memory[address], source, size);
				}
			}
		}
	}

	template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
	inline void GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height)
	{
		if (display != nullptr)
		{
			*display = machine.display.data();
		}
		if (display_width != nullptr)
		{
			*display_width = machine.display_width;
		}
		if (display_height != nullptr)
		{
			*display_height = machine.display_height;
		}
	}

	template <typename T> requires HasSyncSupport<T> && HasCycleSupport<T>
	inline void SetCyclesPerSecond(T &machine, unsigned int cycles_per_second)
	{
		machine.cycles_per_second = cycles_per_second;
		machine.cycle_rate = (machine.cycles_per_second > 0) ? 1.0 / static_cast<double>(machine.cycles_per_second) : 0.0;
		if (machine.cycles_per_second > 0)
		{
			uint32_t previous_cycles_per_frame = machine.RefreshSync.cycles_per_frame;
			machine.RefreshSync.cycles_per_frame = machine.cycles_per_second / 60;
			machine.RefreshSync.cycle_counter = static_cast<double>(machine.RefreshSync.cycle_counter) * (static_cast<double>(machine.RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = machine.DelayTimerSync.cycles_per_frame;
			machine.DelayTimerSync.cycles_per_frame = machine.cycles_per_second / 60;
			machine.DelayTimerSync.cycle_counter = static_cast<double>(machine.DelayTimerSync.cycle_counter) * (static_cast<double>(machine.DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = machine.SoundTimerSync.cycles_per_frame;
			machine.SoundTimerSync.cycles_per_frame = machine.cycles_per_second / 60;
			machine.SoundTimerSync.cycle_counter = static_cast<double>(machine.SoundTimerSync.cycle_counter) * (static_cast<double>(machine.SoundTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
		}
	}

	template <typename T, typename T2> requires HasSyncSupport<T> && HasCycleSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline void SetCyclesPerSecond(T &machine, unsigned int cycles_per_second)
	{
		machine.cycles_per_second = cycles_per_second;
		machine.cycle_rate = (machine.cycles_per_second > 0) ? 1.0 / static_cast<double>(machine.cycles_per_second) : 0.0;
		if (machine.cycles_per_second > 0)
		{
			uint32_t previous_cycles_per_frame = machine.RefreshSync.cycles_per_frame;
			machine.RefreshSync.cycles_per_frame = machine.cycles_per_second / 60;
			machine.RefreshSync.cycle_counter = static_cast<double>(machine.RefreshSync.cycle_counter) * (static_cast<double>(machine.RefreshSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			previous_cycles_per_frame = machine.DelayTimerSync.cycles_per_frame;
			machine.DelayTimerSync.cycles_per_frame = machine.cycles_per_second / 60;
			machine.DelayTimerSync.cycle_counter = static_cast<double>(machine.DelayTimerSync.cycle_counter) * (static_cast<double>(machine.DelayTimerSync.cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			for (size_t v = 0; v < 4; ++v)
			{
				previous_cycles_per_frame = machine.SoundTimerSync[v].cycles_per_frame;
				machine.SoundTimerSync[v].cycles_per_frame = machine.cycles_per_second / 60;
				machine.SoundTimerSync[v].cycle_counter = static_cast<double>(machine.SoundTimerSync[v].cycle_counter) * (static_cast<double>(machine.SoundTimerSync[v].cycles_per_frame) / static_cast<double>(previous_cycles_per_frame));
			}
		}
	}

	template <HasCycleSupport T>
	inline unsigned int GetCyclesPerFrame(T &machine)
	{
		return (machine.cycles_per_second > 0) ? machine.cycles_per_second / 60 : ((machine.cycle_rate > 0.0) ? 1.0 / machine.cycle_rate / 60.0 : 0);
	}

	template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
	inline void SetDelayTimer(T &machine, unsigned char delay_timer)
	{
		machine.delay_timer = delay_timer;
		machine.DelayTimerSync.cycle_counter = 0;
	}

	template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
	inline void SetSoundTimer(T &machine, unsigned char sound_timer)
	{
		machine.sound_timer = sound_timer;
		machine.SoundTimerSync.cycle_counter = 0;
		if (machine.sound_timer > 0)
		{
			switch (machine.audio_system.GetEnvelopeGeneratorState())
			{
				case EnvelopeGeneratorState::Release:
				case EnvelopeGeneratorState::Off:
				{
					machine.audio_system.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
					break;
				}
			}
		}
	}

	template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, XOCHIP_Audio>
	inline void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer)
	{
		machine.sound_timer = sound_timer;
		machine.SoundTimerSync.cycle_counter = 0;
		if (machine.audio_system.IsPaused())
		{
			machine.audio_system.PauseAudio(false);
		}
	}

	template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && HasVoiceSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer)
	{
		if (machine.voice < machine.sound_timer.size())
		{
			machine.sound_timer[machine.voice] = sound_timer;
			machine.SoundTimerSync[machine.voice].cycle_counter = 0;
			if (machine.audio_system.IsPaused(machine.voice))
			{
				machine.audio_system.PauseAudio(false, machine.voice);
			}
		}
	}

	template <HasKeyStatusSupport T>
	inline void SetKeyStatus(T &machine, unsigned char key, bool pressed)
	{
		if (key <= 0xF)
		{
			machine.key_status[key] = pressed;
		}
	}

	template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
	inline bool LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size)
	{
		if (source != nullptr)
		{
			if (size > 0 && size <= machine.memory.size() - 0x200)
			{
				machine.PC = start_address;
				memcpy(&machine.memory[start_address], source, size);
				machine.cycle_accumulator = 0.0;
				if (machine.error_state != MachineError::NoError)
				{
					machine.error_state = MachineError::NoError;
				}
				if (!machine.operational)
				{
					machine.operational = true;
				}
				return true;
			}
		}
		return false;
	}

	template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSynthesizerAudioSupport<T> && HasPauseSupport<T>
	void PauseProgram(T &machine, bool pause)
	{
		if (machine.pause != pause)
		{
			if (machine.pause)
			{
				machine.cycle_tp = machine.current_tp;
				if (machine.sound_timer > 0)
				{
					if (machine.audio_system.GetEnvelopeGeneratorState() == EnvelopeGeneratorState::Off)
					{
						machine.audio_system.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Attack);
					}
				}
			}
			else
			{
				duration<double> delta_time = machine.current_tp - machine.cycle_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				machine.cycle_accumulator += delta_time.count();
				if (machine.sound_timer > 0)
				{
					switch (machine.audio_system.GetEnvelopeGeneratorState())
					{
						case EnvelopeGeneratorState::Attack:
						case EnvelopeGeneratorState::Decay:
						case EnvelopeGeneratorState::Sustain:
						{
							machine.audio_system.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
							break;
						}
					}
				}
			}
		}
		machine.pause = pause;
	}

	template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSampleAudioSupport<T> && HasPauseSupport<T> && std::same_as<T2, XOCHIP_Audio>
	void PauseProgram(T &machine, bool pause)
	{
		if (machine.pause != pause)
		{
			if (machine.pause)
			{
				machine.cycle_tp = machine.current_tp;
				if (machine.sound_timer > 0)
				{
					if (machine.audio_system.IsPaused())
					{
						machine.audio_system.PauseAudio(false);
					}
				}
			}
			else
			{
				duration<double> delta_time = machine.current_tp - machine.cycle_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				machine.cycle_accumulator += delta_time.count();
				if (machine.sound_timer > 0)
				{
					if (!machine.audio_system.IsPaused())
					{
						machine.audio_system.PauseAudio(true);
					}
				}
			}
		}
		machine.pause = pause;
	}

	template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSampleAudioSupport<T> && HasPauseSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	void PauseProgram(T &machine, bool pause)
	{
		if (machine.pause != pause)
		{
			if (machine.pause)
			{
				machine.cycle_tp = machine.current_tp;
				for (size_t v = 0; v < 4; ++v)
				{
					if (machine.sound_timer[v] > 0)
					{
						if (machine.audio_system.IsPaused(v))
						{
							machine.audio_system.PauseAudio(false, v);
						}
					}
				}
			}
			else
			{
				duration<double> delta_time = machine.current_tp - machine.cycle_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = duration<double>(0.25);
				}
				machine.cycle_accumulator += delta_time.count();
				for (size_t v = 0; v < 4; ++v)
				{
					if (!machine.audio_system.IsPaused(v))
					{
						machine.audio_system.PauseAudio(true, v);
					}
				}
			}
		}
		machine.pause = pause;
	}

	template <HasPauseSupport T>
	inline bool IsPaused(T &machine)
	{
		return machine.pause;
	}

	template <HasOperationalSupport T>
	inline bool IsOperational(T &machine)
	{
		return machine.operational;
	}

	template <HasErrorStateSupport T>
	inline MachineError GetErrorState(T &machine)
	{
		return machine.error_state;
	}

	template <typename T> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T>
	inline MachineState GetMachineState(T &machine)
	{
		MachineState State;
		memcpy(&State.V, &machine.V, 0x10);
		State.PC = machine.PC;
		State.I = machine.I;
		State.DT = machine.delay_timer;
		State.ST[0] = machine.sound_timer;
		memset(&State.ST, 0, 3);
		return State;
	}

	template <typename T, typename T2> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline MachineState GetMachineState(T &machine)
	{
		MachineState State;
		memcpy(&State.V, &machine.V, 0x10);
		State.PC = machine.PC;
		State.I = machine.I;
		State.DT = machine.delay_timer;
		memcpy(&State.ST, &machine.sound_timer, 4);
		return State;
	}

	template <HasCurrentTimeSupport T>
	inline void SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp)
	{
		machine.current_tp = current_tp;
	}

	template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
	inline void RunDelayTimer(T &machine)
	{
		if (machine.delay_timer > 0)
		{
			++machine.DelayTimerSync.cycle_counter;
			if (machine.DelayTimerSync.cycle_counter >= machine.DelayTimerSync.cycles_per_frame)
			{
				machine.DelayTimerSync.cycle_counter = 0;
				--machine.delay_timer;
			}
		}
	}

	template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
	inline void RunSoundTimer(T &machine)
	{
		if (machine.sound_timer > 0)
		{
			++machine.SoundTimerSync.cycle_counter;
			if (machine.SoundTimerSync.cycle_counter >= machine.SoundTimerSync.cycles_per_frame)
			{
				machine.SoundTimerSync.cycle_counter = 0;
				--machine.sound_timer;
				if (machine.sound_timer == 0)
				{
					machine.audio_system.SetEnvelopeGeneratorState(EnvelopeGeneratorState::Release);
				}
			}
		}
	}

	template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, XOCHIP_Audio>
	inline void RunSoundTimer(T &machine)
	{
		if (machine.sound_timer > 0)
		{
			++machine.SoundTimerSync.cycle_counter;
			if (machine.SoundTimerSync.cycle_counter >= machine.SoundTimerSync.cycles_per_frame)
			{
				machine.SoundTimerSync.cycle_counter = 0;
				--machine.sound_timer;
				if (machine.sound_timer == 0)
				{
					machine.audio_system.PauseAudio(true);
					machine.audio_system.Reset();
				}
			}
		}
	}

	template <typename T, typename T2, uint8_t voice> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline void RunSoundTimer(T &machine)
	{
		if (machine.sound_timer[voice] > 0)
		{
			++machine.SoundTimerSync[voice].cycle_counter;
			if (machine.SoundTimerSync[voice].cycle_counter >= machine.SoundTimerSync[voice].cycles_per_frame)
			{
				machine.SoundTimerSync[voice].cycle_counter = 0;
				--machine.sound_timer[voice];
				if (machine.sound_timer[voice] == 0)
				{
					machine.audio_system.PauseAudio(true, voice);
					machine.audio_system.Reset(voice);
				}
			}
		}
	}

	template <typename T> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
	inline void SyncToCycle(T &machine)
	{
		RunDelayTimer(machine);
		RunSoundTimer(machine);
		++machine.RefreshSync.cycle_counter;
		if (machine.RefreshSync.cycle_counter >= machine.RefreshSync.cycles_per_frame)
		{
			machine.RefreshSync.cycle_counter = 0;
			machine.DisplayRenderer->WriteToDisplay(machine.display.data(), machine.display_width, machine.display_height);
			machine.DisplayRenderer->Render();
		}
	}

	template <typename T> requires HasTimerSupport<T> && HasVIPDisplayInterruptSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
	inline void SyncToCycle(T &machine)
	{
		RunDelayTimer(machine);
		RunSoundTimer(machine);
		++machine.RefreshSync.cycle_counter;
		if (machine.RefreshSync.cycle_counter >= machine.RefreshSync.cycles_per_frame)
		{
			machine.RefreshSync.cycle_counter = 0;
			if (machine.behavior_data.VIP_Display_Interrupt)
			{
				machine.display_interrupt = true;
				return;
			}
			machine.DisplayRenderer->WriteToDisplay(machine.display.data(), machine.display_width, machine.display_height);
			machine.DisplayRenderer->Render();
		}
	}

	template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, XOCHIP_Audio>
	inline void SyncToCycle(T &machine)
	{
		RunDelayTimer(machine);
		RunSoundTimer<T, T2>(machine);
		++machine.RefreshSync.cycle_counter;
		if (machine.RefreshSync.cycle_counter >= machine.RefreshSync.cycles_per_frame)
		{
			machine.RefreshSync.cycle_counter = 0;
			machine.DisplayRenderer->WriteToDisplay(machine.display.data(), machine.display_width, machine.display_height);
			machine.DisplayRenderer->Render();
		}
	}

	template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
	inline void SyncToCycle(T &machine)
	{
		RunDelayTimer(machine);
		RunSoundTimer<T, T2, 0>(machine);
		RunSoundTimer<T, T2, 1>(machine);
		RunSoundTimer<T, T2, 2>(machine);
		RunSoundTimer<T, T2, 3>(machine);
		++machine.RefreshSync.cycle_counter;
		if (machine.RefreshSync.cycle_counter >= machine.RefreshSync.cycles_per_frame)
		{
			machine.RefreshSync.cycle_counter = 0;
			machine.DisplayRenderer->WriteToDisplay(machine.display.data(), machine.display_width, machine.display_height);
			machine.DisplayRenderer->Render();
		}
	}

	template <HasRendererSupport T>
	inline void DisplayRender(T &machine)
	{
		machine.DisplayRenderer->WriteToDisplay(machine.display.data(), machine.display_width, machine.display_height);
		machine.DisplayRenderer->Render();
	}

	namespace BandCHIP
	{
		class CHIP8_Machine
		{
			public:
				CHIP8_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~CHIP8_Machine();

				inline void StoreBehaviorData(const CHIP8_BehaviorData *source_behavior_data)
				{
					behavior_data = *source_behavior_data;
				}

				void ExecuteInstructions();

				template <HasRegisterSupport T>
				friend void Hyper_BandCHIP::InitializeRegisters(T &machine);
				template <HasTimerSupport T>
				friend void Hyper_BandCHIP::InitializeTimers(T &machine);
				template <HasStackSupport T>
				friend void Hyper_BandCHIP::InitializeStack(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::InitializeMemory(T &machine);
				template <HasDisplaySupport T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::InitializeKeyStatus(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size);
				template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
				friend void Hyper_BandCHIP::GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height);
				template <typename T> requires HasSyncSupport<T> && HasCycleSupport<T>
				friend void Hyper_BandCHIP::SetCyclesPerSecond(T &machine, unsigned int cycles_per_second);
				template <HasCycleSupport T>
				friend unsigned int Hyper_BandCHIP::GetCyclesPerFrame(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::SetDelayTimer(T &machine, unsigned char delay_timer);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::SetKeyStatus(T &machine, unsigned char key, bool pressed);
				template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
				friend bool Hyper_BandCHIP::LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSynthesizerAudioSupport<T> && HasPauseSupport<T>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <HasPauseSupport T>
				friend bool Hyper_BandCHIP::IsPaused(T &machine);
				template <HasOperationalSupport T>
				friend bool Hyper_BandCHIP::IsOperational(T &machine);
				template <HasErrorStateSupport T>
				friend MachineError Hyper_BandCHIP::GetErrorState(T &machine);
				template <typename T> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T>
				friend MachineState Hyper_BandCHIP::GetMachineState(T &machine);
				template <HasCurrentTimeSupport T>
				friend void Hyper_BandCHIP::SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::RunDelayTimer(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::RunSoundTimer(T &machine);
				template <typename T> requires HasTimerSupport<T> && HasVIPDisplayInterruptSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <HasRendererSupport T>
				friend void Hyper_BandCHIP::DisplayRender(T &machine);
			private:
				std::array<uint8_t, 4096> memory;
				double cycle_rate;
				uint32_t cycles_per_second;
				uint16_t PC;
				uint16_t I;
				uint16_t SP;
				CHIP8_BehaviorData behavior_data;
				Audio audio_system;
				Renderer *DisplayRenderer;
				bool display_interrupt;
				unsigned char delay_timer;
				unsigned char sound_timer;
				std::array<uint8_t, 16> V;
				std::array<uint16_t, 32> stack;
				std::array<unsigned char, 64 * 32> display;
				const unsigned short display_width = 64;
				const unsigned short display_height = 32;
				unsigned char key_status[0x10];
				unsigned char key_pressed;
				mt19937 rng_engine;
				uniform_int_distribution<unsigned char> rng_distrib;
				high_resolution_clock::time_point current_tp;
				high_resolution_clock::time_point cycle_tp;
				double cycle_accumulator;
				SyncState RefreshSync;
				SyncState DelayTimerSync;
				SyncState SoundTimerSync;
				bool pause;
				bool operational;
				bool wait_for_key_release;
				MachineError error_state;
		};

		class SuperCHIP_Machine
		{
			public:
				SuperCHIP_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~SuperCHIP_Machine();

				inline void StoreBehaviorData(const SuperCHIP_BehaviorData *source_behavior_data)
				{
					behavior_data = *source_behavior_data;
				}

				void ExecuteInstructions();

				template <HasResolutionModeSupport T>
				friend void Hyper_BandCHIP::SetResolutionMode(T &machine, ResolutionMode Mode);
				template <HasRegisterSupport T>
				friend void Hyper_BandCHIP::InitializeRegisters(T &machine);
				template <HasTimerSupport T>
				friend void Hyper_BandCHIP::InitializeTimers(T &machine);
				template <HasStackSupport T>
				friend void Hyper_BandCHIP::InitializeStack(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::InitializeMemory(T &machine);
				template <HasDisplaySupport T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::InitializeKeyStatus(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size);
				template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
				friend void Hyper_BandCHIP::GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height);
				template <typename T> requires HasSyncSupport<T> && HasCycleSupport<T>
				friend void Hyper_BandCHIP::SetCyclesPerSecond(T &machine, unsigned int cycles_per_second);
				template <HasCycleSupport T>
				friend unsigned int Hyper_BandCHIP::GetCyclesPerFrame(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::SetDelayTimer(T &machine, unsigned char delay_timer);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::SetKeyStatus(T &machine, unsigned char key, bool pressed);
				template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
				friend bool Hyper_BandCHIP::LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSynthesizerAudioSupport<T> && HasPauseSupport<T>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <HasPauseSupport T>
				friend bool Hyper_BandCHIP::IsPaused(T &machine);
				template <HasOperationalSupport T>
				friend bool Hyper_BandCHIP::IsOperational(T &machine);
				template <HasErrorStateSupport T>
				friend MachineError Hyper_BandCHIP::GetErrorState(T &machine);
				template <typename T> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T>
				friend MachineState Hyper_BandCHIP::GetMachineState(T &machine);
				template <HasCurrentTimeSupport T>
				friend void Hyper_BandCHIP::SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::RunDelayTimer(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::RunSoundTimer(T &machine);
				template <typename T> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <typename T> requires HasTimerSupport<T> && HasVIPDisplayInterruptSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <HasRendererSupport T>
				friend void Hyper_BandCHIP::DisplayRender(T &machine);
			private:
				std::array<uint8_t, 4096> memory;
				double cycle_rate;
				uint32_t cycles_per_second;
				uint16_t PC;
				uint16_t I;
				uint16_t SP;
				std::filebuf rpl_user_flags_file;
				SuperCHIP_BehaviorData behavior_data;
				Audio audio_system;
				Renderer *DisplayRenderer;
				unsigned char delay_timer;
				unsigned char sound_timer;
				std::array<uint8_t, 16> V;
				std::array<uint16_t, 32> stack;
				std::array<unsigned char, 128 * 64> display;
				const unsigned short display_width = 128;
				const unsigned short display_height = 64;
				unsigned char key_status[0x10];
				unsigned char key_pressed;
				mt19937 rng_engine;
				uniform_int_distribution<unsigned char> rng_distrib;
				high_resolution_clock::time_point current_tp;
				high_resolution_clock::time_point cycle_tp;
				double cycle_accumulator;
				SyncState RefreshSync;
				SyncState DelayTimerSync;
				SyncState SoundTimerSync;
				ResolutionMode CurrentResolutionMode;
				bool pause;
				bool operational;
				bool wait_for_key_release;
				MachineError error_state;
		};

		class XOCHIP_Machine
		{
			public:
				XOCHIP_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~XOCHIP_Machine();

				inline void StoreBehaviorData(const XOCHIP_BehaviorData *source_behavior_data)
				{
					behavior_data = *source_behavior_data;
				}

				void ExecuteInstructions();

				static void DelayTimerThread(XOCHIP_Machine *machine);

				template <HasResolutionModeSupport T>
				friend void Hyper_BandCHIP::SetResolutionMode(T &machine, ResolutionMode Mode);
				template <HasRegisterSupport T>
				friend void Hyper_BandCHIP::InitializeRegisters(T &machine);
				template <HasTimerSupport T>
				friend void Hyper_BandCHIP::InitializeTimers(T &machine);
				template <HasStackSupport T>
				friend void Hyper_BandCHIP::InitializeStack(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::InitializeMemory(T &machine);
				template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <HasSampleAudioSupport T, typename T2> requires std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::InitializeAudio(T &machine);
				template <typename T, typename T2> requires HasSampleAudioSupport<T> && HasVoiceSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::InitializeAudio(T &machine);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::InitializeKeyStatus(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size);
				template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
				friend void Hyper_BandCHIP::GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height);
				template <typename T> requires HasSyncSupport<T> && HasCycleSupport<T>
				friend void Hyper_BandCHIP::SetCyclesPerSecond(T &machine, unsigned int cycles_per_second);
				template <HasCycleSupport T>
				friend unsigned int Hyper_BandCHIP::GetCyclesPerFrame(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::SetDelayTimer(T &machine, unsigned char delay_timer);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::SetKeyStatus(T &machine, unsigned char key, bool pressed);
				template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
				friend bool Hyper_BandCHIP::LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSampleAudioSupport<T> && HasPauseSupport<T> && std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSampleAudioSupport<T> && HasPauseSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <HasPauseSupport T>
				friend bool Hyper_BandCHIP::IsPaused(T &machine);
				template <HasOperationalSupport T>
				friend bool Hyper_BandCHIP::IsOperational(T &machine);
				template <HasErrorStateSupport T>
				friend MachineError Hyper_BandCHIP::GetErrorState(T &machine);
				template <typename T> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T>
				friend MachineState Hyper_BandCHIP::GetMachineState(T &machine);
				template <HasCurrentTimeSupport T>
				friend void Hyper_BandCHIP::SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::RunDelayTimer(T &machine);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::RunSoundTimer(T &machine);
				template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <HasRendererSupport T>
				friend void Hyper_BandCHIP::DisplayRender(T &machine);
			private:
				std::array<uint8_t, 65536> memory;
				double cycle_rate;
				uint32_t cycles_per_second;
				uint16_t PC;
				uint16_t I;
				uint16_t SP;
				std::filebuf rpl_user_flags_file;
				XOCHIP_BehaviorData behavior_data;
				XOCHIP_Audio audio_system;
				Renderer *DisplayRenderer;
				unsigned char delay_timer;
				unsigned char sound_timer;
				std::array<uint8_t, 16> V;
				std::array<uint16_t, 32> stack;
				std::vector<unsigned char> draw_buffer;
				std::array<unsigned char, 128 * 64> display;
				uint8_t plane;
				const unsigned short display_width = 128;
				const unsigned short display_height = 64;
				unsigned char key_status[0x10];
				unsigned char key_pressed;
				mt19937 rng_engine;
				uniform_int_distribution<unsigned char> rng_distrib;
				high_resolution_clock::time_point current_tp;
				high_resolution_clock::time_point cycle_tp;
				double cycle_accumulator;
				SyncState RefreshSync;
				SyncState DelayTimerSync;
				SyncState SoundTimerSync;
				ResolutionMode CurrentResolutionMode;
				bool pause;
				bool operational;
				bool wait_for_key_release;
				MachineError error_state;
		};

		class HyperCHIP64_Machine
		{
			public:
				HyperCHIP64_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~HyperCHIP64_Machine();

				void ExecuteInstructions();

				template <HasResolutionModeSupport T>
				friend void Hyper_BandCHIP::SetResolutionMode(T &machine, ResolutionMode Mode);
				template <HasRegisterSupport T>
				friend void Hyper_BandCHIP::InitializeRegisters(T &machine);
				template <HasTimerSupport T, typename T2> requires std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::InitializeTimers(T &machine);
				template <HasStackSupport T>
				friend void Hyper_BandCHIP::InitializeStack(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::InitializeMemory(T &machine);
				template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <typename T, typename T2> requires HasSampleAudioSupport<T> && HasVoiceSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::InitializeAudio(T &machine);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::InitializeKeyStatus(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size);
				template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
				friend void Hyper_BandCHIP::GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height);
				template <typename T, typename T2> requires HasSyncSupport<T> && HasCycleSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::SetCyclesPerSecond(T &machine, unsigned int cycles_per_second);
				template <HasCycleSupport T>
				friend unsigned int Hyper_BandCHIP::GetCyclesPerFrame(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::SetDelayTimer(T &machine, unsigned char delay_timer);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && HasVoiceSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::SetKeyStatus(T &machine, unsigned char key, bool pressed);
				template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
				friend bool Hyper_BandCHIP::LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size);
				template <typename T, typename T2> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSampleAudioSupport<T> && HasPauseSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <HasPauseSupport T>
				friend bool Hyper_BandCHIP::IsPaused(T &machine);
				template <HasOperationalSupport T>
				friend bool Hyper_BandCHIP::IsOperational(T &machine);
				template <HasErrorStateSupport T>
				friend MachineError Hyper_BandCHIP::GetErrorState(T &machine);
				template <typename T, typename T2> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend MachineState Hyper_BandCHIP::GetMachineState(T &machine);
				template <HasCurrentTimeSupport T>
				friend void Hyper_BandCHIP::SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::RunDelayTimer(T &machine);
				template <typename T, typename T2, uint8_t voice> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSampleAudioSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::RunSoundTimer(T &machine);
				template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, XOCHIP_Audio>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <typename T, typename T2> requires HasTimerSupport<T> && HasSyncSupport<T> && HasRendererSupport<T> && std::same_as<T2, HyperCHIP64_Audio>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <HasRendererSupport T>
				friend void Hyper_BandCHIP::DisplayRender(T &machine);
			private:
				std::array<uint8_t, 65536> memory;
				double cycle_rate;
				uint32_t cycles_per_second;
				uint16_t PC;
				uint16_t I;
				uint16_t SP;
				std::filebuf rpl_user_flags_file;
				HyperCHIP64_Audio audio_system;
				Renderer *DisplayRenderer;
				uint8_t delay_timer;
				std::array<uint8_t, 4> sound_timer;
				std::array<uint8_t, 16> V;
				std::array<uint16_t, 32> stack;
				std::vector<uint8_t> draw_buffer;
				std::array<uint8_t, 128 * 64> display;
				uint8_t plane;
				uint8_t voice;
				const uint16_t display_width = 128;
				const uint16_t display_height = 64;
				uint8_t key_status[0x10];
				uint8_t key_pressed;
				mt19937 rng_engine;
				uniform_int_distribution<uint8_t> rng_distrib;
				high_resolution_clock::time_point current_tp;
				high_resolution_clock::time_point cycle_tp;
				double cycle_accumulator;
				SyncState RefreshSync;
				SyncState DelayTimerSync;
				std::array<SyncState, 4> SoundTimerSync;
				ResolutionMode CurrentResolutionMode;
				bool pause;
				bool operational;
				bool wait_for_key_release;
				MachineError error_state;
		};

		class Pipelined_CHIP8_Machine
		{
			public:
				Pipelined_CHIP8_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~Pipelined_CHIP8_Machine();

				inline void StoreBehaviorData(const CHIP8_BehaviorData *source_behavior_data)
				{
					behavior_data = *source_behavior_data;
				}
			private:
				CHIP8_BehaviorData behavior_data;
		};

		class Superscalar_CHIP8_Machine
		{
			public:
				Superscalar_CHIP8_Machine(unsigned int cycles_per_second = 600, Renderer *DisplayRenderer = nullptr);
				~Superscalar_CHIP8_Machine();

				inline void StoreBehaviorData(const CHIP8_BehaviorData *source_behavior_data)
				{
					behavior_data = *source_behavior_data;
				}

				void ExecuteInstructions();
				static void InstructionExecution(Superscalar_CHIP8_Machine *machine, ExecutionUnitData *current_execution_unit);

				template <HasRegisterSupport T>
				friend void Hyper_BandCHIP::InitializeRegisters(T &machine);
				template <HasTimerSupport T>
				friend void Hyper_BandCHIP::InitializeTimers(T &machine);
				template <HasStackSupport T>
				friend void Hyper_BandCHIP::InitializeStack(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::InitializeMemory(T &machine);
				template <HasDisplaySupport T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <typename T> requires HasDisplaySupport<T> && HasPlaneSupport<T>
				friend void Hyper_BandCHIP::InitializeVideo(T &machine);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::InitializeKeyStatus(T &machine);
				template <HasMemorySupport T>
				friend void Hyper_BandCHIP::CopyDataToInterpreterMemory(T &machine, const unsigned char *source, unsigned short address, unsigned int size);
				template <typename T> requires HasDisplaySupport<T> && HasDisplaySizeSupport<T>
				friend void Hyper_BandCHIP::GetDisplay(T &machine, unsigned char **display, unsigned short *display_width, unsigned short *display_height);
				template <typename T> requires HasSyncSupport<T> && HasCycleSupport<T>
				friend void Hyper_BandCHIP::SetCyclesPerSecond(T &machine, unsigned int cycles_per_second);
				template <HasCycleSupport T>
				friend unsigned int Hyper_BandCHIP::GetCyclesPerFrame(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::SetDelayTimer(T &machine, unsigned char delay_timer);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::SetSoundTimer(T &machine, unsigned char sound_timer);
				template <HasKeyStatusSupport T>
				friend void Hyper_BandCHIP::SetKeyStatus(T &machine, unsigned char key, bool pressed);
				template <typename T> requires HasProgramCounterSupport<T> && HasMemorySupport<T> && HasCycleSupport<T> && HasOperationalSupport<T> && HasErrorStateSupport<T>
				friend bool Hyper_BandCHIP::LoadProgram(T &machine, const unsigned char *source, unsigned short start_address, unsigned int size);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasCycleSupport<T> && HasSynthesizerAudioSupport<T> && HasPauseSupport<T>
				friend void Hyper_BandCHIP::PauseProgram(T &machine, bool pause);
				template <HasPauseSupport T>
				friend bool Hyper_BandCHIP::IsPaused(T &machine);
				template <HasOperationalSupport T>
				friend bool Hyper_BandCHIP::IsOperational(T &machine);
				template <HasErrorStateSupport T>
				friend MachineError Hyper_BandCHIP::GetErrorState(T &machine);
				template <typename T> requires HasProgramCounterSupport<T> && HasRegisterSupport<T> && HasTimerSupport<T>
				friend MachineState Hyper_BandCHIP::GetMachineState(T &machine);
				template <HasCurrentTimeSupport T>
				friend void Hyper_BandCHIP::SetCurrentTime(T &machine, const high_resolution_clock::time_point current_tp);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T>
				friend void Hyper_BandCHIP::RunDelayTimer(T &machine);
				template <typename T> requires HasCurrentTimeSupport<T> && HasTimerSupport<T> && HasSyncSupport<T> && HasSynthesizerAudioSupport<T>
				friend void Hyper_BandCHIP::RunSoundTimer(T &machine);
				template <typename T> requires HasTimerSupport<T> && HasVIPDisplayInterruptSupport<T> && HasSyncSupport<T> && HasRendererSupport<T>
				friend void Hyper_BandCHIP::SyncToCycle(T &machine);
				template <HasRendererSupport T>
				friend void Hyper_BandCHIP::DisplayRender(T &machine);
			private:
				CHIP8_BehaviorData behavior_data;
				Audio audio_system;
				Renderer *DisplayRenderer;
				bool display_interrupt;
				unsigned int cycles_per_second;
				double cycle_rate;
				unsigned char delay_timer;
				unsigned char sound_timer;
				std::array<uint8_t, 16> V;
				unsigned short PC;
				unsigned short I;
				unsigned short SP;
				std::array<uint16_t, 32> stack;
				std::vector<unsigned char> memory;
				std::array<unsigned char, 64 * 32> display;
				const unsigned short display_width = 64;
				const unsigned short display_height = 32;
				unsigned char key_status[0x10];
				unsigned char key_pressed;
				mt19937 rng_engine;
				uniform_int_distribution<unsigned char> rng_distrib;
				high_resolution_clock::time_point current_tp;
				high_resolution_clock::time_point cycle_tp;
				double cycle_accumulator;
				SyncState RefreshSync;
				SyncState DelayTimerSync;
				SyncState SoundTimerSync;
				bool pause;
				bool operational;
				bool wait_for_key_release;
				bool instruction_fetch;
				bool render_display;
				uint8_t instructions_to_execute;
				MachineError error_state;
				std::array<ExecutionUnitData, 1> execution_unit;
				std::mutex instruction_fetch_mutex;
				std::array<std::mutex, 16> V_mutex;
		};
	}
}

#endif
