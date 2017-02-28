// guiFunctions.h

#include <vector>
#include "signs.hpp"

/* WINDOW settings */

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH  1280

/* Nuklear defines */
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR

#if defined(WIN32)
	#define _CRT_SECURE_NO_WARNINGS

	#define DX_WINTITSIZE 128
	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>
	#include <commdlg.h>

	#define NK_IMPLEMENTATION
	#include "nuklear.h"

	#define NK_GDIP_IMPLEMENTATION
	#include "nuklear_gdip.h"

#elif defined(UNIX)
	#include <GL/glew.h>

	#include <SDL2/SDL.h>
	#include <SDL2/SDL_opengl.h>

	#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
	#define NK_INCLUDE_FONT_BAKING
	#define NK_INCLUDE_DEFAULT_FONT

	#define NK_IMPLEMENTATION
	#include "nuklear.h"

	#define NK_SDL_GL2_IMPLEMENTATION
	#include "nuklear_sdl_gl2.h"

	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
#endif



#if defined(WIN32)
	static LRESULT CALLBACK
	WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg) {
 			case WM_DESTROY:
	 			PostQuitMessage(0);
 				return 0;
		}
		if (nk_gdip_handle_event(wnd, msg, wparam, lparam)) {
			return 0;
		}
		return DefWindowProcW(wnd, msg, wparam, lparam);
	}
#endif



struct Platform
{
	#if defined(WIN32)
		GdipFont* font;
		HWND wnd;
		wchar_t winTitle[DX_WINTITSIZE];
		int needs_refresh;
	#elif defined(UNIX)
		SDL_Window *win;
		SDL_GLContext glContext;
		int win_width;
		int win_height;
	#endif
};



void windowInitialization(struct Platform* wi, const char* title, int width, int height)
{
	#if defined(WIN32)
		WNDCLASSW wc;
		RECT rect = { 0, 0, width, height };
		DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
		DWORD exstyle = WS_EX_APPWINDOW;
		MultiByteToWideChar(CP_ACP, 0, title, strlen(title)+1, wi->winTitle, DX_WINTITSIZE);

		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = GetModuleHandleW(0);
		wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(101));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = wi->winTitle;
		RegisterClassW(&wc);
        
		AdjustWindowRectEx(&rect, style, FALSE, exstyle);

		wi->wnd = CreateWindowExW(exstyle, wc.lpszClassName, wi->winTitle,
			style | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			NULL, NULL, wc.hInstance, NULL);

		wi->needs_refresh = 1;
		
	#elif defined(UNIX)
		SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // DX_OPENGL_VERSION
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2); // DX_OPENGL_VERSION
    
		wi->win = SDL_CreateWindow(title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
			wi->glContext = SDL_GL_CreateContext(wi->win);
		SDL_GetWindowSize(wi->win, &(wi->win_width), &(wi->win_height));

		glViewport(0, 0, width, height);
		glewExperimental = 1;
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to setup GLEW\n");
			exit(1);
		}
	#endif
}



void loadDefaultFont(struct Platform* wi)
{
	#if defined(WIN32)
		wi->font = nk_gdipfont_create("Arial", 12);
		nk_gdip_set_font(wi->font);
	#elif defined(UNIX)
		struct nk_font_atlas *atlas;
		nk_sdl_font_stash_begin(&atlas);
		nk_sdl_font_stash_end();
	#endif
}



struct nk_context* ctxInitialization(Platform* wi, int width, int height)
{
	#if defined(WIN32)
		return nk_gdip_init(wi->wnd, width, height);
	#elif defined(UNIX)
		return nk_sdl_init(wi->win);
	#endif
}



