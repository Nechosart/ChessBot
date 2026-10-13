#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
using namespace std;

const int window_size_x = 800;
const int window_size_y = 800;
const float window_size_x_float = static_cast<float>(window_size_x);
const float window_size_y_float = static_cast<float>(window_size_y);
const float window_part_x = window_size_x_float / 8;
const float window_part_y = window_size_y_float / 8;


sf::RenderWindow window(sf::VideoMode(window_size_x + 50, window_size_y), "Chess");
sf::CircleShape Place_piece_sprite(window_part_x * 0.2);


const int available_squares_max_count = 27;
const int pieces_type_count = 6;
int available_squares_count = 0;
const int available_squares_data_count = 2;
int available_squares[available_squares_max_count][available_squares_data_count];
bool is_checked_king = false;
bool is_checkmate = false;
int king_positions[2];
bool is_move_white = true;
int current_color_number = 0;
int current_color_pieces_id_multiplier = 0;
int game_moves_count = 0;
bool can_castle[2][2];
int material_imbalance = 0;
const int last_moved_piece_number = 2;
int last_moved_piece[last_moved_piece_number];
const int PiecesPrice[6] = { 1,3,3,5,9,1 };


const int board_size_x = 8;
const int board_size = board_size_x * board_size_x;
int Board_pieces[board_size];

bool check_is_square_empty(int x, int y) {
    return Board_pieces[x + y * board_size_x] == -1;
}

bool check_is_square_opponent(int x, int y) {
    int piece = Board_pieces[x + y * board_size_x];
    return piece != -1 && piece / pieces_type_count != current_color_number;
}

bool check_is_square_opponent_id(int id) {
    int piece = Board_pieces[id];
    return piece != -1 && piece / pieces_type_count != current_color_number;
}

int get_piece_type(int x, int y) {
    return Board_pieces[x + y * board_size_x] % pieces_type_count;
}

int get_piece_type_id(int id) {
    return Board_pieces[id] % pieces_type_count;
}

bool check_is_square_mine(int id) {
    return Board_pieces[id] != -1 && (Board_pieces[id] / pieces_type_count == current_color_number);
}

bool check_is_square_not_mine(int x, int y) {
    int piece = Board_pieces[x + y * board_size_x];
    return piece == -1 || piece / pieces_type_count != current_color_number;
}

bool check_is_possible(int x, int y) {
    return x >= 0 && x < board_size_x && y >= 0 && y < board_size_x;
}

void move_king(int pos) {
    Board_pieces[king_positions[current_color_number]] = -1;
    Board_pieces[pos] = 5 + current_color_pieces_id_multiplier;
    king_positions[current_color_number] = pos;
}



int RookXY[4][2] = { {0,1},{1,0},{0,-1},{-1,0} };
int BishopXY[4][2] = { {-1,-1},{1,-1},{1,1},{-1,1} };
int QueenXY[8][2] = { {0,1},{1,0},{0,-1},{-1,0} , {1,1},{-1,-1},{1,-1},{-1,1} };
int KnightXY[8][2] = { {1,2},{2,1},{2,-1},{1,-2} , {-1,-2},{-2,-1},{-2,1},{-1,2} };

int selected_piece = -1;
int pawn_steps_count;


bool check_cant_be_captured(int square) {
    int piece_type;
    int x0 = square % board_size_x;
    int y0 = square / board_size_x;

    int x, y, dx, dy;

    for (int i = 0; i < 8; i++) {
        dx = QueenXY[i][0];
        dy = QueenXY[i][1];
        x = x0 + dx;
        y = y0 + dy;

        bool is_neighbor = true;

        while (check_is_possible(x, y)) {
            if (check_is_square_empty(x, y) == false) {
                if (check_is_square_opponent(x, y)) {
                    piece_type = get_piece_type(x, y);
                    // i = 0-3 -- Rook
                    // i = 4-7 -- Bishop
                    if (piece_type == 4 || (piece_type == 1 && i >= 4) || (piece_type == 3 && i <= 3) || (piece_type == 5 && is_neighbor)) {
                        return false;
                    }
                }
                break;
            }
            x += dx;
            y += dy;
            is_neighbor = false;
        }
    }

    for (int i = 0; i < 8; i++) {
        x = x0 + KnightXY[i][0];
        y = y0 + KnightXY[i][1];

        if (check_is_possible(x, y) && check_is_square_opponent(x, y) && get_piece_type(x, y) == 2) {
            return false;
        }
    }

    for (int i = -1; i < 3; i += 2) {
        x = x0 + i;
        y = y0 - 1 + current_color_number * 2;

        if (check_is_possible(x, y) && check_is_square_opponent(x, y) && get_piece_type(x, y) == 0) {
            return false;
        }
    }

    return true;
}


