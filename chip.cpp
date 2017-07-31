#include "chip.h"
#include <cstdlib>
#include <ctime>

using namespace std;

#define VX V[(opCode & 0x0F00) >> 8]
#define VY V[(opCode & 0x00F0) >> 4]
#define StepPC pc += 2

int Chip8VM::loadRom(const char *f)
{
    printf("Initializing VM...\n");

    pc = 0x200;
    opCode = 0;
    I = 0;
    sp = 0;
    delay_timer = sound_timer = 0;

    drawFlag = true;

    for (int i = 0; i < 2048; i++)
        gfx[i] = 0;

    for (int i = 0; i < 16; i++)
    {
        system_stack[i] = 0;
        V[i] = 0;
        key[i] = 0;
    }

    for (int i = 0; i < 4096; i++)
        memory[i] = 0;

    for (int i = 0; i < 80; i++)
    {
        memory[i] = chip8_fontset[i];
    }

    srand(time(NULL));

    printf("Loading %s...\n", f);
    FILE *pFile = fopen(f, "rb");
    if (pFile == NULL)
    {
        printf("Cannot load file.\n");
        return 0;
    }
    fseek(pFile, 0, SEEK_END);
    long lSz = ftell(pFile);
    rewind(pFile);
    printf("Filesize: %d\n", lSz);

    char *buffer = (char*)malloc(sizeof(char) * lSz);
    if (buffer == NULL)
    {
        printf("Memory error!\n");
        return 0;
    }

    size_t result = fread(buffer, 1, lSz, pFile);
    if (result != lSz)
    {
        printf("Reading error!\n");
        return 0;
    }

    if ((4096-512) > lSz)
    {
        for (int i = 0; i < lSz; i++)
            memory[512+i] = buffer[i];
    }
    else
    {
        printf("ROM is too big for memory. Is it a regular CHIP8 program?\n");
        return 0;
    }

    fclose(pFile);
    free(buffer);


    //sp = 0;
    return 1;
}


int Chip8VM::setPause(bool p)
{
    paused = p;
    return 1;
}


