#include "GraphicalInterface.hpp"
#include "Player.hpp"

GraphicalInterface::GraphicalInterface(GameEngine *game_engine) : _game_engine(game_engine) {
    this->_quit = false;
    this->_mouse_action = false;
    this->_init_sdl();

    this->_font_handler = new FontHandler(this->_renderer, this->_res_ratio);
    this->_analytics = new Analytics(this->_game_engine, this->_font_handler);

    this->_grid_padding = 8;                                   /* defaults to 12 for screen size of 1280, old is 8 : 0.0625 */
    this->_stone_size = (int32_t)(this->_res_h * 0.04375);     /* defaults to 56 for screen size of 1280 */
    this->_pad[1] = (int32_t)(this->_main_viewport.w * (float)(this->_grid_padding / 100.));
    this->_pad[0] = (int32_t)(this->_res_h * (float)(this->_grid_padding / 100.));
    this->_inc[1] = (float)(this->_main_viewport.w - (this->_pad[1] * 2)) / (COLS-1);
    this->_inc[0] = (float)(this->_res_h - (this->_pad[0] * 2)) / (ROWS-1);
    this->_bg_color = (SDL_Color){ 217, 165, 84, 255 };
    this->_load_images();
    this->_init_grid();

    TTF_Font *font = this->_font_handler->load_font("./resources/fonts/Roboto-Regular.ttf", 16);
    this->_button_newgame = new Button(this->_renderer, "new game", {this->_main_viewport.w + 10, 270}, font, {255, 240, 224, 255});
    this->_button_restart = new Button(this->_renderer, "restart", {this->_main_viewport.w + 10, 300}, font, {255, 240, 224, 255});
    this->_button_undo = new Button(this->_renderer, "undo", {this->_main_viewport.w + 10, 330}, font, {255, 240, 224, 255});
}

GraphicalInterface::GraphicalInterface(GraphicalInterface const &src) : _game_engine(src.get_game_engine()) {
    *this = src;
}

GraphicalInterface::~GraphicalInterface(void) {
    delete this->_font_handler;
    this->_close_sdl();
}

GraphicalInterface	&GraphicalInterface::operator=(GraphicalInterface const &src) {
    return (*this);
}

