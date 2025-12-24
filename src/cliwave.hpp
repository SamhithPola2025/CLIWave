// doing it this way for organization purposes, realistically this file doesnt need to exist.
#pragma once

#include <ncurses.h>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

void showMainMenu();
void showNewSessionScreen();
void showEditSessionScreen(char* sessionName, char* sessionLength, char* bufferLength);
void showDAWInterface(char* sessionName, char* sessionLength, char* bufferLength);