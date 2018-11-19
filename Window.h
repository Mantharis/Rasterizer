#pragma once
#include "Assert.h"

#include <string>
#include <memory>
#include <chrono>
#include <SDL.h>

class SDL_Window;
class SDL_Surface;

namespace Wolverine
{



	class SdlCanvas
	{
	public:
		SdlCanvas(SDL_Surface &sdlSurface);

		unsigned int GetWidth();
		unsigned int GetHeight();
		void Clear();
		void Lock();
		void Unlock();
		void Write(unsigned int x, unsigned int y, float red, float green, float blue);
	private:
		SDL_Surface *m_Surface;
	};


	__forceinline void SdlCanvas::SdlCanvas::Write(unsigned int x, unsigned int y, float red, float green, float blue)
	{
		ASSERT(x < m_Surface->w && y < m_Surface->h, "Index is out of range");

		uint8_t *target_pixel = (uint8_t *)m_Surface->pixels + y * m_Surface->pitch + x * 4;

		target_pixel[2] = (uint8_t)(red);
		target_pixel[1] = (uint8_t)(green);
		target_pixel[0] = (uint8_t)(blue);
		target_pixel[3] = 255;
	}



	class Window
	{
	public:
		Window(std::string const &title, int resX, unsigned int resY);
		~Window();

		void Update();
		SdlCanvas& GetCanvas();

	private:
		std::string m_Title;
		long long m_Fps = 0;
		std::chrono::high_resolution_clock::time_point m_LastTime;

		SDL_Window *m_Window;
		std::unique_ptr<SdlCanvas> m_Canvas;
	};
	
}

