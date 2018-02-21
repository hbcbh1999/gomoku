#ifndef GAMEENGINE_HPP
# define GAMEENGINE_HPP

# include <iostream>
# include <chrono>
# include <list>
# include <Eigen/Dense>

# define BOARD_COLS 19
# define BOARD_ROWS 19
# define ALIGNTOWIN 5

class Player;

typedef struct  s_action {
    Eigen::Array2i                              pos;
    Player                                      *player;
    uint64_t                                    id;
    std::chrono::duration<double, std::milli>   timepoint;  /* time since start */
    std::chrono::duration<double, std::milli>   duration;   /* duration of the action */
    Eigen::ArrayXXi                             old_grid;   /* the state of the grid before the action */
}               t_action;


class GameEngine {

public:
    GameEngine(void);
    GameEngine(GameEngine const &src);
    ~GameEngine(void);
    GameEngine	&operator=(GameEngine const &rhs);

    bool                check_action(t_action &action); // check if action is valid
    bool                check_end(uint8_t player_pairs);
    void                update_game_state(t_action &action, Player *player); // update the game state given an action
    void                delete_last_action(void);

    Eigen::Array22i     get_end_line(void);

    /* Getters */
    std::list<t_action>                     *get_history(void) { return &_history; };
    std::list<t_action>                     get_history_copy(void) const { return _history; };
    uint64_t                                get_history_size(void) const { return _history.size(); };
    std::chrono::steady_clock::time_point   get_initial_timepoint(void) const { return _initial_timepoint; };
    /* Setters */
    void                                    inc_game_turn(void);

    Eigen::ArrayXXi                         grid;

private:
    std::list<t_action>                     _history;
    std::chrono::steady_clock::time_point   _initial_timepoint;
    uint64_t                                _game_turn;

    /* the possible states of the board cells */
    struct state {
        enum {
            black_free = -10,
            black = -1,
            free = 0,
            white = 1,
            white_free = 10,
            forbidden = 20
        };
    };

    /* Utils */
    bool            _check_boundary(int row, int col);

    /* Update game state utils */
    uint8_t         _pair_detection(Eigen::Array2i pos);
    int             _check_pair(Eigen::Array2i pos, int max, int row_dir, int col_dir);
    void            _double_threes_detection(void);
    // int             _sum_free_threes(int row, int col, int max, int row_dir, int col_dir);
    bool            _count_double_threes(Eigen::Array4i);
    // bool            _detect_threes(int row, int col, int row_dir, int col_dir, int p);
    bool            _detect_threes_xcoxox(int row, int col, int row_dir, int col_dir, int p);
    bool            _detect_threes_xcxoox(int row, int col, int row_dir, int col_dir, int p);
    bool            _detect_threes_xocxox(int row, int col, int row_dir, int col_dir, int p);
    bool            _detect_threes_xcoox(int row, int col, int row_dir, int col_dir, int p);
    bool            _detect_threes_xocox(int row, int col, int row_dir, int col_dir, int p);

    /* Check end utils */
    bool            _check_col(size_t col, size_t row);
    bool            _check_row(size_t col, size_t row);
    bool            _check_dil(size_t col, size_t row);
    bool            _check_dir(size_t col, size_t row);
    bool            _check_pairs_captured(uint8_t pairs);

};

#endif
