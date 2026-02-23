#pragma once
#if __has_include("SDL3/SDL.h")
#include <cstdint>
namespace fasm::gui
{
    class window;
    class renderer;
    window *createWindow();
    int deleteWindow(window *w);
    int setWindowSize(window *w, int32_t x, int32_t y);
    int getWindowSize(window *w, int32_t *x, int32_t *y);

    int setWindowPosition(window *w, int32_t x, int32_t y);
    int getWindowPosition(window *w, int32_t *x, int32_t *y);

    int setWindowTitle(window *w, const char *title);
    const char *getWindowTitle(window *w);

    int showWindow(window *w);
    int hideWindow(window *w);
    int maximizeWindow(window *w);
    int minimizeWindow(window *w);
    int restoreWindow(window *w);
    int pollEvents();

    renderer *createRenderer(window *w);
    int deleteRenderer(renderer *r);

    int setDrawColor(renderer *r, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
    int clear(renderer *r);
    int present(renderer *r);

    int drawPoint(renderer *r, int32_t x, int32_t y);
    int drawLine(renderer *r, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
    int drawRect(renderer *r, int32_t x, int32_t y, int32_t w, int32_t h);
    int fillRect(renderer *r, int32_t x, int32_t y, int32_t w, int32_t h);
}
#endif
