#if __has_include("SDL3/SDL.h")
#include "D_guipanel.h"

#include "SDL3/SDL.h"
namespace fasm::gui
{
    class window
    {
    public:
        explicit window(SDL_Window *native_window) : native(native_window) {}
        SDL_Window *native{};
    };

    class renderer
    {
    public:
        explicit renderer(SDL_Renderer *native_renderer) : native(native_renderer) {}
        SDL_Renderer *native{};
    };

    static SDL_Window *to_native(window *w)
    {
        return w ? w->native : nullptr;
    }

    static SDL_Renderer *to_native(renderer *r)
    {
        return r ? r->native : nullptr;
    }

    window *createWindow()
    {
        SDL_Window *native = SDL_CreateWindow("fasm::gui", 640, 480, SDL_WINDOW_RESIZABLE);
        if (!native)
            return nullptr;
        return new window(native);
    }

    int deleteWindow(window *w)
    {
        if (!w)
            return -1;
        if (w->native)
            SDL_DestroyWindow(w->native);
        delete w;
        return 0;
    }

    int setWindowSize(window *w, int32_t x, int32_t y)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_SetWindowSize(native, x, y);
        return 0;
    }

    int getWindowSize(window *w, int32_t *x, int32_t *y)
    {
        SDL_Window *native = to_native(w);
        if (!native || !x || !y)
            return -1;
        SDL_GetWindowSize(native, x, y);
        return 0;
    }

    int setWindowPosition(window *w, int32_t x, int32_t y)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_SetWindowPosition(native, x, y);
        return 0;
    }

    int getWindowPosition(window *w, int32_t *x, int32_t *y)
    {
        SDL_Window *native = to_native(w);
        if (!native || !x || !y)
            return -1;
        SDL_GetWindowPosition(native, x, y);
        return 0;
    }

    int setWindowTitle(window *w, const char *title)
    {
        SDL_Window *native = to_native(w);
        if (!native || !title)
            return -1;
        SDL_SetWindowTitle(native, title);
        return 0;
    }

    const char *getWindowTitle(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return nullptr;
        return SDL_GetWindowTitle(native);
    }

    int showWindow(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_ShowWindow(native);
        return 0;
    }

    int hideWindow(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_HideWindow(native);
        return 0;
    }

    int maximizeWindow(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_MaximizeWindow(native);
        return 0;
    }

    int minimizeWindow(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_MinimizeWindow(native);
        return 0;
    }

    int restoreWindow(window *w)
    {
        SDL_Window *native = to_native(w);
        if (!native)
            return -1;
        SDL_RestoreWindow(native);
        return 0;
    }

    int pollEvents()
    {
        SDL_Event ev{};
        int should_close = 0;
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_EVENT_QUIT || ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                should_close = 1;
        }
        return should_close;
    }

    renderer *createRenderer(window *w)
    {
        SDL_Window *native_window = to_native(w);
        if (!native_window)
            return nullptr;

        SDL_Renderer *native_renderer = SDL_CreateRenderer(native_window, nullptr);
        if (!native_renderer)
            return nullptr;

        return new renderer(native_renderer);
    }

    int deleteRenderer(renderer *r)
    {
        if (!r)
            return -1;
        if (r->native)
            SDL_DestroyRenderer(r->native);
        delete r;
        return 0;
    }

    int setDrawColor(renderer *r, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;
        SDL_SetRenderDrawColor(native, red, green, blue, alpha);
        return 0;
    }

    int clear(renderer *r)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;
        SDL_RenderClear(native);
        return 0;
    }

    int present(renderer *r)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;
        SDL_RenderPresent(native);
        return 0;
    }

    int drawPoint(renderer *r, int32_t x, int32_t y)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;
        SDL_RenderPoint(native, static_cast<float>(x), static_cast<float>(y));
        return 0;
    }

    int drawLine(renderer *r, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;
        SDL_RenderLine(native,
                       static_cast<float>(x1),
                       static_cast<float>(y1),
                       static_cast<float>(x2),
                       static_cast<float>(y2));
        return 0;
    }

    int drawRect(renderer *r, int32_t x, int32_t y, int32_t w, int32_t h)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;

        SDL_FRect rect{};
        rect.x = static_cast<float>(x);
        rect.y = static_cast<float>(y);
        rect.w = static_cast<float>(w);
        rect.h = static_cast<float>(h);

        SDL_RenderRect(native, &rect);
        return 0;
    }

    int fillRect(renderer *r, int32_t x, int32_t y, int32_t w, int32_t h)
    {
        SDL_Renderer *native = to_native(r);
        if (!native)
            return -1;

        SDL_FRect rect{};
        rect.x = static_cast<float>(x);
        rect.y = static_cast<float>(y);
        rect.w = static_cast<float>(w);
        rect.h = static_cast<float>(h);

        SDL_RenderFillRect(native, &rect);
        return 0;
    }
}
#endif