void Chip8VM::dbgStep()
{
    printf("V:\n");
    for (int i = 0; i < 16; i++)
        printf("%x\n", V[i]);

    printf("I: %x\n", I);
    printf("Op: %x\n", opCode);
    printf("SP: %i\n", (int)sp);

}
int Chip8VM::stepCPU()
{
    opCode = memory[pc] << 8 | memory[pc+1];

    switch (opCode &0xF000)
    {
        case 0x0000:
            switch(opCode & 0x000F)
            {
            case 0x0000: // 0x00E0: Clear the screen
                opClearScreen();
                break;
            case 0x000E: // 0x00EE: Return from subroutine
                opReturn();
                break;
            default:
                printf("Unknown opcode: 0x%x\n", opCode);
                StepPC;
            }
            break;
        case 0x1000: // 0x1NNN: Jump to address NNN
            opGoto();
            break;
        case 0x2000: // 0x2NNN: Call subroutine at NNN
            opCallRoutine();
            break;
        case 0x3000: // 0x3XNN: Skip next instruction in V[X] == NN
            opSkip_If_Equal();
            break;
        case 0x4000: // 0x4XNN: Skip if V[X] != NN
            opSkip_If_Not_Equal();
            break;
        case 0x5000: // 0x5XY0: Skip if V[X] == V[Y]
            opSkip_If_VX_Equals_VY();
            break;
        case 0x6000: // 0x6XNN: Set V[X] to NN
            opSet_VX();
            break;
        case 0x7000: // 0x7XNN: Add NN to V[X]
            opAdd_To_VX();
            break;
        case 0x8000:
            switch (opCode &0x000F)
            {
            case 0x0000: // 0x8XY0: Set V[X] to V[Y]
                opSet_VX_To_VY();
                break;
            case 0x0001: // 0x8XY1: Set VX to VX | VY
                opVX_OR_VY();
                break;
            case 0x0002: // 0x8XY2: Set VX to VX & VY
                opVX_AND_VY();
                break;
            case 0x0003: // 0x8XY3: Set VX to VX ^ VY
                opVX_XOR_VY();
                break;
            case 0x0004: // 0x8XY4: VX += VY
                opAdd_VY_To_VX();
                break;
            case 0x0005: // 0x8XY5: VX -= VY
                opSubtract_VY_From_VX();
                break;
            case 0x0006: // 0x8XY6: Shift VX >>= 1
                opShift_VX_Right();
                break;
            case 0x0007: // 0x8XY7: VX = VY-VX
                opSubtract_VY_From_VX();
                break;
            case 0x000E: // 0x8XYE: Shift VX <<= 1
                opShift_VX_Left();
                break;
            default:
                printf("Unknown opcode: 0x%x\n", opCode);
                StepPC;
            }
            break;
        case 0x9000: // 0x9XY0: Skip if VX != VY
            opSkip_If_VX_Not_Equal_VY();
            break;
        case 0xA000: // 0xANNN: Set I to NNN
            opSet_I();
            break;
        case 0xB000: // 0xBNNN: Jump to NNN + V0
            opJump_To_NNN_Plus_V0();
            break;
        case 0xC000: // 0xCXNN: Set VX to Rand & NN
            opRand();
            break;
        case 0xD000: // 0xDXYN: Draw sprite at VX,VY with height of N
            opDrawSprite();
            break;
        case 0xE000:
            switch(opCode &0x00FF)
            {
            case 0x009E:
                opSkip_If_KeyPressed();
                break;
            case 0x00A1:
                opSkip_If_Key_NotPressed();
                break;
            default:
                printf("Unknown opCode: %x\n", opCode);
                StepPC;
            }
            break;
        case 0xF000:
            switch (opCode & 0x00FF)
            {
            case 0x0007: // 0xFX07: Set VX to Delay Timer
                opSet_VX_To_Delay();
                break;
            case 0x000A: // FX0A: Wait for keypress, store in VX
                opWait_For_KeyPress();
                break;
            case 0x0015: // FX15: Set delay timer to VX
                opSet_Delay();
                break;
            case 0x0018: // FX18: Set Sound timer to VX
                opSet_Sound();
                break;
            case 0x001E: // FX1E: Ad VX to I
                opAdd_VX_To_I();
                break;
            case 0x0029: // FX29: Set I to location of sprite for character in VX
                opSet_I_To_VX();
                break;
            case 0x0033: // FX33: Store binary coded VX at I to I+2
                opSet_BCD();
                break;
            case 0x0055: //FX55: Store V0 to VX in memory at I
                opReg_Dump();
                break;
            case 0x0065: // FX65: Fill V0 to VX with I
                opReg_Load();
                break;
            default:
                printf("Unknown opcode: 0x%x\n", opCode);
                StepPC;
            }
            break;
        default:
            printf("Unknown opcode: 0x%x\n", opCode);
            StepPC;
            break;
    }
    if (delay_timer > 0)
        delay_timer--;
    if (sound_timer > 0)
        sound_timer--;
    return 1;
}

int Chip8VM::renderScreen(SDL_Renderer *scr)
{
    if (drawFlag)
    {
        for (int y = 0; y < 32; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                if (gfx[(y*64)+x] == 0)
                {
                    SDL_Rect pix;
                    pix.x = x*(720/64);
                    pix.w = (720/64);
                    pix.y = y*(480/32);
                    pix.h = (480/32);
                    SDL_SetRenderDrawColor(scr, 0, 0, 20, 255);
                    SDL_RenderFillRect(scr, &pix);
                }
                else
                {
                    SDL_Rect pix;
                    pix.x = x*(720/64);
                    pix.w = (720/64);
                    pix.y = y*(480/32);
                    pix.h = (480/32);
                    SDL_SetRenderDrawColor(scr, 0, 100, 0, 255);
                    SDL_RenderFillRect(scr, &pix);
                }
            }
        }
        drawFlag = false;
    }
}

void Chip8VM::opClearScreen()
{
    if (dbgMode)
        printf("opCode 00E0: Clear the Screen.\n");
    for (int i = 0; i < 2048; i++)
        gfx[i] = 0;
    StepPC;
}

void Chip8VM::opReturn()
{
    if (dbgMode)
        printf("opCode 00EE: Return from subroutine.\n");
    sp--;
    pc = system_stack[sp];
    StepPC;
}

void Chip8VM::opGoto()
{
    if (dbgMode)
        printf("opCode 1NNN: GOTO NNN\n");
    pc = (opCode & 0x0FFF);
}

void Chip8VM::opCallRoutine()
{
    if (dbgMode)
        printf("opCode 2NNN: Call routine at NNN.\n");
    system_stack[sp] = pc;
    sp++;
    pc = (opCode & 0x0FFF);
}