int last_move_square0;
int last_move_square;
int last_move_piece_type;
int last_move_piece_captured_type;
int last_move_special_move;

void move_piece_only_board(int square, int square0, int special_move, bool is_material_update) {
    last_move_square0 = square0;
    last_move_square = square;
    last_move_special_move = special_move;

    last_move_piece_type = Board_pieces[last_move_square0] % pieces_type_count;
    last_move_piece_captured_type = Board_pieces[last_move_square] % pieces_type_count;

    int current_color_multiplier = 1 - current_color_number * 2;

    // Check if its en passant
    if (last_move_special_move == 1) {
        last_move_piece_captured_type = 0; // Because pawn type_id = 0
        Board_pieces[last_move_square + current_color_multiplier * board_size_x] = -1;
    }

    // If king then change king position
    if (last_move_piece_type == 5) {
        king_positions[current_color_number] = last_move_square;
    }

    // Check if its promotion
    if (last_move_special_move == 3) {
        last_move_piece_type = 4;
    }

    Board_pieces[last_move_square0] = -1;
    Board_pieces[last_move_square] = last_move_piece_type + current_color_pieces_id_multiplier;

    // Check if its castling
    if (last_move_special_move == 2) {
        if (last_move_square > last_move_square0) {
            Board_pieces[last_move_square0 + 1] = 3 + current_color_pieces_id_multiplier;
            Board_pieces[(last_move_square0 / board_size_x) * board_size_x + 7] = -1;
        }
        else {
            Board_pieces[last_move_square0 - 1] = 3 + current_color_pieces_id_multiplier;
            Board_pieces[(last_move_square0 / board_size_x) * board_size_x + 0] = -1;
        }
    }

    if (is_material_update) {

        // Change material imbalance
        if (last_move_piece_captured_type != -1) {
            material_imbalance += PiecesPrice[last_move_piece_captured_type] * current_color_multiplier;
        }

        // Add material imbalance if pawn becomes queen
        if (last_move_special_move == 3)
        {
            // 8 because Queen(9) - pawn(1) = 8
            material_imbalance += 8 * current_color_multiplier;
        }
    }
}

void move_piece_only_board_undo(bool is_material_update) {
    int current_color_multiplier = 1 - current_color_number * 2;
    int opposite_color_pieces_id_multiplier = (1 - current_color_number) * pieces_type_count;

    // Check if its en passant
    if (last_move_special_move == 1) {
        Board_pieces[last_move_square + current_color_multiplier * board_size_x] = last_move_piece_captured_type + opposite_color_pieces_id_multiplier;
    }
    else if (last_move_piece_captured_type == -1) {
        Board_pieces[last_move_square] = last_move_piece_captured_type;
    }
    else {
        Board_pieces[last_move_square] = last_move_piece_captured_type + opposite_color_pieces_id_multiplier;
    }

    // If king then change king position
    if (last_move_piece_type == 5) {
        king_positions[current_color_number] = last_move_square0;
    }

    if (last_move_special_move == 3) {
        // Check if it was promotion
        Board_pieces[last_move_square0] = 0 + current_color_pieces_id_multiplier;
    }
    else {
        Board_pieces[last_move_square0] = last_move_piece_type + current_color_pieces_id_multiplier;
    }

    // Check if it was castling
    if (last_move_special_move == 2) {
        if (last_move_square > last_move_square0) {
            Board_pieces[last_move_square0 + 1] = -1;
            Board_pieces[(last_move_square0 / board_size_x) * board_size_x + 7] = 3 + current_color_pieces_id_multiplier;
        }
        else {
            Board_pieces[last_move_square0 - 1] = -1;
            Board_pieces[(last_move_square0 / board_size_x) * board_size_x + 0] = 3 + current_color_pieces_id_multiplier;
        }
    }

    if (is_material_update) {

        // Change material imbalance
        if (last_move_piece_captured_type != -1) {
            material_imbalance -= PiecesPrice[last_move_piece_captured_type] * current_color_multiplier;
        }

        // Add material imbalance if pawn becomes queen
        if (last_move_special_move == 3)
        {
            // 8 because Queen(9) - pawn(1) = 8
            material_imbalance -= 8 * current_color_multiplier;
        }
    }
}


