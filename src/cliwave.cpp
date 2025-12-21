#include "cliwave.hpp"

void showNewSessionScreen();
void showEditSessionScreen(char* sessionName, char* sessionLength, char* bufferLength);
void showDAWInterface(char* sessionName, char* sessionLength, char* bufferLength);

void showMainMenu() {
    std::vector<std::string> options = {
        "Start New Session",
        "Load Session",
        "Settings",
        "Exit"
    };

    int choice = 0;
    int ch;
    int num_options = options.size();

    clear();
    printw("Welcome to CLIWave!\n");
    printw("Use arrow keys to navigate. Press Enter to select.\n\n");
    for (int i = 0; i < num_options; ++i) {
        if (i == choice) attron(A_REVERSE);
        printw("[%d] %s\n", i + 1, options[i].c_str());
        if (i == choice) attroff(A_REVERSE);
    }
    refresh();

    while (true) {
        ch = getch();
        int prev_choice = choice;

        switch(ch) {
            case KEY_UP:
                choice = (choice - 1 + num_options) % num_options;
                break;
            case KEY_DOWN:
                choice = (choice + 1) % num_options;
                break;
            case '\n':
                if (options[choice] == "Start New Session") {
                    showNewSessionScreen();
                    clear();
                    printw("Welcome to CLIWave!\n");
                    printw("Use arrow keys to navigate. Press Enter to select.\n\n");
                    for (int i = 0; i < num_options; ++i) {
                        if (i == choice) attron(A_REVERSE);
                        printw("[%d] %s\n", i + 1, options[i].c_str());
                        if (i == choice) attroff(A_REVERSE);
                    }
                    refresh();
                    continue;
                }
                if (options[choice] == "Exit") {
                    printw("\nExiting...\n");
                    refresh(); 
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                    endwin();
                    exit(0);
                }
                break;
        }

        if (prev_choice != choice) {
            move(prev_choice + 3, 0); 
            attron(A_NORMAL);
            printw("[%d] %s", prev_choice + 1, options[prev_choice].c_str());
            attroff(A_NORMAL);

            move(choice + 3, 0);
            attron(A_REVERSE);
            printw("[%d] %s", choice + 1, options[choice].c_str());
            attroff(A_REVERSE);

            refresh();
        }
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    showMainMenu();

    endwin();
    return 0;
}