void render(Platform* wi)
{
	#if defined(WIN32)
		nk_gdip_render(NK_ANTI_ALIASING_ON, nk_rgb(30,30,30));
	#elif defined(UNIX)
		float bg[4];
		nk_color_fv(bg, nk_rgb(30,30,30));
		SDL_GetWindowSize(wi->win, &(wi->win_width), &(wi->win_height) );
		glViewport(0, 0, wi->win_width, wi->win_height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(bg[0], bg[1], bg[2], bg[3]);
		nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
		SDL_GL_SwapWindow(wi->win);
	#endif
}



void shutdown(Platform* wi)
{
	#if defined(WIN32)
		nk_gdipfont_del(wi->font);
		nk_gdip_shutdown();
		UnregisterClassW( wi->winTitle, GetModuleHandleW(0) );
	#elif defined(UNIX)
		nk_sdl_shutdown();
		SDL_GL_DeleteContext(wi->glContext);
		SDL_DestroyWindow(wi->win);
		SDL_Quit();
	#endif
}



static struct nk_image loadImageFromFile(const char *filename)
{
	#if defined(WIN32)
		wchar_t* wFilename = new wchar_t[strlen(filename)];
		mbstowcs(wFilename, filename, strlen(filename));

		struct nk_image returnImage = nk_gdip_load_image_from_file(wFilename);
		delete[] wFilename;
		return returnImage;
	#elif defined(UNIX)
		int x, y, n;
		GLuint tex;

		unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
		if (!data) {
			fprintf(stdout, "[SDL]: failed to load image\n");
			exit(1);
		}

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
		return nk_image_id((int)tex);
	#endif
}



void drawLine(unsigned char* data, int left, int top, int right, int bottom, int red, int green, int blue, int alpha = 255)
{
	int row, col, depth = 4;
	right = right < 640 ? right : 640;
	left = left > 0 ? left : 0;
	for (row = top; row < bottom; row++) {
		for (col = left; col < right; col++) {            
			data[(row * 640 + col) * depth + 0] = red;
			data[(row * 640 + col) * depth + 1] = green;
			data[(row * 640 + col) * depth + 2] = blue;
			data[(row * 640 + col) * depth + 3] = alpha;
		}
	}
}



void drawLineTransparent(unsigned char* data, int left, int top, int right, int bottom, int red, int green, int blue, int alpha = 255)
{
	int row, col, depth = 4;
	for (row = top; row < bottom; row++) {
		for (col = left; col < right; col++) {            
			data[(row * 640 + col) * depth + 0] += red;
			data[(row * 640 + col) * depth + 1] += green;
			data[(row * 640 + col) * depth + 2] += blue;
			data[(row * 640 + col) * depth + 3] = alpha;
		}
	}
}



void streamImageFunc(unsigned char* data, int x, int y, line_data& myline, std::vector<sign_data>& mysigns)
{
	if (myline.on_line) {
		drawLineTransparent(data, 0, 450, x, y, 0, 0, 0, 130);            
		drawLine(data, 0, y - 2, myline.robot_center - 5, y, 255, 188, 0);
		drawLine(data, myline.robot_center + 5, y - 2, x, y, 255, 188, 0);

		drawLine(data, myline.robot_center - 5, 450, myline.robot_center + 5, y,     0, 0, 0, 0);
		drawLine(data, myline.robot_center + 3, 450, myline.robot_center + 5, y,     255, 188, 0);
		drawLine(data, myline.robot_center - 5, 450, myline.robot_center - 3, y,     255, 188, 0);
		drawLine(data, myline.center_of_line - 3, 450, myline.center_of_line + 3, y, 255, 188, 0);
	}
	else {
		drawLine(data, 0, 450, x, y, 255, 0, 0, 130);
	}

	int left = 325, top = 200, right = 640, bottom = 350;
	drawLine(data, left, top, left + 10, top + 2,       255, 188, 0);
	drawLine(data, right - 10, top, right, top + 2,     255, 188, 0);
	drawLine(data, left, bottom - 2, left + 10, bottom, 255, 188, 0);
	drawLine(data, right - 10, bottom - 2, right, bottom, 255, 188, 0);
	drawLine(data, left, top, left + 2, top + 10,     255, 188, 0);
	drawLine(data, left, bottom - 10, left + 2, bottom,     255, 188, 0);
	drawLine(data, right - 2, top, right, top + 10,   255, 188, 0);
	drawLine(data, right - 2, bottom - 10, right, bottom,   255, 188, 0);

	for (unsigned i = 0; i < mysigns.size(); ++i) {
		left = 320 + mysigns[i].area.x;
		top = 200 + mysigns[i].area.y;
		right = left + mysigns[i].area.w;
		bottom = top + mysigns[i].area.h;

		drawLine(data, left, top, right, top + 2, 0, 255, 255);
		drawLine(data, left, bottom - 2, right, bottom, 0, 255, 255);
		drawLine(data, left, top, left + 2, bottom, 0, 255, 255);
		drawLine(data, right - 2, top, right, bottom, 0, 255, 255);
	}
}



struct nk_image loadImageFromMemory(const unsigned char* buf, int bufSize, line_data& myline, std::vector<sign_data>& mysigns, void (*drawingFunction)(unsigned char*, int, int, line_data&, std::vector<sign_data>&) = NULL)
{
	#if defined(WIN32)
		return nk_gdip_load_image_from_memory(buf, bufSize);
	#elif defined(UNIX)
		int x, y, n;
		GLuint tex;

		unsigned char *data = stbi_load_from_memory(buf, bufSize, &x, &y, &n, 4);
		if (!data) {
			fprintf(stdout, "[SDL]: failed to load image\n");
			exit(1);
		}

		if (drawingFunction) {
			drawingFunction(data, x, y, myline, mysigns);
		}

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return nk_image_id((int)tex);
	#endif
}