bool check_king_wont_be_captured(int x, int y, int x0, int y0, int special_move = 0) {
    int square = y * board_size_x + x;
    int square0 = y0 * board_size_x + x0;

    move_piece_only_board(square, square0, special_move, false);

    bool cant_be_captured = check_cant_be_captured(king_positions[current_color_number]);

    move_piece_only_board_undo(false);

    return cant_be_captured;
}


void reverse_color() {
    current_color_number = 1 - current_color_number;
    current_color_pieces_id_multiplier = current_color_number * pieces_type_count;
}


bool isAddingAvailableMoveToList = true;
void add_available_square(int x, int y, int x0, int y0, int special_move = 0) {
    if (check_king_wont_be_captured(x, y, x0, y0, special_move)) {
        available_squares[available_squares_count][0] = x + y * 8;
        available_squares[available_squares_count][1] = special_move;
        available_squares_count++;
    }
}

void move_piece_rays(int x0, int y0, int moveArray[4][2]) {
    int x, y, dx, dy;
    for (int i = 0; i != 4; i++) {
        dx = moveArray[i][0];
        dy = moveArray[i][1];
        x = x0 + dx;
        y = y0 + dy;

        while (check_is_possible(x, y)) {
            if (check_is_square_empty(x, y)) {
                add_available_square(x, y, x0, y0);
            }
            else {
                if (check_is_square_opponent(x, y)) {
                    add_available_square(x, y, x0, y0);
                }
                break;
            }
            x += dx;
            y += dy;
        }
    }
}

void move_piece_one(int x0, int y0, int moveArray[8][2]) {
    int x, y;
    for (int i = 0; i < 8; i++) {
        x = x0 + moveArray[i][0];
        y = y0 + moveArray[i][1];

        if (check_is_possible(x, y) && check_is_square_not_mine(x, y)) {
            add_available_square(x, y, x0, y0);
        }
    }
}


