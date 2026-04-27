// ============================================================
//  MINESWEEPER - main.cpp
//  Raylib GUI | Beginner OOP
// ============================================================

#include "raylib.h"
#include <string>
#include <cstdlib>
#include <ctime>

// ============================================================
//  CONSTANTS
// ============================================================

const int WINDOW_W = 800;
const int WINDOW_H = 600;
const int CELL_SIZE = 36;

// ============================================================
//  ENUM - tracks which screen we are on
// ============================================================

enum GameStatus {
    LOGIN       = 0,
    MENU        = 1,
    DIFFICULTY  = 2,
    GAME        = 3,
    WIN         = 4,
    LOSS        = 5,
    LEADERBOARD = 6
};

GameStatus currentScreen = LOGIN;

// ============================================================
//  CLASSES: Cell, MineCell, SafeCell
// ============================================================

class Cell {
public:
    int  x;
    int  y;
    char tag;       // 'M' = mine, 'S' = safe
    bool isHidden;
    bool isFlagged;

    Cell() {
        x         = 0;
        y         = 0;
        tag       = '?';
        isHidden  = true;
        isFlagged = false;
    }

    virtual void RevealCell() = 0;

    virtual ~Cell() {}
};


class MineCell : public Cell {
public:
    MineCell() {
        tag = 'M';
    }

    void RevealCell() override {
        isHidden = false;
    }
};


class SafeCell : public Cell {
public:
    int displayValue;   // how many mines are touching this cell

    SafeCell() {
        tag          = 'S';
        displayValue = 0;
    }

    void RevealCell() override {
        isHidden = false;
    }
};

// ============================================================
//  CLASS: Minesweeper  (holds the board and game info)
// ============================================================

class Minesweeper {
public:
    char Difficulty;
    int  size;
    int  MineCount;
    int  MinesFlagged;

    Cell** Board;   // dynamic 2D array - size changes with difficulty

    Minesweeper() {
        Difficulty    = 'E';
        size          = 8;
        MineCount     = 10;
        MinesFlagged  = 0;
        Board         = nullptr;
    }

    void SetDifficulty(char setting) {
        Difficulty = setting;

        if (Difficulty == 'E') { size =  8; MineCount = 10; }
        if (Difficulty == 'M') { size = 16; MineCount = 40; }
        if (Difficulty == 'H') { size = 20; MineCount = 99; }
    }

    void ResetBoard() {
        MinesFlagged = 0;

        // Delete all the cells and the array itself
        if (Board != nullptr) {
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    delete Board[y][x];
                }
                delete[] Board[y];   // delete each row
            }
            delete[] Board;          // delete the array of row pointers
        }

        // Allocate a fresh board at the current size
        Board = new Cell*[size];
        for (int y = 0; y < size; y++) {
            Board[y] = new Cell*[size];
            for (int x = 0; x < size; x++) {
                Board[y][x] = nullptr;
            }
        }
    }

    void PopulateBoard() {
        // Step 1: Place mines
        int minesToPlace = MineCount;

        while (minesToPlace > 0) {
            int rx = rand() % size;
            int ry = rand() % size;

            if (Board[ry][rx] == nullptr) {
                Board[ry][rx] = new MineCell();
                Board[ry][rx]->x = rx;
                Board[ry][rx]->y = ry;
                minesToPlace--;
            }
        }

        // Step 2: Fill the rest with safe cells
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (Board[y][x] == nullptr) {
                    Board[y][x] = new SafeCell();
                    Board[y][x]->x = x;
                    Board[y][x]->y = y;
                }
            }
        }

        // Step 3: Count mines around each safe cell
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (Board[y][x]->tag == 'S') {
                    int count = 0;

                    for (int yo = -1; yo <= 1; yo++) {
                        for (int xo = -1; xo <= 1; xo++) {
                            int ny = y + yo;
                            int nx = x + xo;

                            // Check it's inside the board
                            if (ny >= 0 && ny < size && nx >= 0 && nx < size) {
                                if (Board[ny][nx]->tag == 'M') {
                                    count++;
                                }
                            }
                        }
                    }

                    SafeCell* s = (SafeCell*)Board[y][x];
                    s->displayValue = count;
                }
            }
        }
    }

    bool CheckWin() {
        // Win = all safe cells have been revealed
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (Board[y][x]->tag == 'S' && Board[y][x]->isHidden == true) {
                    return false;   // still hidden safe cells left
                }
            }
        }
        return true;
    }

    void RevealAllMines() {
        for (int y = 0; y < size; y++)
            for (int x = 0; x < size; x++)
                if (Board[y][x]->tag == 'M')
                    Board[y][x]->isHidden = false;
    }

    ~Minesweeper() {
        if (Board != nullptr) {
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    delete Board[y][x];
                }
                delete[] Board[y];
            }
            delete[] Board;
        }
    }
};

