#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <fstream>
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

	enum class MachineCore
	{
		BandCHIP_CHIP8, // BandCHIP CHIP-8 Core
		BandCHIP_SuperCHIP, // BandCHIP SuperCHIP Core
		BandCHIP_HyperCHIP64 // BandCHIP HyperCHIP-64 Core
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
		SuperCHIP10, // Emulates the original SuperCHIP V1.0 behavior. (Likely needed for games and programs that were assembled for this version.)
		SuperCHIP11 // Emulates the original SuperCHIP V1.1 behavior (with CHIP-48 shifting and load/store).
	};

	enum class ResolutionMode
	{
		LoRes, // 64x32
		HiRes // 128x64
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
	};

	struct SuperCHIP_BehaviorData
	{
		SuperCHIPVersion Version;
	};

	template<>
	struct InstructionData<MachineCore::BandCHIP_CHIP8>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	template<>
	struct InstructionData<MachineCore::BandCHIP_SuperCHIP>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	template<>
	struct InstructionData<MachineCore::BandCHIP_HyperCHIP64>
	{
		void operator()(Machine *TargetMachine);
		unsigned char opcode;
		unsigned short operand;
	};

	struct MachineState
	{
		unsigned char V[0x10];
		unsigned short PC;
		unsigned short I;
		unsigned char DT;
		unsigned char ST;
	};

	class Machine
	{
		public:
			Machine(MachineCore Core = MachineCore::BandCHIP_CHIP8, unsigned int cycles_per_second = 600, unsigned int memory_size = 0x1000, unsigned short display_width = 64, unsigned short display_height = 32);
			~Machine();
			template <typename T>
			void StoreBehaviorData(const T *source_behavior_data)
			{
				behavior_data = *source_behavior_data;
			}
			void SetResolutionMode(ResolutionMode Mode);
			void InitializeRegisters();
			void InitializeTimers();
			void InitializeStack();
			void InitializeMemory();
			void InitializeVideo();
			void InitializeKeyStatus();
			void CopyDataToInterpreterMemory(const unsigned char *source, unsigned short address, unsigned int size);
			void GetDisplay(unsigned char **display, unsigned short *display_width, unsigned short *display_height);
			void SetDelayTimer(unsigned char delay_timer);
			void SetSoundTimer(unsigned char sound_timer);
			void SetKeyStatus(unsigned char key, bool pressed);
			bool LoadProgram(const unsigned char *source, unsigned short start_address, unsigned int size);
			void PauseProgram(bool pause = true);
			bool IsPaused() const;
			MachineCore GetMachineCore() const;
			MachineError GetErrorState() const;
			MachineState GetMachineState() const;
			void SetCurrentTime(const high_resolution_clock::time_point current_tp);
			void ExecuteInstructions();
			void RunDelayTimer();
			void RunSoundTimer();
			friend struct InstructionData<MachineCore::BandCHIP_CHIP8>;
			friend struct InstructionData<MachineCore::BandCHIP_SuperCHIP>;
			friend struct InstructionData<MachineCore::BandCHIP_HyperCHIP64>;
		private:
			MachineCore CurrentMachineCore;
			ResolutionMode CurrentResolutionMode;
			std::filebuf rpl_user_flags_file;
			std::variant<CHIP8_BehaviorData, SuperCHIP_BehaviorData> behavior_data;
			Audio audio;
			unsigned int cycles_per_second;
			unsigned char delay_timer;
			unsigned char sound_timer;
			unsigned char V[16];
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
			unsigned char prefix_flags;
			unsigned char address_nibble_store;
			unsigned char register_store;
			mt19937 rng_engine;
			uniform_int_distribution<unsigned char> rng_distrib;
			high_resolution_clock::time_point current_tp;
			high_resolution_clock::time_point cycle_tp;
			double cycle_accumulator;
			high_resolution_clock::time_point dt_tp;
			double dt_accumulator;
			high_resolution_clock::time_point st_tp;
			double st_accumulator;
			bool pause;
			MachineError error_state;
	};
}

#endif