void get_available_square(int square) {
    available_squares_count = 0;
    int x0 = square % board_size_x;
    int y0 = square / board_size_x;
    int piece_type = get_piece_type(x0, y0);

    if (piece_type == 0) {

        if ((current_color_number == 0 && y0 == 6) || (current_color_number == 1 && y0 == 1)) { pawn_steps_count = 2; }
        else { pawn_steps_count = 1; }
        int move_direction = current_color_number * 2 - 1;

        for (int i = 1; i <= pawn_steps_count; i++) {
            if (check_is_square_empty(x0, y0 + i * move_direction)) {

                if ((current_color_number == 0 && square / board_size_x == 0) ||
                    (current_color_number == 1 && square / board_size_x == 7)) {
                    // Check if its promotion
                    add_available_square(x0, y0 + i * move_direction, x0, y0, 3);
                }
                else {
                    add_available_square(x0, y0 + i * move_direction, x0, y0);
                }
            }
            else { break; }
        }

        int y = y0 + move_direction;
        for (int x = x0 - 1; x != x0 + 3; x += 2) {
            if (check_is_possible(x, y)) {
                if (check_is_square_opponent(x, y)) {
                    add_available_square(x, y, x0, y0);
                }
                else if (check_is_square_opponent(x, y0) && get_piece_type(x, y0) == 0
                    && (last_moved_piece[1] == x + y0 * board_size_x)
                    && (last_moved_piece[1] - last_moved_piece[0] == 2 * board_size_x)) {
                    add_available_square(x, y, x0, y0, 1);
                }
            }
        }

    }
    else if (piece_type == 1) {
        move_piece_rays(x0, y0, BishopXY);

    }
    else if (piece_type == 2) {
        move_piece_one(x0, y0, KnightXY);

    }
    else if (piece_type == 3) {
        move_piece_rays(x0, y0, RookXY);

    }
    else if (piece_type == 4) {
        move_piece_rays(x0, y0, RookXY);
        move_piece_rays(x0, y0, BishopXY);

    }
    else if (piece_type == 5) {
        int x, y;
        for (int i = 0; i < 8; i++) {
            x = x0 + QueenXY[i][0];
            y = y0 + QueenXY[i][1];

            if (check_is_possible(x, y) && check_is_square_not_mine(x, y)) {
                add_available_square(x, y, x0, y0);
            }
        }

        // Check castling
        if ((can_castle[current_color_number][0] || can_castle[current_color_number][1]) && (is_checked_king == false)) {
            int x;

            // Check right side castling
            for (x = 5; x <= 6; x++) {
                if ((check_is_square_empty(x, y0) && check_king_wont_be_captured(x, y0, x0, y0)) == false) {
                    break;
                }
            }
            if (x == 7 && get_piece_type(x, y0) == 3) {
                add_available_square(x0 + 2, y0, x0, y0, 2);
            }

            // Check left side castling
            for (x = 3; x >= 2; x--) {
                if ((check_is_square_empty(x, y0) && check_king_wont_be_captured(x, y0, x0, y0)) == false) {
                    break;
                }
            }
            if (check_is_square_empty(x, y0)) {
                x--;
            }
            if (x == 0 && get_piece_type(x, y0) == 3) {
                add_available_square(x0 - 2, y0, x0, y0, 2);
            }
        }
    }
}


void move_piece(int move_id, int square0) {
    int square = available_squares[move_id][0];
    int piece_type = Board_pieces[square0] % pieces_type_count;
    int piece_captured_type = Board_pieces[square] % pieces_type_count;
    int current_color_multiplier = 1 - current_color_number * 2;

    move_piece_only_board(square, square0, available_squares[move_id][1], true);

    // Check if no longer can castle
    if (can_castle[current_color_number][0] || can_castle[current_color_number][1]) {
        if (piece_type == 5) {
            can_castle[current_color_number][0] = false;
            can_castle[current_color_number][1] = false;
        }
        else if (piece_type == 3) {
            if (square % board_size_x == 0) {
                can_castle[current_color_number][0] = false;
            }
            else if (square % board_size_x == 7) {
                can_castle[current_color_number][1] = false;
            }
        }
    }

    if (is_checked_king) { is_checked_king = false; }
    // Check if it makes check
    reverse_color();

    if (check_cant_be_captured(king_positions[current_color_number]) == false) {
        is_checked_king = true;

        int numberOfTotalMoves = 0;
        for (int i = 0; i < board_size; i++) {
            if (check_is_square_mine(i)) {
                get_available_square(i);
                numberOfTotalMoves += available_squares_count;
            }
            if (numberOfTotalMoves != 0) { break; }
        }

        if (numberOfTotalMoves == 0) {
            is_checkmate = true;
        }
    }
    // End check

    last_moved_piece[0] = square0;
    last_moved_piece[1] = square;

    selected_piece = -1;
}



const string PiecesName[6] = { "Pawn","Bishop","Knight","Rook","Queen","King" };


bool isCanBeTakenByCheaper(int position) {
    return false;
}



bool is_white_side_view = true;
sf::Font font;
sf::Text fpsText("0", font, 25);
sf::Text Material_imbalance_sprite("0", font, 25);
sf::Text Game_moves_count_sprite("0", font, 25);
sf::Sprite Board_sprite;
sf::RectangleShape Panel_sprite;
vector<sf::Sprite> Pieces(2 * pieces_type_count, sf::Sprite());
sf::RectangleShape Selected_piece_sprite;
sf::RectangleShape Checked_king_sprite;
sf::RectangleShape Last_moved_piece_sprite[last_moved_piece_number];
sf::RectangleShape Checkmate_sprite;
sf::RectangleShape Is_bot_playing_sprite;
sf::RectangleShape Button_make_move;
sf::RectangleShape Button_reverse_sides;
sf::RectangleShape Button_restart;