void Chip8VM::opSkip_If_Equal()
{
    if (dbgMode)
        printf("opCode 3XNN: Skip if VX equals NN\n");
    if (VX == (opCode &0x00FF))
        StepPC;
    StepPC;
}

void Chip8VM::opSkip_If_Not_Equal()
{
    if (dbgMode)
        printf("opCode 4XNN: Skip if VX does not equal NN\n");
    if (VX != (opCode &0x00FF))
        StepPC;
    StepPC;
}

void Chip8VM::opSkip_If_VX_Equals_VY()
{
    if (dbgMode)
        printf("opCode 5XY0: Skip if VX equals VY\n");
    if (VX = VY)
        StepPC;
    StepPC;
}

void Chip8VM::opSet_VX()
{
    if (dbgMode)
        printf("opCode 6XNN: Set VX to NN\n");
    VX = (opCode &0x00FF);
    StepPC;
}

void Chip8VM::opAdd_To_VX()
{
    if (dbgMode)
        printf("opCode 7XNN: Add NN to VX\n");
    VX += (opCode &0x00FF);
    StepPC;
}

void Chip8VM::opSet_VX_To_VY()
{
    if (dbgMode)
        printf("opCode 8XY0: Set VX to VY\n");
    VX = VY;
    StepPC;
}

void Chip8VM::opVX_OR_VY()
{
    if (dbgMode)
        printf("opCode 8XY1: Bitwise OR XY|VY\n");
    VX |= VY;
    StepPC;
}

void Chip8VM::opVX_AND_VY()
{
    if (dbgMode)
        printf("opCode 8XY2: Bitwise AND VX&VY\n");
    VX &= VY;
    StepPC;
}

void Chip8VM::opVX_XOR_VY()
{
    if (dbgMode)
        printf("opCode 8XY3: Bitwise XOR VX^VY\n");
    VX ^= VY;
    StepPC;
}

void Chip8VM::opAdd_VY_To_VX()
{
    if (dbgMode)
        printf("opCode 8XY4: Add VY to VX\n");
    if(VY > (0xFF - VX))
        V[0xF] = 1;
    else
        V[0xF] = 0;
    VX += VY;
    StepPC;
}

void Chip8VM::opSubtract_VY_From_VX()
{
    if (dbgMode)
        printf("opcode 8XY5: Subtract VY from VX\n");
    if (VY > VX)
        V[0xF] = 0;
    else
        V[0xF] = 1;
    VX -= VY;
    StepPC;
}

void Chip8VM::opShift_VX_Right()
{
    if (dbgMode)
        printf("opCode 8XY6: Shift VX left\n");
    V[0xF] = VX &0x1;
    VX >>= 1;
    StepPC;
}

void Chip8VM::opSubtract_VX_From_VY()
{
    if (dbgMode)
        printf("opCode 8XY7: Set VX to VY-VX\n");
    if (VX > VY)
        V[0xF] = 0;
    else
        V[0xF] = 1;
    VX = VY-VX;
    StepPC;
}

void Chip8VM::opShift_VX_Left()
{
    if (dbgMode)
        printf("opCode 8XYE: Shift VX left\n");
    V[0xF] = VX >> 7;
    VX <<= 1;
    StepPC;
}

void Chip8VM::opSkip_If_VX_Not_Equal_VY()
{
    if (dbgMode)
        printf("opCode 9XY0: Skip if VX not equal to VY\n");
    if (VX != VY)
        StepPC;
    StepPC;
}

void Chip8VM::opSet_I()
{
    if (dbgMode)
        printf("opCode ANNN: Set I to NNN\n");
    I = (opCode & 0x0FFF);
    StepPC;
}

void Chip8VM::opJump_To_NNN_Plus_V0()
{
    if (dbgMode)
        printf("opCode BNNN: Jump to NNN + V0\n");
    pc = (opCode & 0x0FFF) + V[0];
}

void Chip8VM::opRand()
{
    if (dbgMode)
        printf("opCode CXNN: Set VX to rand()&NN\n");
    VX = ((rand() % 0xFF) & (opCode & 0x00FF));
    StepPC;
}

void Chip8VM::opDrawSprite()
{
    if (dbgMode)
        printf("opCode DXYN: Draw sprite at VX,VY, height of N\n");
    unsigned short x = VX;
    unsigned short y = VY;
    unsigned short height = (opCode & 0x000F);
    unsigned short pixel;
    V[0xF] = 0;

    for (int yline = 0; yline < height; yline++)
    {
        pixel = memory[I+yline];
        for (int xline = 0; xline < 8; xline++)
        {
            if ((pixel & (0x80 >> xline)) != 0)
            {
                if (gfx[(x + xline + ((y + yline) *64))] == 1)
                    V[0xF] = 1;
                gfx[x + xline + ((y + yline) *64)] ^= 1;
            }
        }
    }
    drawFlag = true;
    StepPC;
}

