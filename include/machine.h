#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <fstream>
#include <array>
#include <chrono>
#include <random>
#include <variant>
#include "audio.h"

using namespace std::chrono;
using std::mt19937;
using std::uniform_int_distribution;

namespace Hyper_BandCHIP
{
	class Machine;

	using XOCHIP_Audio = SampleAudio<1, 1>;
	using HyperCHIP64_Audio = SampleAudio<2, 4>;

	enum class MachineCore
	{
		BandCHIP_CHIP8, // BandCHIP CHIP-8 Core
		BandCHIP_SuperCHIP, // BandCHIP SuperCHIP Core
		BandCHIP_XOCHIP, // BandCHIP XO-CHIP Core
		BandCHIP_HyperCHIP64 // BandCHIP HyperCHIP-64 Core
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

	template <MachineCore T>
	struct InstructionData
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

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

	struct MachineState
	{
		// unsigned char V[0x10];
		RegisterData_8Bit V;
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
			
			void RunSoundTimer();
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
}

#endif