sf::Vector2f from_square_to_vector2(int square) {
    int endX = square % board_size_x; int endY = square / board_size_x;
    if (is_white_side_view == false) { 
        endX = board_size_x - 1 - endX; 
        endY = board_size_x - 1 - endY; 
    }
    return sf::Vector2f::Vector2(static_cast<float>(endX) * window_part_x, static_cast<float>(endY) * window_part_y);
}

void draw_possible_place_piece(int square) {
    int x = square % board_size_x;
    int y = square / board_size_x; 

    if (is_white_side_view == false) { 
        y = board_size_x - 1 - y; 
    }

    Place_piece_sprite.setPosition(
        (static_cast<float>(x) + 0.3f) * window_part_x, 
        (static_cast<float>(y) + 0.3f) * window_part_y);
    window.draw(Place_piece_sprite);
}




void screen_update() {

    // Draw basics
    window.clear();
    window.draw(Board_sprite);
    window.draw(Panel_sprite);


    // Draw selected piece
    if (selected_piece != -1) {
        int x = selected_piece % board_size_x;
        int y = selected_piece / board_size_x;
        if (is_white_side_view == false) {
            y = board_size_x - 1 - y;
        }
        Selected_piece_sprite.setPosition(window_part_x * x, window_part_y * y);
        window.draw(Selected_piece_sprite); 
    }

    // Draw last moves
    if (game_moves_count != 0) {
        for (int i = 0; i < last_moved_piece_number; i++) {
            window.draw(Last_moved_piece_sprite[i]);
        }
    }

    // Draw if check
    if (is_checked_king) {
        Checked_king_sprite.setPosition(from_square_to_vector2(king_positions[current_color_number]));
        window.draw(Checked_king_sprite);
    }


    // Draw pieces on board
    int piece_type;
    for (int y = 0; y < board_size_x; y++) {
        for (int x = 0; x < board_size_x; x++) {
            piece_type = Board_pieces[x + y * board_size_x];
            if (piece_type != -1) {
                if (is_white_side_view) {
                    Pieces[piece_type].setPosition(x * window_part_x, y * window_part_y);
                }
                else {
                    Pieces[piece_type].setPosition(x * window_part_x, (board_size_x - 1 - y) * window_part_y);
                }
                window.draw(Pieces[piece_type]);
            }
        }
    }


    // Draw possible moves
    if (selected_piece != -1) {
        for (int i = 0; i != available_squares_count; i++) {
            draw_possible_place_piece(available_squares[i][0]);
        }
    }

    Last_moved_piece_sprite[0].setPosition(from_square_to_vector2(last_moved_piece[0]));
    Last_moved_piece_sprite[1].setPosition(from_square_to_vector2(last_moved_piece[1]));


    // Draw if checkmate
    if (is_checkmate) {
        window.draw(Checkmate_sprite);
    }


    // Draw panel
    window.draw(Is_bot_playing_sprite);
    Material_imbalance_sprite.setString(to_string(material_imbalance));
    window.draw(Material_imbalance_sprite);
    Material_imbalance_sprite.setString(to_string(game_moves_count));
    window.draw(Game_moves_count_sprite);
    window.draw(Button_make_move);
    window.draw(Button_reverse_sides);
    window.draw(Button_restart);
    //window.draw(fpsText);


    window.display();
}


bool check_is_clicked(sf::RectangleShape icon, sf::Vector2i mouse_position) {
    sf::Vector2f icon_position = icon.getPosition();
    sf::Vector2f icon_size = icon.getSize();

    return icon_position.x <= mouse_position.x && mouse_position.x <= icon_position.x + icon_size.x
        && icon_position.y <= mouse_position.y && mouse_position.y <= icon_position.y + icon_size.y;
}








