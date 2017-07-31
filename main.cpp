/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#include "chip.h"


//Screen dimension constants
const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 480;

// Screen, window, test image
SDL_Window *gWindow = NULL;
SDL_Surface *gTestImage, *gRenderScreen = NULL;

SDL_Renderer *gRenderer = NULL;

Chip8VM myChip;

//Functions

//init() : Initialize SDL and app
bool init()
{
    bool result = true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Could not init SDL: %s\n", SDL_GetError());
        result = false;
    }
    else
    {
        gWindow = SDL_CreateWindow("Chipul8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Could not create window: %s\n", SDL_GetError());
            result = false;
        }
        else
        {
            int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
            if (!(IMG_Init(imgFlags) & imgFlags))
            {
                printf("SDL_Image failed to init: %s\n", IMG_GetError());
                result = false;
            }
            gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
        }
    }
    return result;
}

//loadMedia() : load media...
bool loadMedia()
{
    bool result = true;

    if (myChip.loadRom("data/roms/Chip-8 Programs/IBM Logo.ch8"))
    {
        myChip.dumpMem();
        result = true;
    }
    else
        result = false;

    return result;
}


//close() : Free memory and close app
void close()
{
    SDL_FreeSurface(gTestImage);
    gTestImage = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}


int main( int argc, char* args[] )
{
	if (!init())
    {
        printf("Couldn't run, init failure!\n");
    }
    else
    {
        if (!loadMedia())
        {
            printf("Failed to load data, aborting.\n");
        }
        else
        {
            bool running = true;
            SDL_Event event;
            while (running)
            {
                while (SDL_PollEvent(&event) != 0)
                {
                    if (event.type == SDL_QUIT)
                    {
                        running = false;
                    }
                    else if (event.type == SDL_KEYUP)
                    {
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_1:
                                myChip.setKey(0x1, 0);
                                break;
                            case SDLK_2:
                                myChip.setKey(0x2, 0);
                                break;
                            case SDLK_3:
                                myChip.setKey(0x3, 0);
                                break;
                            case SDLK_4:
                                myChip.setKey(0xC, 0);
                                break;
                            case SDLK_q:
                                myChip.setKey(0x4, 0);
                                break;
                            case SDLK_w:
                                myChip.setKey(0x5, 0);
                                break;
                            case SDLK_e:
                                myChip.setKey(0x6, 0);
                                break;
                            case SDLK_r:
                                myChip.setKey(0xD, 0);
                                break;
                            case SDLK_a:
                                myChip.setKey(0x7, 0);
                                break;
                            case SDLK_s:
                                myChip.setKey(0x8, 0);
                                break;
                            case SDLK_d:
                                myChip.setKey(0x9, 0);
                                break;
                            case SDLK_f:
                                myChip.setKey(0xE, 0);
                                break;
                            case SDLK_z:
                                myChip.setKey(0xA, 0);
                                break;
                            case SDLK_x:
                                myChip.setKey(0x0, 0);
                                break;
                            case SDLK_c:
                                myChip.setKey(0xB, 0);
                                break;
                            case SDLK_v:
                                myChip.setKey(0xF, 0);
                                break;
                            default:
                                printf("Unknown key up: %c\n", event.key.keysym.sym);
                                break;
                        }
                    }
                    else if (event.type == SDL_KEYDOWN)
                    {
                        switch (event.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:
                                running = false;
                                break;
                            case SDLK_1:
                                myChip.setKey(0x1, 1);
                                break;
                            case SDLK_2:
                                myChip.setKey(0x2, 1);
                                break;
                            case SDLK_3:
                                myChip.setKey(0x3, 1);
                                break;
                            case SDLK_4:
                                myChip.setKey(0xC, 1);
                                break;
                            case SDLK_q:
                                myChip.setKey(0x4, 1);
                                break;
                            case SDLK_w:
                                myChip.setKey(0x5, 1);
                                break;
                            case SDLK_e:
                                myChip.setKey(0x6, 1);
                                break;
                            case SDLK_r:
                                myChip.setKey(0xD, 1);
                                break;
                            case SDLK_a:
                                myChip.setKey(0x7, 1);
                                break;
                            case SDLK_s:
                                myChip.setKey(0x8, 1);
                                break;
                            case SDLK_d:
                                myChip.setKey(0x9, 1);
                                break;
                            case SDLK_f:
                                myChip.setKey(0xE, 1);
                                break;
                            case SDLK_z:
                                myChip.setKey(0xA, 1);
                                break;
                            case SDLK_x:
                                myChip.setKey(0x0, 1);
                                break;
                            case SDLK_c:
                                myChip.setKey(0xB, 1);
                                break;
                            case SDLK_v:
                                myChip.setKey(0xF, 1);
                                break;
                            case SDLK_p:
                                if (dbgMode)
                                {
                                    //myChip.dbgStep();
                                    myChip.stepCPU();
                                    myChip.PrintMachineData();
                                }
                                break;
                            default:
                                printf("Unknown key pressed: %c\n", event.key.keysym.sym);
                                break;
                        }
                    }
                }
            if (!dbgMode)
            {
                myChip.stepCPU();


            }
            myChip.renderScreen(gRenderer);
            SDL_RenderPresent(gRenderer);
            //SDL_UpdateWindowSurface(gWindow);
            SDL_Delay(1);
            }
        }
    }
    close();
	return 0;
}
