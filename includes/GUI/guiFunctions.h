// guiFunctions.h

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH  1280

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define NK_INCLUDE_FIXED_TYPES
    #define NK_INCLUDE_STANDARD_IO
    #define NK_INCLUDE_STANDARD_VARARGS
    #define NK_INCLUDE_DEFAULT_ALLOCATOR

#if defined(WINDOWS)
    #define NK_GDIP_IMPLEMENTATION

    #include <windows.h>
    #include <commdlg.h>

    #define NK_IMPLEMENTATION
    #include "nuklear.h"
    #include "nuklear_gdip.h"

    #define DX_WINTITSIZE 128
#else
    
    #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    #define NK_INCLUDE_FONT_BAKING
    #define NK_INCLUDE_DEFAULT_FONT

    #include <GL/glew.h>

    #define NK_SDL_GL2_IMPLEMENTATION
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
 

    #define NK_IMPLEMENTATION
    #include "nuklear.h"
    #include "nuklear_sdl_gl2.h"

    #define STB_IMAGE_IMPLEMENTATION
    #include "stb_image.h"
#endif



struct Platform
{
    #if defined(WINDOWS)
        GdipFont* font;
        HWND wnd;
        wchar_t winTitle[DX_WINTITSIZE];
        int needs_refresh;
    #else
        SDL_Window *win;
        SDL_GLContext glContext;
        int win_width;
        int win_height;
    #endif
};



void windowInitialization(struct Platform* wi, const char* title, int width, int height)
{
#if defined(WINDOWS)
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

#else
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
#if defined(WINDOWS)
    wi->font = nk_gdipfont_create("Arial", 12);
    nk_gdip_set_font(wi->font);
#else
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();
#endif
}



struct nk_context* ctxInitialization(Platform* wi, int width, int height){
#if defined(WINDOWS)
    return nk_gdip_init(wi->wnd, width, height);
#else
    return nk_sdl_init(wi->win);
#endif
}



void render(Platform* wi)
{
    #if defined(WIN32)
        nk_gdip_render(NK_ANTI_ALIASING_ON, nk_rgb(30,30,30));
    #else
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
    #if defined(WINDOWS)
        nk_gdipfont_del(wi->font);
        nk_gdip_shutdown();
        UnregisterClassW( wi->winTitle, GetModuleHandleW(0) );
    #else
        nk_sdl_shutdown();
        SDL_GL_DeleteContext(wi->glContext);
        SDL_DestroyWindow(wi->win);
        SDL_Quit();
    #endif
}



static struct nk_image loadImageFromFile(const char *filename)
{
    #if defined(WINDOWS)
        return nk_gdip_load_image_from_file(filename);
    #else
        int x, y, n;
        GLuint tex;

        unsigned char *data = stbi_load(filename, &x, &y, &n, 4);
        if (!data) 
        {
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



static struct nk_image loadImageFromMemory(const unsigned char* buf, int bufSize)
{
    #if defined(WINDOWS)
        return nk_gdip_load_image_from_memory(buf, bufSize);
    #else
        int x, y, n;
        GLuint tex;
        
        unsigned char *data = stbi_load_from_memory(buf, bufSize, &x, &y, &n, 4);
        if (!data) 
        {
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