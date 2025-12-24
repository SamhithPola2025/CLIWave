#include "cliwave.hpp"
#include "audiomanager.h"

void showNewSessionScreen() {
    clear();
    printw("===== New Session =====\n\n");
    refresh();
    
    char sessionName[100];
    char sessionLength[100];
    char bufferLength[100];
    
    printw("Enter session name: ");
    refresh();
    echo();
    getstr(sessionName);
    noecho();
    
    printw("\nEnter session length (seconds): ");
    refresh();
    echo();
    getstr(sessionLength);
    noecho();
    
    printw("\nEnter buffer length (samples): ");
    refresh();
    echo();
    getstr(bufferLength);
    noecho();

    std::string line(30, '=');
    printw("\n%s\n", line.c_str());
    printw("Session Name: %s\n", sessionName);
    printw("Session Length: %s seconds\n", sessionLength);
    printw("Buffer Length: %s samples\n", bufferLength);
    printw("%s\n", line.c_str());
    
    printw("\nPress Enter to start DAW, '[' to edit settings, or any other key to return...\n");
    
    refresh();
    int key = getch();
    
    if (key == '[') {
        showEditSessionScreen(sessionName, sessionLength, bufferLength);
    } else if (key == '\n') {
        showDAWInterface(sessionName, sessionLength, bufferLength);
    }
}

void showEditSessionScreen(char* sessionName, char* sessionLength, char* bufferLength) {
    clear();
    printw("===== Edit Session Settings =====\n\n");
    
    printw("Current Settings:\n");
    printw("1. Session Name: %s\n", sessionName);
    printw("2. Session Length: %s seconds\n", sessionLength);
    printw("3. Buffer Length: %s samples\n\n", bufferLength);
    
    printw("Which setting would you like to edit? (1-3): ");
    refresh();
    
    char choice[10];
    echo();
    getstr(choice);
    noecho();
    
    if (choice[0] == '1') {
        printw("\nEnter new session name: ");
        refresh();
        echo();
        getstr(sessionName);
        noecho();
    } else if (choice[0] == '2') {
        printw("\nEnter new session length (seconds): ");
        refresh();
        echo();
        getstr(sessionLength);
        noecho();
    } else if (choice[0] == '3') {
        printw("\nEnter new buffer length (samples): ");
        refresh();
        echo();
        getstr(bufferLength);
        noecho();
    }
    
    printw("\nSettings updated! Press any key to continue...\n");
    refresh();
    getch();
}

void showDAWInterface(char* sessionName, char* sessionLength, char* bufferLength) {
    int numTracks = 4;
    int timelinePos = 0;
    int selectedTrack = 0;
    bool isPlaying = false;
    bool isRecording = false;
    int takeCounter = 1;
    int maxTime = atoi(sessionLength);
    int timelineWidth = maxTime * 5;
    
    nodelay(stdscr, TRUE);
    
    while (true) {
        clear();
        std::string headerLine(60, '=');
        printw("%s\n", headerLine.c_str());
        printw("CLIWave DAW - Session: %s | Length: %ss | Buffer: %s\n", 
               sessionName, sessionLength, bufferLength);
        printw("%s\n", headerLine.c_str());
        
        printw("\nPlayback: ");
        if (isPlaying) {
            attron(A_BOLD);
            printw("[PLAYING] ");
            attroff(A_BOLD);
        } else {
            printw("[STOPPED] ");
        }
        if (isRecording) {
            attron(A_BOLD | COLOR_PAIR(1));
            printw("[REC]");
            attroff(A_BOLD | COLOR_PAIR(1));
        }
        printw("Time: %f", float(timelinePos)/5, "s \n");
        
        printw("\nSeconds:  |");
        for (int s = 0; s < maxTime; s++) {
            if (s % 10 == 0) {
                printw("%d", s);
            } else {
                printw(" ");
            }
            for (int i = 1; i < 5; i++) {
                printw(" ");
            }
        }
        printw("|\n");
        
        printw("Timeline: |");
        for (int i = 0; i < timelineWidth; i++) {
            if (i == timelinePos) {
                attron(A_REVERSE);
                printw(">");
                attroff(A_REVERSE);
            } else if (i % 5 == 0) {
                printw("|");
            } else {
                printw("-");
            }
        }
        printw("|\n\n");
        
        printw("Tracks:\n");
        std::string trackLine(timelineWidth + 2, '-');
        for (int i = 0; i < numTracks; i++) {
            if (i == selectedTrack) attron(A_REVERSE);
            printw("[Track %d] ", i + 1);
            if (i == selectedTrack) attroff(A_REVERSE);
            
            printw("|");
            for (int j = 0; j < timelineWidth; j++) {
                printw(" ");
            }
            printw("|\n");
        }
        
        printw("\n%s\n", trackLine.c_str());
        
        // Controls
        printw("\nControls:\n");
        printw("  Space   - Play/Pause\n");
        printw("  R       - Record\n");
        printw("  S       - Stop\n");
        printw("  Up/Down - Select track\n");
        printw("  Left/Right - Move timeline\n");
        printw("  M       - Mute selected track\n");
        printw("  +/-     - Add/Remove track\n");
        printw("  Q       - Quit to menu\n");
        
        refresh();
        
        int ch = getch();
        
        if (ch != ERR) {
            switch(ch) {
                case ' ':
                    isPlaying = !isPlaying;
                    break;
                case 'r':
                case 'R':
                    if (!isRecording) {
                        // Build output filename
                        std::string fname = std::string("recordings/") + std::string(sessionName) +
                                            "_track" + std::to_string(selectedTrack + 1) +
                                            "_take" + std::to_string(takeCounter) + ".wav";
                        // Start recording: 16-bit PCM, stereo, 44100 Hz
                        ma_result res = start_recording(fname.c_str(), ma_format_s16, 2, 44100);
                        if (res == MA_SUCCESS) {
                            isRecording = true;
                            takeCounter++;
                            // Show brief status
                            move(0, 0);
                            printw("Recording -> %s\n", fname.c_str());
                        } else {
                            move(0, 0);
                            printw("Recording failed (code %d)\n", res);
                        }
                    }
                    break;
                case 's':
                case 'S':
                    isPlaying = false;
                    if (isRecording) {
                        stop_recording();
                        isRecording = false;
                    }
                    timelinePos = 0;
                    break;
                case KEY_UP:
                    if (selectedTrack > 0) selectedTrack--;
                    break;
                case KEY_DOWN:
                    if (selectedTrack < numTracks - 1) selectedTrack++;
                    break;
                case KEY_LEFT:
                    if (timelinePos > 0) timelinePos--;
                    break;
                case KEY_RIGHT:
                    if (timelinePos < timelineWidth - 1) timelinePos++;
                    break;
                case '+':
                    if (numTracks < 8) numTracks++;
                    break;
                case '-':
                    if (numTracks > 1) numTracks--;
                    if (selectedTrack >= numTracks) selectedTrack = numTracks - 1;
                    break;
                case 'q':
                case 'Q':
                    if (isRecording) {
                        stop_recording();
                        isRecording = false;
                    }
                    nodelay(stdscr, FALSE);
                    return;
            }
        }
        
        if (isPlaying) {
            napms(200);
            if (timelinePos < timelineWidth - 1) {
                timelinePos++;
            } else {
                isPlaying = false;
            }
        } else {
            napms(50);
        }
    }
}