SDL_Texture *GraphicalInterface::load_texture(std::string path) {
    SDL_Texture *texture = NULL;
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());
    texture = SDL_CreateTextureFromSurface(this->_renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

Eigen::Array2i  GraphicalInterface::grid_to_screen(Eigen::Array2i pos) {
    Eigen::Array2i  s_pos;
    s_pos[0] = this->_pad[0] + pos[0] * this->_inc[0];
    s_pos[1] = this->_pad[1] + pos[1] * this->_inc[1];
    return s_pos;
}

Eigen::Array2i  GraphicalInterface::screen_to_grid(Eigen::Array2i pos) {
    Eigen::Array2i  g_pos;
    g_pos[0] = std::round((pos[0] - this->_pad[0]) / this->_inc[0]);
    g_pos[1] = std::round((pos[1] - this->_pad[1]) / this->_inc[1]);
    g_pos[0] = std::min(std::max(g_pos[0], 0), COLS-1);
    g_pos[1] = std::min(std::max(g_pos[1], 0), ROWS-1);
    return g_pos;
}

Eigen::Array2i  GraphicalInterface::snap_to_grid(Eigen::Array2i pos) {
    Eigen::Array2i  s_pos;
    s_pos[0] = this->_pad[0] + std::round((pos[0] - this->_pad[0]) / this->_inc[0]) * this->_inc[0];
    s_pos[1] = this->_pad[1] + std::round((pos[1] - this->_pad[1]) / this->_inc[1]) * this->_inc[1];
    s_pos[0] = std::min(std::max(s_pos[0], this->_pad[0]), this->_main_viewport.h - this->_pad[0]);
    s_pos[1] = std::min(std::max(s_pos[1], this->_pad[1]), this->_main_viewport.w - this->_pad[1]);
    return s_pos;
}

void    GraphicalInterface::_load_images(void) {
    this->_white_tex = this->load_texture(std::string("./resources/circle_white.png"));
    this->_white_stone_tex = this->load_texture(std::string("./resources/circle_white_outlined.png"));
    this->_black_stone_tex = this->load_texture(std::string("./resources/circle_black.png"));
    this->_select_stone_tex = this->load_texture(std::string("./resources/circle_select.png"));
}

void    GraphicalInterface::_init_sdl(void) {
    int32_t secondary_viewport_width = 300;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    IMG_Init(IMG_INIT_PNG);

    /* get the screen dimensions */
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    this->_win_h = (int32_t)(std::min(DM.w, DM.h) / 1.125);
    this->_win_w = this->_win_h + secondary_viewport_width;

    this->_window = SDL_CreateWindow("gomoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->_win_w, this->_win_h, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    this->_renderer = SDL_CreateRenderer(this->_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    /* get the renderer resolution, may differ from window size for high-dpi displays */
    SDL_GetRendererOutputSize(this->_renderer, &this->_res_w, &this->_res_h);
    this->_res_ratio = (this->_res_h / (float)this->_win_h);
    secondary_viewport_width = (int32_t)(secondary_viewport_width * this->_res_ratio);
    this->_global_viewport = (SDL_Rect){ 0, 0, this->_res_w, this->_res_h };
    this->_main_viewport = (SDL_Rect){ 0, 0, (int32_t)(this->_res_w - secondary_viewport_width), this->_res_h };
    this->_secondary_viewport = (SDL_Rect){ (int32_t)(this->_res_w - secondary_viewport_width), 0, secondary_viewport_width, this->_res_h };

    this->_board_grid_tex = SDL_CreateTexture(this->_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, this->_main_viewport.w, this->_main_viewport.h);

    SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(this->_board_grid_tex, SDL_BLENDMODE_BLEND);
    SDL_RenderSetViewport(this->_renderer, &this->_main_viewport);
}

void    GraphicalInterface::_init_grid(void) {
    SDL_SetRenderTarget(this->_renderer, this->_board_grid_tex);
    SDL_RenderClear(this->_renderer);
    SDL_SetRenderDrawColor(this->_renderer, 0, 0, 0, 255);
    for (int i = 0; i < ROWS; i++) {
        SDL_RenderDrawLine(this->_renderer, this->_pad[1], (int)(this->_pad[0]+i*this->_inc[0]),
            this->_main_viewport.w - this->_pad[1], (int)(this->_pad[0]+i*this->_inc[0]));
    }
    for (int i = 0; i < COLS; i++) {
        SDL_RenderDrawLine(this->_renderer, (int)(this->_pad[1]+i*this->_inc[1]), this->_pad[0],
            (int)(this->_pad[0]+i*this->_inc[0]), this->_res_h - this->_pad[0]);
    }
    this->_init_grid_points();
    SDL_Rect    rect = { this->_main_viewport.w - 1, 0, 1, this->_main_viewport.h };
    SDL_RenderFillRect(this->_renderer, &rect);
    SDL_SetRenderTarget(this->_renderer, NULL);
}

void    GraphicalInterface::_init_grid_points(void) {
    Eigen::Array2i  pos;
    SDL_Rect        rect;
    int32_t         pt_size = (int32_t)(0.005625 * this->_main_viewport.w);

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            pos = this->grid_to_screen((Eigen::Array2i){3+j*6, 3+i*6});
            rect = {pos[1]-(pt_size/2), pos[0]-(pt_size/2), pt_size, pt_size};
            SDL_RenderCopy(this->_renderer, this->_black_stone_tex, NULL, &rect);
        }
    }
}

void    GraphicalInterface::update_events(void) {
    while (SDL_PollEvent(&this->_event) != 0) {
        switch (this->_event.type) {
            case SDL_QUIT: this->_quit = true; break;
            case SDL_MOUSEMOTION: SDL_GetMouseState(&this->_mouse_pos[1], &this->_mouse_pos[0]);
                this->_mouse_pos *= this->_res_ratio;
                break;
            case SDL_MOUSEBUTTONUP: this->_mouse_action = true; break;
        }
    }
    this->_key_states = SDL_GetKeyboardState(NULL);
    if (this->_key_states[SDL_SCANCODE_ESCAPE])
        this->_quit = true;
}