class Game_state {
public:
    bool is_move_white;
    bool is_checked_king;
    bool is_checkmate;
    int material_imbalance;
    int game_moves_count;
    bool can_castle[2][2];
    int king_positions[2];
    int Board_pieces[board_size];
};


void sync_game_state(Game_state game_state, bool is_sync_board_pieces) {
    is_move_white = game_state.is_move_white;
    if (is_move_white) { 
        current_color_number = 0; 
    } else {
        current_color_number = 1;
    }

    is_checked_king = game_state.is_checked_king;
    is_checkmate = game_state.is_checkmate;
    material_imbalance = game_state.material_imbalance;
    game_moves_count = game_state.game_moves_count;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            can_castle[i][j] = game_state.can_castle[i][j];
        }
        king_positions[i] = game_state.king_positions[i];
    }
    
    if (is_sync_board_pieces) {
        for (int i = 0; i < board_size; i++) {
            Board_pieces[i] = game_state.Board_pieces[i];
        }
    }
}


void bot_analyze() {
    static const int analyze_steps_max_count = 2;
    static const int most_possible_moves_in_one_turn = 218;

    int current_position[analyze_steps_max_count];

    Game_state game_states[analyze_steps_max_count];
    int possible_moves_count[analyze_steps_max_count];
    int possible_moves[analyze_steps_max_count][most_possible_moves_in_one_turn][available_squares_data_count+1];

    int step = 0;
    int best_profit = 0;
    while (true) {
        // Check new moves
        for (; step < analyze_steps_max_count; step++) {

            // Copying current game_state
            game_states[step].is_move_white = is_move_white;
            game_states[step].is_checked_king = is_checked_king;
            game_states[step].is_checkmate = is_checkmate;
            game_states[step].material_imbalance = material_imbalance;
            game_states[step].game_moves_count = game_moves_count;

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    game_states[step].can_castle[i][j] = can_castle[i][j];
                }
                game_states[step].king_positions[i] = king_positions[i];
            }

            for (int i = 0; i < board_size; i++) {
                game_states[step].Board_pieces[i] = Board_pieces[i];
            }

            // Setting current_position
            current_position[step] = 0;

            // Adding all possible moves
            possible_moves_count[step] = 0;
            int first_piece_i = -1;

            for (int i = 0; i < board_size; i++) {
                if (check_is_square_mine(i)) {
                    get_available_square(i);
                    if (available_squares_count > 0) {
                        for (int j = 0; j < available_squares_count; j++) {
                            possible_moves[step][possible_moves_count[step]][0] = available_squares[j][0];
                            possible_moves[step][possible_moves_count[step]][1] = available_squares[j][1];

                            possible_moves[step][possible_moves_count[step]][2] = i;
                            possible_moves_count[step]++;
                        }

                        if (first_piece_i == -1) {
                            first_piece_i = i;
                        }
                    }
                }
            }

            if (possible_moves_count[step] > 0) {
                if (step < analyze_steps_max_count - 1) {
                    move_piece(0, first_piece_i);
                }
            }
            else {
                // No possible moves / stallmate / checkmate
                break;
            }
        }

        cout << endl;
        step--; // step = analyze_steps_max_count, so we need step 1 back
        cout << "1 " << step << " " << possible_moves_count[step] << " " << current_color_number << endl;

        for (int i = 0; i < possible_moves_count[step]; i++) {
            cout << possible_moves[step][i][2] << " ";
            move_piece_only_board(possible_moves[step][i][0], possible_moves[step][i][2], possible_moves[step][i][1], true);

            int profit = 0;
            // Count profit
            //if ((is_move_white && profit > best_profit))

            move_piece_only_board_undo(true);
            sync_game_state(game_states[step], false);
        }

        step--;
        reverse_color();
        for (; step >= 0; step--) {
            current_position[step]++;
            if (current_position[step] < possible_moves_count[step]) {
                break;
            }
            reverse_color();
        }
        for (int i = 0; i < 30; i++) {
            screen_update();
        }

        if (step >= 0) {
            sync_game_state(game_states[step], true);
        }
        else {
            break;
        }

    }

}









