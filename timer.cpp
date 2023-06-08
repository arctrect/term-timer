#include <iostream>
#include <csignal>
#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <regex>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

class Timer {
private:
    int countdownLength;
    std::chrono::steady_clock::time_point startTime;
    bool shouldExit;

public:
    Timer() : shouldExit(false) {}

    void start() {
        initscr();
        signal(SIGTERM, signalHandler);

        countdownLength = getTimeInput("Enter time (HH:MM:SS): ");
        startTime = std::chrono::steady_clock::now();
        displayTimer();

        endwin();
    }

private:
    static void signalHandler(int signal) {
        // Handle signal, if needed
    }

    void displayTimer() {
        bool endtimer = false;
        curs_set(0);

        int elapsedSeconds = 0;
        bool flashBackground = false; // Flag to indicate if the background should flash

        while (!shouldExit) {
            auto currentTime = std::chrono::steady_clock::now();
            elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

            std::string timeRemaining = getRemainingTime(elapsedSeconds);

            clear();

            // Flash the background every second when countdown finishes
            if (timeRemaining == "Countdown finished") {
                if (flashBackground) {
                    attron(A_REVERSE); // Reverse video (flashing background)
                }
                flashBackground = !flashBackground; // Toggle the flag
            }

            box(stdscr, 0, 0);
            centerText(timeRemaining, LINES, "other");
            centerText("Timer", LINES, "top");
            refresh();

            if (timeRemaining == "Countdown finished") {
                endtimer = true;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            if (endtimer && kbhit()) {
                // Stop flashing and exit the loop when a key is pressed
                attroff(A_REVERSE);
                shouldExit = true;
            }
        }

        if (endtimer) {
            clear();
            box(stdscr, 0, 0);
            centerText("Countdown finished. Press any key to exit.", LINES, "other");
            refresh();
            getch();
            shouldExit = true; // Set shouldExit to true to exit the main loop
        }

        curs_set(1);
    }

    std::string getRemainingTime(int elapsedSeconds) {
        auto remainingTime = countdownLength - elapsedSeconds;

        if (remainingTime <= 0) {
            return "Countdown finished";
        }

        int remainingHour = remainingTime / 3600;
        int remainingMinute = (remainingTime % 3600) / 60;
        int remainingSecond = remainingTime % 60;

        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << remainingHour << ":"
            << std::setfill('0') << std::setw(2) << remainingMinute << ":"
            << std::setfill('0') << std::setw(2) << remainingSecond;

        return oss.str();
    }

    void centerText(const std::string& text, int terminalHeight, std::string position) {
        int padding;

        if (position == "top") {
            padding = 0;
        } else {
            padding = (LINES - 1) / 2;
        }

        int row = padding;
        int col = (COLS - text.length()) / 2;
        mvprintw(row, col, "%s", text.c_str());
    }

    int getTimeInput(const std::string& prompt) {
        char input[256];
        int returnSeconds = 0;

        while (true) {
            curs_set(1);
            clear();
            box(stdscr, 0, 0);

            centerText(prompt, LINES, "top");
            move((LINES / 2) - 1, COLS / 2);
            refresh();

            flushinp();
            getstr(input);

            std::regex regPattern(R"((\d+):(\d+):(\d+))");
            std::string inputStr(input);
            std::smatch matches;

            if (std::regex_match(inputStr, matches, regPattern)) {
                int hours = std::stoi(matches[1]);
                int minutes = std::stoi(matches[2]);
                int seconds = std::stoi(matches[3]);

                if (hours >= 0 && minutes >= 0 && seconds >= 0) {
                    returnSeconds = (hours * 3600) + (minutes * 60) + seconds;
                    break;
                }
            }

            mvprintw(LINES, 0, "Invalid input");
            curs_set(0);
            refresh();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        curs_set(0);
        return returnSeconds;
    }

    int kbhit() {
        struct timeval tv;
        fd_set fds;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds); // STDIN_FILENO is the file descriptor for standard input
        select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &fds);
    }
};

int main() {
    Timer timer;
    timer.start();

    return 0;
}