void    GraphicalInterface::update_display(void) {
    SDL_RenderSetViewport(this->_renderer, &this->_main_viewport);
    SDL_SetRenderDrawColor(this->_renderer, this->_bg_color.r, this->_bg_color.g, this->_bg_color.b, this->_bg_color.a);
    SDL_RenderClear(this->_renderer);
    SDL_RenderCopyEx(this->_renderer, this->_board_grid_tex, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
    this->_render_stones();
    this->_render_select();
    this->_render_secondary_viewport();
    this->_render_buttons();
    SDL_RenderPresent(this->_renderer);
}

void    GraphicalInterface::_render_stones(void) {
    Eigen::Array2i  s_pos;
    SDL_Rect        rect;
    SDL_Texture     *stone;

    for (int j = 0; j < COLS; j++) {
        for (int i = 0; i < ROWS; i++) {
            if (this->_game_engine->grid(j,i) == -1 || this->_game_engine->grid(j,i) == 1) {
                s_pos = this->grid_to_screen((Eigen::Array2i){j,i});
                rect = {s_pos[1] - this->_stone_size / 2, s_pos[0] - this->_stone_size / 2, this->_stone_size, this->_stone_size};
                stone = (this->_game_engine->grid(j,i) == -1 ? this->_black_stone_tex : this->_white_stone_tex);
                SDL_RenderCopy(this->_renderer, stone, NULL, &rect);
            }
        }
    }
    /* look for the last action and display an indicator on it */
    t_action            last = this->_game_engine->get_history()->back();
    int32_t             size = (int32_t)(this->_stone_size * 0.1);

    s_pos = this->grid_to_screen(last.pos);
    rect = { s_pos[1]-size/2, s_pos[0]-size/2, size, size };
    stone = (this->_game_engine->grid(last.pos[0],last.pos[1]) == 1 ? this->_black_stone_tex : this->_white_tex);
    SDL_RenderCopy(this->_renderer, stone, NULL, &rect);
}

void    GraphicalInterface::_render_select(void) {
    Eigen::Array2i  s_pos;
    Eigen::Array2i  g_pos;
    SDL_Rect        rect;

    s_pos = this->snap_to_grid(this->_mouse_pos);
    g_pos = this->screen_to_grid(this->_mouse_pos);
    rect = {s_pos[1] - this->_stone_size / 2, s_pos[0] - this->_stone_size / 2, this->_stone_size, this->_stone_size};
    if (this->_game_engine->grid(g_pos[0], g_pos[1]) != -1 && this->_game_engine->grid(g_pos[0], g_pos[1]) != 1)
        SDL_RenderCopy(this->_renderer, this->_select_stone_tex, NULL, &rect);
}

void    GraphicalInterface::_render_secondary_viewport(void) {
    SDL_RenderSetViewport(this->_renderer, &this->_secondary_viewport);
    SDL_SetRenderDrawColor(this->_renderer, 238, 214, 178, 255);

    SDL_Rect    rect = {0, 0, this->_secondary_viewport.w, this->_secondary_viewport.h};
    SDL_RenderFillRect(this->_renderer, &rect);
    /* TODO : put that on a texture instead of drawing the lines each time */
    SDL_SetRenderDrawColor(this->_renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(this->_renderer, 0,  90, this->_secondary_viewport.w,  90);
    SDL_RenderDrawLine(this->_renderer, 0, 180, this->_secondary_viewport.w, 180);

    this->_analytics->render_text();
}

void    GraphicalInterface::_render_buttons(void) {
    SDL_RenderSetViewport(this->_renderer, &this->_global_viewport);

    // for (std::list<Button*>::iterator it = this->_buttons.begin(); it != this->_buttons.end(); it++) {
    //     (*it)->update_state(&this->_mouse_pos, this->_mouse_action);
    //     (*it)->render(this->_renderer);
    // }
    this->_button_newgame->update_state(&this->_mouse_pos, this->_mouse_action);
    this->_button_newgame->render(this->_renderer);
    this->_button_restart->update_state(&this->_mouse_pos, this->_mouse_action);
    this->_button_restart->render(this->_renderer);
    this->_button_undo->update_state(&this->_mouse_pos, this->_mouse_action);
    this->_button_undo->render(this->_renderer);
    SDL_RenderSetViewport(this->_renderer, &this->_main_viewport);
}


void    GraphicalInterface::render_choice_menu(void) {
    // Button *button_start = new Button(this->_renderer, "restart", {10, 270}, this->_font_handler->default_font, {255, 255, 255, 255});
    SDL_RenderSetViewport(this->_renderer, &this->_global_viewport);
    SDL_SetRenderDrawColor(this->_renderer, 45, 42, 35, 255);
    SDL_RenderClear(this->_renderer);

    SDL_RenderPresent(this->_renderer);
}

bool    GraphicalInterface::check_mouse_action(void) {
    if (this->_mouse_action == true) {
        this->_mouse_action = false;
        return true;
    }
    return false;
}

bool    GraphicalInterface::check_mouse_on_board(void) {
    /*  check if the mouse is on the main viewport
    */
    if (this->_main_viewport.x < this->_mouse_pos[0] && this->_mouse_pos[0] < this->_main_viewport.w &&
        this->_main_viewport.y < this->_mouse_pos[1] && this->_mouse_pos[1] < this->_main_viewport.h)
        return true;
    return false;
}

bool    GraphicalInterface::check_close(void) {
    return this->_quit;
}

void    GraphicalInterface::_close_sdl(void) {
    SDL_DestroyTexture(this->_white_tex);
    SDL_DestroyTexture(this->_white_stone_tex);
    SDL_DestroyTexture(this->_black_stone_tex);
    SDL_DestroyTexture(this->_select_stone_tex);
    SDL_DestroyTexture(this->_board_grid_tex);
    this->_white_tex = NULL;
    this->_white_stone_tex = NULL;
    this->_black_stone_tex = NULL;
    this->_select_stone_tex = NULL;
    this->_board_grid_tex = NULL;

    SDL_DestroyWindow(this->_window);
    SDL_DestroyRenderer(this->_renderer);
    this->_window = NULL;
    this->_renderer = NULL;

    IMG_Quit();
    SDL_Quit();
}