int main()
{
    // SETUP AND DECLARATIONS


    // Import text file
    if (!font.loadFromFile("../Arial.ttf")) {
        cout << "Error text";
        window.close();
    }


    // FPS text, clock, etc
    window.setFramerateLimit(60);
    fpsText.setOutlineColor(sf::Color::Black);
    fpsText.setOutlineThickness(4);
    fpsText.setPosition(window_size_x + 10, 100);
    sf::Clock clock;


    // Board
    sf::Texture Board_texture;
    if (!Board_texture.loadFromFile("../ChessBoard.png")) {}
    Board_sprite.setTexture(Board_texture);
    Board_sprite.setScale(window_size_x_float / Board_texture.getSize().x, window_size_y_float / Board_texture.getSize().y);


    // Panel
    Panel_sprite.setSize(sf::Vector2f(50, window_size_y));
    Panel_sprite.setPosition(window_size_x, 0);
    Panel_sprite.setFillColor(sf::Color::Color(51, 51, 51));


    // Pieces textures, sprites
    const string Piece_colors[2] = { "White","Black" };
    const string Piece_names[pieces_type_count] = { "Pawn","Bishop","Knight","Rook","Queen","King" };
    vector<sf::Texture> Piece_textures(2 * pieces_type_count, sf::Texture());

    for (int i = 0; i < 2 * pieces_type_count; i++) {
        // Load texture getting name by color and unique_piece
        Piece_textures[i].loadFromFile("../" +
            Piece_colors[i / pieces_type_count] +
            Piece_names[i % pieces_type_count] + ".png");

        Piece_textures[i].setSmooth(true);
        Pieces[i].setTexture(Piece_textures[i]);
        Pieces[i].setScale(
            window_part_x / Piece_textures[i].getSize().x,
            window_part_y / Piece_textures[i].getSize().y);
    }


    // Selected Piece
    Selected_piece_sprite.setSize(sf::Vector2f(window_part_x, window_part_y));
    Selected_piece_sprite.setFillColor(sf::Color::Color(0, 255, 0));


    // Checked King
    Checked_king_sprite.setSize(sf::Vector2f(window_part_x, window_part_y));
    Checked_king_sprite.setFillColor(sf::Color::Color(255, 0, 0));


    // Place Piece
    Place_piece_sprite.setFillColor(sf::Color::Color(127, 127, 127, 127));


    // Last Moved Piece
    for (int i = 0; i < last_moved_piece_number; i++) {
        Last_moved_piece_sprite[i].setSize(sf::Vector2f(window_part_x, window_part_y));
        Last_moved_piece_sprite[i].setFillColor(sf::Color::Color(0, 127, 255, 101));
    }


    // Checkmate Sprite
    Checkmate_sprite.setSize(sf::Vector2f(window_size_x, window_size_y));
    Checkmate_sprite.setFillColor(sf::Color::Color(127, 127, 127, 127));


    // Is bot playing sprite
    Is_bot_playing_sprite.setSize(sf::Vector2f(32, 32));
    Is_bot_playing_sprite.setPosition(window_size_x + 14, 4);
    Is_bot_playing_sprite.setFillColor(sf::Color::Color(255, 0, 0));
    Is_bot_playing_sprite.setOutlineThickness(4);
    Is_bot_playing_sprite.setOutlineColor(sf::Color::Color(0, 0, 0));
    bool is_bot_playing = false;


    // Material Imbalabce Sprite
    Material_imbalance_sprite.setOutlineColor(sf::Color::Black);
    Material_imbalance_sprite.setOutlineThickness(4);
    Material_imbalance_sprite.setPosition(window_size_x + 10, 200);


    // Game Moves Count Sprite
    Game_moves_count_sprite.setOutlineColor(sf::Color::Black);
    Game_moves_count_sprite.setOutlineThickness(4);
    Game_moves_count_sprite.setPosition(window_size_x + 10, 250);


    // Button Make Move
    Button_make_move.setSize(sf::Vector2f(40, 40));
    Button_make_move.setPosition(window_size_x + 10, 300);
    Button_make_move.setFillColor(sf::Color::Color(127, 127, 127));


    // Button Reverse sides
    Button_reverse_sides.setSize(sf::Vector2f(32, 32));
    Button_reverse_sides.setPosition(window_size_x + 14, 404);
    Button_reverse_sides.setOutlineThickness(4);
    Button_reverse_sides.setFillColor(sf::Color::Color(255, 255, 255));
    Button_reverse_sides.setOutlineColor(sf::Color::Color(0, 0, 0));

    // Button Restart
    Button_restart.setSize(sf::Vector2f(32.0f, 32.0f));
    Button_restart.setPosition(window_size_x + 14, 454);
    Button_restart.setOutlineThickness(4.f);
    Button_restart.setFillColor(sf::Color::Color(0, 0, 0));
    Button_restart.setOutlineColor(sf::Color::Color(255, 0, 0));


    // Declaration variables
    sf::Vector2i mousePosition;
    int mouseClickedPiece;
    float tick, fps;

    const int StartBackPosition[8] = { 3,2,1,4,5,1,2,3 };
    Game_state game_state_starting;
    game_state_starting.is_move_white = true;
    game_state_starting.is_checked_king = false;
    game_state_starting.is_checkmate = false;
    game_state_starting.material_imbalance = 0;
    game_state_starting.game_moves_count = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            game_state_starting.can_castle[i][j] = true;
        }
    }
    game_state_starting.king_positions[0] = 60;
    game_state_starting.king_positions[1] = 4;
    for (int i = 0; i != 8; i++) { game_state_starting.Board_pieces[i] = StartBackPosition[i] + 6; }
    for (int i = 8; i != 16; i++) { game_state_starting.Board_pieces[i] = 6; }
    for (int i = 16; i != 48; i++) { game_state_starting.Board_pieces[i] = -1; }
    for (int i = 48; i != 56; i++) { game_state_starting.Board_pieces[i] = 0; }
    for (int i = 0; i != 8; i++) { game_state_starting.Board_pieces[i + 56] = StartBackPosition[i]; }





    // Main menu
    sync_game_state(game_state_starting, true);
    screen_update();
    while (window.isOpen())
    {
        sf::Event event;
        int is_screen_update = false;


        // Inputs
        while (window.pollEvent(event)) {

            // Close game
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {

                // Select clicked piece
                mousePosition = sf::Mouse::getPosition(window);
                int x_int = int(mousePosition.x / window_part_x);
                int y_int = int(mousePosition.y / window_part_y);

                if (is_white_side_view == false) { 
                    x_int = board_size_x - 1 - x_int; 
                    y_int = board_size_x - 1 - y_int; 
                }

                if (check_is_possible(x_int, y_int)) {
                    int x_int = int(mousePosition.x / window_part_x);
                    int y_int = int(mousePosition.y / window_part_y);

                    if (is_white_side_view == false) {
                        y_int = board_size_x - 1 - y_int;
                    }

                    mouseClickedPiece = x_int + y_int * board_size_x;

                    bool is_new_available_square = true;
                    if (selected_piece != -1) {
                        for (int i = 0; i < available_squares_count; i++) {
                            if (available_squares[i][0] == mouseClickedPiece) {
                                move_piece(i, selected_piece);
                                screen_update();
                                is_new_available_square = false;
                                break;
                            }
                        }
                    }

                    if (is_new_available_square) {
                        if (check_is_square_not_mine(x_int, y_int)) {
                            selected_piece = -1;
                            screen_update();
                        } else {
                            selected_piece = mouseClickedPiece;
                            get_available_square(mouseClickedPiece);
                            screen_update();
                        }
                    }
                }
                else if (check_is_clicked(Button_reverse_sides, mousePosition)) {
                    is_white_side_view = !is_white_side_view;
                    screen_update();
                }
                else if (check_is_clicked(Button_make_move, mousePosition)) {
                    bot_analyze();
                }
            }
        }



        tick = clock.restart().asSeconds();
        fps = round(1.f / tick);
        fpsText.setString(to_string(fps));
    }

    return 0;
}
