#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_mouse.h"
#include <SDL.h>

int main(int argc, char* argv[]) {
    // 初始化SDL视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("无法初始化SDL: %s", SDL_GetError());
        return 1;
    }

        // 设置缩放质量提示（必须在创建窗口前设置）
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // 1=线性过滤 0=最近邻

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow(
        "demo player",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        SDL_Log("无法创建窗口: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);

    // 设置初始背景
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);

    int running = 1;
    SDL_Event event;
    volatile bool is_dragging = false;
    int offset_x = 0, offset_y = 0;  // 鼠标在窗口内的偏移量
    Uint32 last_draw_time = SDL_GetTicks();
    Uint32 current_time = last_draw_time;

    /*
     * 事件处理
     * 1. 窗口大小调整，重绘画，保持宽高比
     * 2. 处理鼠标点击，左键键点击拖拽移动，右键点击暂停恢复
     * 3. 空格暂停恢复
     * 4. 上下调节音量
     * 5. 左右快进倒退五秒
     * 6. 滚轮上下调整音量
     */
    while (running) {
        if(SDL_WaitEventTimeout(&event, 10)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

               case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        // 用户拖拽调整大小
                        case SDL_WINDOWEVENT_RESIZED:
                            printf("窗口调整大小");
                            break;

                        // 程序触发的大小变化
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            printf("尺寸变化");
                            break;

                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            // stop painting
                            break;
                    }
                    break;
                
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = 0;
                            break;
                        case SDLK_KP_SPACE:
                            break;
                        case SDLK_UP:
                            break;
                        case SDLK_DOWN:
                            break;
                        case SDLK_LEFT:
                            break;
                        case SDLK_RIGHT:
                            break;
                    };
                    break;
                
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        // pause

                    }
                    break;
                
                // 处理所有可能的释放事件
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        // pause

                    }
                    break;

                case SDL_MOUSEMOTION:
                    break;

                // 处理鼠标滚轮事件
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) {
                        printf("滚轮向上滚动\n");
                    } else if (event.wheel.y < 0) {
                        printf("滚轮向下滚动\n");
                    }
                    break;

            }
        }


    }

    // 清理资源
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

