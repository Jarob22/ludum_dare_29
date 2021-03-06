#include "Window.h"
#include <SDL_ttf.h>
#include <SDL_image.h>


std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> Window::mWindow
= std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>(nullptr, SDL_DestroyWindow);
std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)> Window::mRenderer
= std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)>(nullptr, SDL_DestroyRenderer);
//Other static members
SDL_Rect Window::mBox;

Window::Window()
{
}


Window::~Window()
{
}


void Window::Init(int width, int height, std::string title){
	//initialize all SDL subsystems
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		throw std::runtime_error("SDL Init Failed");
	if (TTF_Init() == -1)
		throw std::runtime_error("TTF Init Failed");

	//Setup our window size
	mBox.x = 0;
	mBox.y = 0;
	mBox.w = width;
	mBox.h = height;
	//Create our window
	mWindow.reset(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, mBox.w, mBox.h, SDL_WINDOW_SHOWN));
	//Make sure it created ok
	if (mWindow == nullptr)
		throw std::runtime_error("Failed to create window");

	//Create the renderer
	mRenderer.reset(SDL_CreateRenderer(mWindow.get(), -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	//Make sure it created ok
	if (mRenderer == nullptr)
		throw std::runtime_error("Failed to create renderer");
}

void Window::Quit(){
	TTF_Quit();
	SDL_Quit();
}

void Window::Draw(SDL_Texture *tex, SDL_Rect &dstRect, SDL_Rect *clip, float angle,
	int xPivot, int yPivot, SDL_RendererFlip flip)
{
	//Convert pivot pos from relative to object's center to screen space
	xPivot += dstRect.w / 2;
	yPivot += dstRect.h / 2;
	//SDL expects an SDL_Point as the pivot location
	SDL_Point pivot = { xPivot, yPivot };
	//Draw the texture
	SDL_RenderCopyEx(mRenderer.get(), tex, clip, &dstRect, angle, &pivot, flip);
}

void Window::Draw(SDL_Texture *tex, int x, int y)
{
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	Window::Draw(tex, rect);
}

SDLTexture Window::LoadImage(const std::string& file){
	SDLTexture texture(IMG_LoadTexture(mRenderer.get(), file.c_str()));
	if (texture.get() == nullptr)
		throw std::runtime_error("Failed to load image: " + file + IMG_GetError());
	return texture;
}


SDLTexture Window::RenderText(const std::string& message, const std::string& fontFile,
	SDL_Color color, int fontSize)
{
	//Open the font
	TTF_Font *font = nullptr;
	font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr)
		throw std::runtime_error("Failed to load font: " + fontFile + TTF_GetError());

	//Render the message to an SDL_Surface, as that's what TTF_RenderText_X returns
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	SDLTexture texture(SDL_CreateTextureFromSurface(mRenderer.get(), surf));
	//Clean up unneeded stuff
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);

	return texture;
}

void Window::Clear(){
	SDL_RenderClear(mRenderer.get());
}
void Window::Present(){
	SDL_RenderPresent(mRenderer.get());
}
SDL_Rect Window::Box(){
	//Update mBox to match the current window size
	SDL_GetWindowSize(mWindow.get(), &mBox.w, &mBox.h);
	return mBox;
}

bool Window::sdlRectsIntersect(SDL_Rect rectA, SDL_Rect rectB)
{
	return (basicSdlRectsIntersect(rectA, rectB) || basicSdlRectsIntersect(rectB, rectA));
}

bool Window::basicSdlRectsIntersect(SDL_Rect rectA, SDL_Rect rectB)
{
	bool horiz_overlap, vert_overlap = false;

	horiz_overlap = (rectA.x >= rectB.x && rectA.x <= (rectB.x + rectB.w)) ||
		((rectA.x + rectA.w) >= rectB.x && (rectA.x + rectA.w) <= (rectB.x + rectB.w));

	vert_overlap = (rectA.y >= rectB.y && rectA.y <= (rectB.y + rectB.h)) ||
		((rectA.y + rectA.h) >= rectB.y && (rectA.y + rectA.h) <= (rectB.y + rectB.h));

	return (horiz_overlap && vert_overlap);
}