// ============================================================
//  GLOBAL OBJECTS
// ============================================================

Minesweeper game;
std::string  username   = "";
std::string  errorMsg   = "";
float        gameTimer  = 0.0f;
bool         scoreSaved = false;

// Simple leaderboard - just parallel arrays
std::string scoreNames[10];
float       scoreTimes[10];
int         scoreCount = 0;

// ============================================================
//  BASE CLASS: Screen
// ============================================================

class Screen {
public:
    virtual void Render()      = 0;
    virtual void HandleInput() = 0;
    virtual ~Screen() {}
};

// ============================================================
//  LOGIN SCREEN
// ============================================================

class LoginScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("MINESWEEPER",     250, 80,  50, YELLOW);
        DrawText("Enter username:", 270, 220, 22, WHITE);

        // Input box
        DrawRectangle(250, 255, 300, 40, DARKGRAY);
        DrawText(username.c_str(), 260, 265, 22, WHITE);

        DrawText(errorMsg.c_str(), 250, 310, 18, RED);
        DrawText("Press ENTER to continue", 250, 350, 18, GRAY);
    }

    void HandleInput() override {
        // Read typed characters
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 125 && (int)username.size() < 15) {
                username += (char)key;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && username.size() > 0) {
            username.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (username.size() > 0) {
                currentScreen = MENU;
                errorMsg = "";
            } else {
                errorMsg = "Please enter a username!";
            }
        }
    }
};

// ============================================================
//  MENU SCREEN
// ============================================================

class MenuScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("MAIN MENU",       300, 80,  44, YELLOW);
        DrawText(("Hi, " + username + "!").c_str(), 300, 145, 20, GRAY);

        DrawText("P - Play",        300, 250, 28, GREEN);
        DrawText("D - Difficulty",  300, 300, 28, YELLOW);
        DrawText("L - Leaderboard", 300, 350, 28, SKYBLUE);
    }

    void HandleInput() override {
        if (IsKeyPressed(KEY_P)) {
            game.ResetBoard();
            game.PopulateBoard();
            gameTimer  = 0.0f;
            scoreSaved = false;
            currentScreen = GAME;
        }
        if (IsKeyPressed(KEY_D)) currentScreen = DIFFICULTY;
        if (IsKeyPressed(KEY_L)) currentScreen = LEADERBOARD;
    }
};

// ============================================================
//  DIFFICULTY SCREEN
// ============================================================

class DifficultyScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("DIFFICULTY",                        270, 80,  44, YELLOW);
        DrawText("E - Easy   (8x8,   10 mines)",      200, 220, 24, GREEN);
        DrawText("M - Medium (16x16, 40 mines)",      200, 270, 24, YELLOW);
        DrawText("H - Hard   (20x20, 99 mines)",      200, 320, 24, RED);
        DrawText("B - Back",                          200, 400, 20, GRAY);

        // Show current selection
        std::string curr = "Selected: ";
        if      (game.Difficulty == 'E') curr += "Easy";
        else if (game.Difficulty == 'M') curr += "Medium";
        else                             curr += "Hard";
        DrawText(curr.c_str(), 200, 460, 20, LIGHTGRAY);
    }

    void HandleInput() override {
        if (IsKeyPressed(KEY_E)) game.SetDifficulty('E');
        if (IsKeyPressed(KEY_M)) game.SetDifficulty('M');
        if (IsKeyPressed(KEY_H)) game.SetDifficulty('H');
        if (IsKeyPressed(KEY_B)) currentScreen = MENU;
    }
};

// ============================================================
//  GAME SCREEN
// ============================================================

class GameScreen : public Screen {
public:

