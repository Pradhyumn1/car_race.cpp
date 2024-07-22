#include <iostream>
#include <unistd.h>  // For usleep 
#include <termios.h>  // For getch()
#include <fcntl.h>  // For fcntl()
#include <stdlib.h>  // For system() and srand()
#include <ctime>  // For time()

#define SCREEN_WIDTH 90
#define SCREEN_HEIGHT 26
#define WIN_WIDTH 70

using namespace std;

void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void clear_screen() {
    printf("\033[2J");
}

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

bool kbhit() {
    struct termios oldt, newt;
    int ch;
    bool retval = false;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    ch = getchar();
    if (ch != EOF) {
        retval = true;
        ungetc(ch, stdin);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    return retval;
}

int enemyY[3];
int enemyX[3];
int enemyFlag[3];

char car[4][4] = {' ', '|', '|', ' ',
                  '|', '|', '|', '|',
                  ' ', '|', '|', ' ',
                  '|', '|', '|', '|'};

int carPos = WIN_WIDTH / 2;
int score = 0;

void drawBorder() {
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < 17; j++) {
            gotoxy(0 + j, i); cout << "|";
            gotoxy(WIN_WIDTH - j, i); cout << "|";
        }
    }

    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        gotoxy(SCREEN_WIDTH, i); cout << "|";
    }
}

void updateScore() {
    gotoxy(WIN_WIDTH + 7, 5); cout << "Score: " << score << endl;
}

void genEnemy(int ind) {
    enemyX[ind] = 17 + rand() % (33);
}

void drawCar() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            gotoxy(j + carPos, i + 22); cout << car[i][j];
        }
    }
}

void drawEnemy(int ind) {
    if (enemyFlag[ind] == true) {
        gotoxy(enemyX[ind], enemyY[ind]);   cout << "****";
        gotoxy(enemyX[ind], enemyY[ind] + 1); cout << " ** ";
        gotoxy(enemyX[ind], enemyY[ind] + 2); cout << "****";
        gotoxy(enemyX[ind], enemyY[ind] + 3); cout << " ** ";
    }
}

int collision() {
    if (enemyY[0] + 4 >= 23) {
        if (enemyX[0] + 4 - carPos >= 0 && enemyX[0] + 4 - carPos < 9) {
            return 1;
        }
    }
    return 0;
}

void gameover() {
    clear_screen();
    cout << endl;
    cout << "\t\t--------------------------" << endl;
    cout << "\t\t-------- Game Over -------" << endl;
    cout << "\t\t--------------------------" << endl << endl;
    cout << "\t\tPress any key to go back to menu.";
    getch();
}

void eraseCar() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            gotoxy(j + carPos, i + 22); cout << " ";
        }
    }
}

void eraseEnemy(int ind) {
    if (enemyFlag[ind] == true) {
        gotoxy(enemyX[ind], enemyY[ind]); cout << "    ";
        gotoxy(enemyX[ind], enemyY[ind] + 1); cout << "    ";
        gotoxy(enemyX[ind], enemyY[ind] + 2); cout << "    ";
        gotoxy(enemyX[ind], enemyY[ind] + 3); cout << "    ";
    }
}

void resetEnemy(int ind) {
    eraseEnemy(ind);
    enemyY[ind] = 1;
    genEnemy(ind);
}

void play() {
    carPos = -1 + WIN_WIDTH / 2;
    score = 0;
    enemyFlag[0] = 1;
    enemyFlag[1] = 0;
    enemyY[0] = enemyY[1] = 1;

    clear_screen();
    drawBorder();
    updateScore();
    genEnemy(0);
    genEnemy(1);

    gotoxy(WIN_WIDTH + 7, 2); cout << "Car Game";
    gotoxy(WIN_WIDTH + 6, 4); cout << "----------";
    gotoxy(WIN_WIDTH + 6, 6); cout << "----------";
    gotoxy(WIN_WIDTH + 7, 12); cout << "Control ";
    gotoxy(WIN_WIDTH + 7, 13); cout << "-------- ";
    gotoxy(WIN_WIDTH + 2, 14); cout << " A Key - Left";
    gotoxy(WIN_WIDTH + 2, 15); cout << " D Key - Right";

    gotoxy(18, 5); cout << "Press any key to start";
    getch();
    gotoxy(18, 5); cout << "                      ";

    while (true) {
        if (kbhit()) {
            char ch = getch();
            if (ch == 'a' || ch == 'A') {
                if (carPos > 18)
                    carPos -= 4;
            }
            if (ch == 'd' || ch == 'D') {
                if (carPos < 50)
                    carPos += 4;
            }
            if (ch == 27) {
                break;
            }
        }

        drawCar();
        drawEnemy(0);
        drawEnemy(1);

        if (collision() == 1) {
            gameover();
            return;
        }

        usleep(50000);  // Sleep for 50ms
        eraseCar();
        eraseEnemy(0);
        eraseEnemy(1);

        if (enemyY[0] == 10)
            if (enemyFlag[1] == 0)
                enemyFlag[1] = 1;

        if (enemyFlag[0] == 1)
            enemyY[0] += 1;

        if (enemyFlag[1] == 1)
            enemyY[1] += 1;

        if (enemyY[0] > SCREEN_HEIGHT - 4) {
            resetEnemy(0);
            score++;
            updateScore();
        }
        if (enemyY[1] > SCREEN_HEIGHT - 4) {
            resetEnemy(1);
            score++;
            updateScore();
        }
    }
}

int main() {
    srand((unsigned)time(NULL));

    do {
        clear_screen();
        gotoxy(10, 5); cout << " -------------------------- ";
        gotoxy(10, 6); cout << " |        Car Game        | ";
        gotoxy(10, 7); cout << " --------------------------";

        gotoxy(10, 9); cout << "1. Start Game";
        gotoxy(10, 10); cout << "2. Instructions";
        gotoxy(10, 11); cout << "3. Quit";

        gotoxy(10, 13); cout << "Select option: ";
        char op = getch();

        if (op == '1') play();
        else if (op == '2') cout << "'a' for left \n'd' for right ";
        else if (op == '3') exit(0);

    } while (true);

    return 0;
}
