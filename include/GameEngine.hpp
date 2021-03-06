#ifndef GAMEENGINE_HPP
# define GAMEENGINE_HPP

# include <iostream>
# include <chrono>
# include <list>
# include <iterator>
# include <Eigen/Dense>
# include "BitBoard.hpp"

# define BOARD_COLS 19
# define BOARD_ROWS 19

class Player;

typedef struct  s_action {
    Eigen::Array2i                              pos;        /* action pos on grid */
    uint8_t                                     pid;        /* player id */
    uint8_t                                     ppc;        /* player pairs captured */
    uint32_t                                    id;         /* action id number */
    std::chrono::duration<double, std::milli>   timepoint;  /* time since start */
    std::chrono::duration<double, std::milli>   duration;   /* duration of the action */
    BitBoard                                    p1_last;    /* the bitboard for player 1 */
    BitBoard                                    p2_last;    /* the bitboard for player 2 */
}               t_action;

class GameEngine {

public:
    GameEngine(void);
    GameEngine(GameEngine const &src);
    ~GameEngine(void);
    GameEngine	&operator=(GameEngine const &rhs);

    bool                check_action(t_action const &action, Player const &p1, Player const &p2);
    void                update_game_state(t_action &action, Player *p1, Player *p2);
    void                update_grid(Player const &p1, Player const &p2);
    void                update_grid_with_bitboard(BitBoard const &bitboard, int8_t const &state);
    void                update_dynamic_pattern_weights(void);
    void                delete_last_action(Player *p1, Player *p2);

    Eigen::Array22i     get_end_line(BitBoard const &bitboard);

    /* Getters */
    std::list<t_action>                     *get_history(void) { return &_history; };
    std::list<t_action>                     get_history_copy(void) const { return _history; };
    uint32_t                                get_history_size(void) const { return _history.size(); };
    std::chrono::steady_clock::time_point   get_initial_timepoint(void) const { return _initial_timepoint; };

    Eigen::ArrayXXi                         grid;

private:
    std::list<t_action>                     _history;
    std::chrono::steady_clock::time_point   _initial_timepoint;

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
};

namespace end {
    enum end {
        none = 0,
        player_win = 1,
        opponent_win = 2,
        draw = 3
    };
};

uint8_t check_end(BitBoard const& p1, BitBoard const& p2, uint8_t const& p1_pairs_captured, uint8_t const& p2_pairs_captured, uint16_t const& p);

#endif