    // Figure out where to draw the board so it's centred
    int offsetX() { return (WINDOW_W - game.size * CELL_SIZE) / 2; }
    int offsetY() { return 60 + (WINDOW_H - 60 - game.size * CELL_SIZE) / 2; }

    void Render() override {
        ClearBackground(BLACK);

        // HUD
        DrawText(("Mines left: " + std::to_string(game.MineCount - game.MinesFlagged)).c_str(),
                 10, 10, 22, RED);
        DrawText(("Time: " + std::to_string((int)gameTimer) + "s").c_str(),
                 300, 10, 22, WHITE);
        DrawText("Left click: reveal  |  Right click: flag  |  R: menu",
                 10, 38, 14, DARKGRAY);

        // Draw each cell
        int ox = offsetX();
        int oy = offsetY();

        for (int y = 0; y < game.size; y++) {
            for (int x = 0; x < game.size; x++) {
                Cell* c = game.Board[y][x];
                if (c == nullptr) continue;

                int drawX = ox + x * CELL_SIZE;
                int drawY = oy + y * CELL_SIZE;

                if (c->isHidden && c->isFlagged) {
                    DrawRectangle(drawX, drawY, CELL_SIZE - 2, CELL_SIZE - 2, ORANGE);
                    DrawText("F", drawX + 10, drawY + 8, 18, WHITE);

                } else if (c->isHidden) {
                    DrawRectangle(drawX, drawY, CELL_SIZE - 2, CELL_SIZE - 2, DARKGRAY);

                } else if (c->tag == 'M') {
                    DrawRectangle(drawX, drawY, CELL_SIZE - 2, CELL_SIZE - 2, RED);
                    DrawText("*", drawX + 10, drawY + 8, 18, WHITE);

                } else {
                    // Revealed safe cell
                    DrawRectangle(drawX, drawY, CELL_SIZE - 2, CELL_SIZE - 2, LIGHTGRAY);

                    SafeCell* s = (SafeCell*)c;
                    if (s->displayValue > 0) {
                        DrawText(std::to_string(s->displayValue).c_str(),
                                 drawX + 10, drawY + 8, 18, BLUE);
                    }
                }

                // Cell border
                DrawRectangleLines(drawX, drawY, CELL_SIZE - 2, CELL_SIZE - 2, GRAY);
            }
        }
    }

    void HandleInput() override {
        int ox = offsetX();
        int oy = offsetY();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int bx = (GetMouseX() - ox) / CELL_SIZE;
            int by = (GetMouseY() - oy) / CELL_SIZE;

            if (bx >= 0 && bx < game.size && by >= 0 && by < game.size) {
                Cell* c = game.Board[by][bx];
                if (c->isHidden && !c->isFlagged) {
                    c->RevealCell();

                    if (c->tag == 'M') {
                        game.RevealAllMines();
                        currentScreen = LOSS;
                    }

                    if (game.CheckWin()) {
                        currentScreen = WIN;
                    }
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            int bx = (GetMouseX() - ox) / CELL_SIZE;
            int by = (GetMouseY() - oy) / CELL_SIZE;

            if (bx >= 0 && bx < game.size && by >= 0 && by < game.size) {
                Cell* c = game.Board[by][bx];
                if (c->isHidden) {
                    c->isFlagged = !c->isFlagged;
                    if (c->isFlagged) game.MinesFlagged++;
                    else              game.MinesFlagged--;
                }
            }
        }

        if (IsKeyPressed(KEY_R)) {
            game.ResetBoard();
            currentScreen = MENU;
        }
    }
};

// ============================================================
//  WIN SCREEN
// ============================================================

class WinScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("YOU WIN!",  270, 180, 60, YELLOW);
        DrawText(("Time: " + std::to_string((int)gameTimer) + " seconds").c_str(), 270, 270, 26, WHITE);
        DrawText("R - Play Again   |   L - Leaderboard   |   ESC - Menu", 150, 370, 20, GRAY);
    }

    void HandleInput() override {
        if (IsKeyPressed(KEY_R)) {
            game.ResetBoard();
            game.PopulateBoard();
            gameTimer  = 0.0f;
            scoreSaved = false;
            currentScreen = GAME;
        }
        if (IsKeyPressed(KEY_L))      currentScreen = LEADERBOARD;
        if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;
    }
};

