#include "Window.h"
#include "Assert.h"
#include <sstream>

using namespace std;
using namespace std::chrono;
using namespace Wolverine;



SdlCanvas::SdlCanvas(SDL_Surface &sdlSurface) : m_Surface(&sdlSurface)
{
}

size_t SdlCanvas::getWidth()
{
	return m_Surface->w;
}

size_t SdlCanvas::getHeight()
{
	return m_Surface->h;
}

void SdlCanvas::clear()
{
	SDL_memset(m_Surface->pixels, 0, m_Surface->h * m_Surface->pitch);
}

void SdlCanvas::lock()
{
	SDL_LockSurface(m_Surface);
}

void SdlCanvas::unlock()
{
	SDL_UnlockSurface(m_Surface);
}


Window::Window(std::string const &title, int resX, unsigned int resY)
{
	m_Title = title;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ERROR("SDL could not initialize!");
		//printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else
	{
		//Create window
		m_Window = SDL_CreateWindow(m_Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resX, resY, SDL_WINDOW_SHOWN);
		if (m_Window == NULL)
		{
			ERROR("Window coult not be created!");
			//printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else
		{
			m_Canvas = make_unique<SdlCanvas>(*SDL_GetWindowSurface(m_Window));
		}
	}
}

Window::~Window()
{
	SDL_DestroyWindow(m_Window);
}

SdlCanvas& Window::getCanvas()
{
	return *m_Canvas;
}

void Window::update()
{
	++m_Fps;

	SDL_UpdateWindowSurface(m_Window);

	high_resolution_clock::time_point stop = high_resolution_clock::now();

	auto timeDiff = duration_cast<microseconds>(stop - m_LastTime).count();

	static const long oneSecond = 1000000;
	if (timeDiff >= oneSecond)
	{
		double fps = ((double)m_Fps * oneSecond) / timeDiff;

		std::stringstream caption;
		caption << m_Title << " FPS=" << (unsigned int)fps;

		SDL_SetWindowTitle(m_Window, caption.str().c_str());

		m_Fps = 0;
		m_LastTime = stop;
	}
	
}