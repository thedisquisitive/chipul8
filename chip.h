#ifndef CHIP8VM_H_
#define CHIP8VM_H_

#include <SDL.h>
#include <cstdio>

using namespace std;

#define dbgMode 0

const unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8VM
{
private:
    unsigned short opCode;
    unsigned short pc;
    unsigned short I;
    unsigned char memory[4096];
    unsigned char V[16];
    unsigned char gfx[64*32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short system_stack[16];
    unsigned short sp;
    unsigned char key[16];
    bool drawFlag;
    char *romFile;
    bool paused;
public:
    int loadRom(const char *f); // Load a rom into memory
    int reset();                // Reset the machine but keep the current rom
    int setPause(bool p);       // Toggle pause setting
    int stepCPU();              // Perform CPU cycle
    int renderScreen(SDL_Renderer *scr); // Render console
    void setKey(int k, int s) { key[k] = s; }
    void dumpMem();
    void dbgStep(); // Print memory map data, stepping through prog.
    unsigned char* getMemory(); // Return memory array
    // Return specific register
    unsigned char getRegister(int reg) { if (reg > -1 && reg < 16) return V[reg]; }
    // Return delay timer
    unsigned char getDelay() { return delay_timer; }
    // Return sound timer
    unsigned char getSound() { return sound_timer; }
    // Return PC, opCode, etc
    unsigned short getOpCode() { return opCode; }
    unsigned short getPC() { return pc; }
    // Return paused state
    bool getPaused() { return paused; }

    // OpCode Functions
    void opClearScreen();
    void opReturn();
    void opGoto();
    void opCallRoutine();
    void opSkip_If_Equal();
    void opSkip_If_Not_Equal();
    void opSkip_If_VX_Equals_VY();
    void opSet_VX();
    void opAdd_To_VX();
    void opSet_VX_To_VY();
    void opAdd_VY_To_VX();
    void opVX_OR_VY();
    void opVX_AND_VY();
    void opVX_XOR_VY();
    void opSubtract_VY_From_VX();
    void opShift_VX_Right();
    void opSubtract_VX_From_VY();
    void opShift_VX_Left();
    void opSkip_If_VX_Not_Equal_VY();
    void opSet_I();
    void opJump_To_NNN_Plus_V0();
    void opRand();
    void opDrawSprite();
    void opSkip_If_KeyPressed();
    void opSkip_If_Key_NotPressed();
    void opSet_VX_To_Delay();
    void opWait_For_KeyPress();
    void opSet_Delay();
    void opSet_Sound();
    void opAdd_VX_To_I();
    void opSet_I_To_VX();
    void opSet_BCD();
    void opReg_Dump();
    void opReg_Load();

    // Debug Output
    void PrintMachineData();
};

#endif // CHIP8VM_H_