// ============================================================
//  LOSS SCREEN
// ============================================================

class LossScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("GAME OVER",   240, 180, 60, RED);
        DrawText("You hit a mine!", 270, 270, 26, WHITE);
        DrawText("R - Try Again   |   ESC - Menu", 240, 370, 20, GRAY);
    }

    void HandleInput() override {
        if (IsKeyPressed(KEY_R)) {
            game.ResetBoard();
            game.PopulateBoard();
            gameTimer  = 0.0f;
            scoreSaved = false;
            currentScreen = GAME;
        }
        if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MENU;
    }
};

// ============================================================
//  LEADERBOARD SCREEN
// ============================================================

class LeaderboardScreen : public Screen {
public:
    void Render() override {
        ClearBackground(BLACK);
        DrawText("LEADERBOARD", 280, 50, 44, YELLOW);

        if (scoreCount == 0) {
            DrawText("No scores yet!", 300, 250, 24, GRAY);
        }

        for (int i = 0; i < scoreCount; i++) {
            std::string line = std::to_string(i + 1) + ".  " +
                               scoreNames[i] + "  -  " +
                               std::to_string((int)scoreTimes[i]) + "s";
            DrawText(line.c_str(), 200, 130 + i * 35, 22, WHITE);
        }

        DrawText("B - Back to Menu", 290, 540, 20, GRAY);
    }

    void HandleInput() override {
        if (IsKeyPressed(KEY_B)) currentScreen = MENU;
    }
};

// ============================================================
//  SAVE SCORE  (simple - just adds to the arrays)
// ============================================================

void SaveScore(std::string name, float time) {
    if (scoreCount < 10) {
        scoreNames[scoreCount] = name;
        scoreTimes[scoreCount] = time;
        scoreCount++;
    }

    // Bubble sort - lowest time goes to the top
    for (int i = 0; i < scoreCount - 1; i++) {
        for (int j = 0; j < scoreCount - i - 1; j++) {
            if (scoreTimes[j] > scoreTimes[j + 1]) {
                float       tempTime = scoreTimes[j];
                std::string tempName = scoreNames[j];
                scoreTimes[j] = scoreTimes[j + 1];
                scoreNames[j] = scoreNames[j + 1];
                scoreTimes[j + 1] = tempTime;
                scoreNames[j + 1] = tempName;
            }
        }
    }
}

// ============================================================
//  MAIN
// ============================================================

int main() {

    srand(time(nullptr));

    SetTargetFPS(60);
    InitWindow(WINDOW_W, WINDOW_H, "Minesweeper");

    game.SetDifficulty('E');

    LoginScreen       loginScreen;
    MenuScreen        menuScreen;
    DifficultyScreen  diffScreen;
    GameScreen        gameScreen;
    WinScreen         winScreen;
    LossScreen        lossScreen;
    LeaderboardScreen leaderboardScreen;

    while (!WindowShouldClose()) {

        // Update the game timer only while playing
        if (currentScreen == GAME) {
            gameTimer += GetFrameTime();
        }

        // Save score once when player wins
        if (currentScreen == WIN && scoreSaved == false) {
            SaveScore(username, gameTimer);
            scoreSaved = true;
        }

        // Handle input
        switch (currentScreen) {
            case LOGIN:       loginScreen.HandleInput();       break;
            case MENU:        menuScreen.HandleInput();        break;
            case DIFFICULTY:  diffScreen.HandleInput();        break;
            case GAME:        gameScreen.HandleInput();        break;
            case WIN:         winScreen.HandleInput();         break;
            case LOSS:        lossScreen.HandleInput();        break;
            case LEADERBOARD: leaderboardScreen.HandleInput(); break;
        }

        // Draw
        BeginDrawing();

        switch (currentScreen) {
            case LOGIN:       loginScreen.Render();       break;
            case MENU:        menuScreen.Render();        break;
            case DIFFICULTY:  diffScreen.Render();        break;
            case GAME:        gameScreen.Render();        break;
            case WIN:         winScreen.Render();         break;
            case LOSS:        lossScreen.Render();        break;
            case LEADERBOARD: leaderboardScreen.Render(); break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