void Chip8VM::opSkip_If_KeyPressed()
{
    if (dbgMode)
        printf("opCode EX9E: Skip if Key Pressed\n");
    if (key[VX])
        StepPC;
    StepPC;
}

void Chip8VM::opSkip_If_Key_NotPressed()
{
    if (dbgMode)
        printf("opCode EXA1: Skip if Key Not Pressed\n");
    if (!key[VX])
        StepPC;
    StepPC;
}

void Chip8VM::opSet_VX_To_Delay()
{
    if (dbgMode)
        printf("opCode FX07: Set VX to delay timer\n");
    VX = delay_timer;
    StepPC;
}

void Chip8VM::opWait_For_KeyPress()
{
    if (dbgMode)
        printf("opCode FX0A: Wait for keypress\n");
    bool keyPress = false;
    for (int i = 0; i < 16; i++)
    {
        if (key[i] != 0)
        {
            VX = i;
            keyPress = true;
        }
    }
    if (!keyPress) return;
    StepPC;
}

void Chip8VM::opSet_Delay()
{
    if (dbgMode)
        printf("opCode FX15: Set delay to VX\n");
    delay_timer = VX;
    StepPC;
}

void Chip8VM::opSet_Sound()
{
    if (dbgMode)
        printf("opCode FX18: Set sound timer to VX\n");
    sound_timer = VX;
    StepPC;
}

void Chip8VM::opAdd_VX_To_I()
{
    if (dbgMode)
        printf("opCode FX1E: Add VX to I\n");
    I += VX;
    StepPC;
}

void Chip8VM::opSet_I_To_VX()
{
    if (dbgMode)
        printf("opCode FX29: Set I to location of sprite in VX\n");
    I = VX*0x5;
    StepPC;
}

void Chip8VM::opSet_BCD()
{
    if (dbgMode)
        printf("opCode FX33: Set Binary Coded Decimal of VX at I\n");
    memory[I] = VX/100;
    memory[I+1] = (VX / 10) % 10;
    memory[I+2] = (VX % 100) % 10;
    StepPC;
}

void Chip8VM::opReg_Dump()
{
    if (dbgMode)
        printf("opCode FX55: Store registers V0 to VX in memory at I\n");
    for (int i = 0; i <= VX; i++)
    {
        memory[I + i] = V[i];
    }
    I += VX + 1;
    StepPC;
}

void Chip8VM::opReg_Load()
{
    if (dbgMode)
        printf("opCode FX65: Fill V0 to VX with memory starting at I\n");
    for (int i = 0; i <= VX; i++)
    {
        V[i] = memory[I+i];
    }
    I += VX + 1;
    StepPC;
}

void Chip8VM::PrintMachineData()
{
    printf("Registers:\n");
    printf("V[0]: %.2x V[4]: %.2x V[8]: %.2x V[C]: %.2x\n", V[0x0], V[0x4], V[0x8], V[0xC]);
    printf("V[1]: %.2x V[5]: %.2x V[9]: %.2x V[D]: %.2x\n", V[0x1], V[0x5], V[0x9], V[0xD]);
    printf("V[2]: %.2x V[6]: %.2x V[A]: %.2x V[E]: %.2x\n", V[0x2], V[0x6], V[0xA], V[0xE]);
    printf("V[3]: %.2x V[7]: %.2x V[B]: %.2x V[F]: %.2x\n\n", V[0x3], V[0x7], V[0xB], V[0xF]);

    printf("Program Counter: %.3x | Stack Pointer: %i | opCode: %.4x\n\n", pc, sp, opCode);

    printf("Display:\n");
    int x = 0;
    for (int i = 0; i < 2048; i++)
    {
        printf("%i", gfx[i]);
        x++;
        if (x == 65)
        {
            printf("\n");
            x = 0;
        }
    }
}

void Chip8VM::dumpMem()
{
    FILE *pFile = fopen("memdump.txt", "w");
    for (int i = 0; i < 4096; i+=2)
    {
        fprintf(pFile, "%.3x-%.3x: %.2x%.2x\n", i, i+1, memory[i], memory[i+1]);
    }
    fclose(pFile);
}
