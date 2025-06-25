#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class Point {
public:
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class Snake {
private:
    vector<Point> body;
    Direction dir;

public:
    Snake(int startX, int startY) {
        body.push_back(Point(startX, startY));
        dir = STOP;
    }

    void changeDirection(Direction newDir) {
        if ((dir == LEFT && newDir == RIGHT) || (dir == RIGHT && newDir == LEFT) ||
            (dir == UP && newDir == DOWN) || (dir == DOWN && newDir == UP)) return;
        dir = newDir;
    }

    void move() {
        Point head = body[0];
        switch (dir) {
        case LEFT:  head.x--; break;
        case RIGHT: head.x++; break;
        case UP:    head.y--; break;
        case DOWN:  head.y++; break;
        default:    return;
        }

        for (int i = body.size() - 1; i > 0; i--)
            body[i] = body[i - 1];

        body[0] = head;
    }

    void grow() {
        body.push_back(body.back());
    }

    Point getHead() const {
        return body[0];
    }

    vector<Point> getBody() const {
        return body;
    }

    bool hasCollided() {
        Point head = body[0];
        for (size_t i = 1; i < body.size(); i++) {
            if (head == body[i])
                return true;
        }
        return false;
    }

    Direction getDirection() const {
        return dir;
    }
};

class Game {
private:
    int width, height, score, level, baseSpeed, nextLevelScore;
    bool gameOver;
    Point fruit, powerUp;
    Snake snake;
    vector<Point> obstacles;
    bool hasPowerUp;

    HANDLE hConsole;

public:
    Game(int w, int h)
        : width(w), height(h), score(0), level(1), baseSpeed(150), nextLevelScore(30), snake(w / 2, h / 2) {
        gameOver = false;
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        srand(time(0));
        spawnFruit();
        spawnPowerUp();
        generateObstacles();
    }

    void spawnFruit() {
        while (true) {
            fruit.x = 1 + rand() % (width - 2);
            fruit.y = 1 + rand() % (height - 2);

            bool conflict = false;

            for (auto segment : snake.getBody())
                if (segment == fruit) conflict = true;

            for (auto obs : obstacles)
                if (obs == fruit) conflict = true;

            if (!conflict) break;
        }
    }

    void spawnPowerUp() {
        while (true) {
            powerUp.x = 1 + rand() % (width - 2);
            powerUp.y = 1 + rand() % (height - 2);

            bool conflict = false;

            for (auto segment : snake.getBody())
                if (segment == powerUp) conflict = true;

            for (auto obs : obstacles)
                if (obs == powerUp) conflict = true;

            if (!conflict) break;
        }
        hasPowerUp = true;
    }

    void generateObstacles() {
        obstacles.clear();
        for (int i = 0; i < level * 3; i++) {
            Point obs;
            obs.x = 1 + rand() % (width - 2);
            obs.y = 1 + rand() % (height - 2);
            obstacles.push_back(obs);
        }
    }

    void draw() {
        system("cls");

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        for (int y = 0; y < height; y++) {
            cout << "#";
            for (int x = 0; x < width; x++) {
                Point head = snake.getHead();
                bool printed = false;

                if (x == head.x && y == head.y) {
                    SetConsoleTextAttribute(hConsole, 10);
                    cout << "O";
                    printed = true;
                } else if (x == fruit.x && y == fruit.y) {
                    SetConsoleTextAttribute(hConsole, 12);
                    cout << "F";
                    printed = true;
                } else if (hasPowerUp && x == powerUp.x && y == powerUp.y) {
                    SetConsoleTextAttribute(hConsole, 11);
                    cout << "P";
                    printed = true;
                } else {
                    for (auto p : snake.getBody()) {
                        if (x == p.x && y == p.y) {
                            SetConsoleTextAttribute(hConsole, 10);
                            cout << "o";
                            printed = true;
                            break;
                        }
                    }
                    for (auto o : obstacles) {
                        if (x == o.x && y == o.y) {
                            SetConsoleTextAttribute(hConsole, 8);
                            cout << "X";
                            printed = true;
                            break;
                        }
                    }
                }
                if (!printed) {
                    SetConsoleTextAttribute(hConsole, 7);
                    cout << " ";
                }
            }
            SetConsoleTextAttribute(hConsole, 7);
            cout << "#" << endl;
        }

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;
        cout << "Score: " << score << "  Level: " << level << "  Speed: " << baseSpeed << "ms" << endl;
    }

    void input() {
        if (_kbhit()) {
            switch (_getch()) {
            case 'a': snake.changeDirection(LEFT); break;
            case 'd': snake.changeDirection(RIGHT); break;
            case 'w': snake.changeDirection(UP); break;
            case 's': snake.changeDirection(DOWN); break;
            case 'x': gameOver = true; break;
            case 'l': // Manual level up
                level++;
                if (baseSpeed > 50) baseSpeed -= 10;
                generateObstacles();
                break;
            }
        }
    }

    void logic() {
        snake.move();
        Point head = snake.getHead();

        if (head.x <= 0 || head.x >= width - 1 || head.y <= 0 || head.y >= height - 1)
            gameOver = true;

        for (auto o : obstacles)
            if (head == o) gameOver = true;

        if (snake.hasCollided()) gameOver = true;

        if (head == fruit) {
            score += 10;
            snake.grow();
            spawnFruit();
            PlaySound(TEXT("pickup.wav"), NULL, SND_FILENAME | SND_ASYNC);

            while (score >= nextLevelScore) {
                level++;
                nextLevelScore += 30;
                if (baseSpeed > 50) baseSpeed -= 10;
                generateObstacles();
            }
        }

        if (hasPowerUp && head == powerUp) {
            score += 25;
            hasPowerUp = false;
            PlaySound(TEXT("powerup.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }
    }

    void run() {
        while (!gameOver) {
            draw();
            input();
            logic();

            int delay = baseSpeed;
            if (snake.getDirection() == UP || snake.getDirection() == DOWN)
                delay += 15;

            Sleep(delay);
        }

        SetConsoleTextAttribute(hConsole, 7);
        cout << "\nGame Over! Final Score: " << score << endl;
        PlaySound(TEXT("gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
    }
};

int main() {
    Game game(25, 20);
    game.run();
    system("pause");
    return 0;
}
