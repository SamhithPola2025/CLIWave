#include "cliwave.hpp"
#include "audiomanager.h"

// other includes (im trying to keep it relatively minimal)
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector> // surprised I didn't have a need for this earlier
#include <cmath> // for fabs

class Segment {public: int startPos; int length; std::string filename;};

static bool mixdownAllTracks(const std::vector<std::vector<Segment>>& trackSegments, int maxTimeSeconds, const std::string& exportPath);

// helpers:
static bool ensureDir(const std::string& path) {
    struct stat st = {0};
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
    return mkdir(path.c_str(), 0755) == 0;
}

static std::string joinPath(const std::string& dir, const std::string& file) {
    if (dir.empty()) return file;
    if (dir.back() == '/') return dir + file;
    return dir + "/" + file;
}

// helper that i'll use later on
static std::string baseName(const std::string& path) {
    size_t pos = path.find_last_of('/');
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

// helper that i'll use later on
static bool copyFile(const std::string& src, const std::string& dst) { 
    // let src be the existing file that i'm copying from.
    // let dst be the destination of the new file path im copying to.
    std::ifstream in(src, std::ios::binary);
    std::ofstream out(dst, std::ios::binary);
    out << in.rdbuf();
    return in.good() && out.good();
}

void showNewSessionScreen() {
    clear();
    printw("===== New Session =====\n\n");
    refresh();
    
    char sessionName[100];
    char sessionLength[100];
    char bufferLength[100];
    char recordDir[200];
    char exportDir[200];
    
    printw("Enter session name: ");
    refresh(); echo(); getstr(sessionName); noecho();
    
    printw("\nEnter session length (seconds): ");
    refresh(); echo(); getstr(sessionLength); noecho();
    
    printw("\nEnter buffer length (samples): ");
    refresh(); echo(); getstr(bufferLength); noecho();

    printw("\nEnter recording directory (e.g., recordings): ");
    refresh(); echo(); getstr(recordDir); noecho();

    printw("\nEnter export directory (e.g., exports): ");
    refresh(); echo(); getstr(exportDir); noecho();

    std::string line(30, '=');
    printw("\n%s\n", line.c_str());
    printw("Session Name: %s\n", sessionName);
    printw("Session Length: %s seconds\n", sessionLength);
    printw("Buffer Length: %s samples\n", bufferLength);
    printw("Recording Dir: %s\n", recordDir);
    printw("Exporting Dir: %s\n", exportDir);
    printw("%s\n", line.c_str());
    
    printw("\nPress Enter to start DAW, '[' to edit settings, or any other key to return...\n");
    
    refresh();
    int key = getch();
    
    if (key == '[') {
        showEditSessionScreen(sessionName, sessionLength, bufferLength, recordDir, exportDir);
    } else if (key == '\n') {
        showDAWInterface(sessionName, sessionLength, bufferLength, recordDir, exportDir);
    }
}

void showEditSessionScreen(char* sessionName, char* sessionLength, char* bufferLength, char* recordDir, char* exportDir) {
    clear();
    printw("===== Edit Session Settings =====\n\n");
    
    printw("Current Settings:\n");
    printw("1. Session Name: %s\n", sessionName);
    printw("2. Session Length: %s seconds\n", sessionLength);
    printw("3. Buffer Length: %s samples\n", bufferLength);
    printw("4. Recording Dir: %s\n", recordDir);
    printw("5. Export Dir: %s\n", exportDir);
    
    printw("Which setting would you like to edit? (1-5): ");
    refresh();
    
    char choice[10];
    echo();
    getstr(choice);
    noecho();
    
    if (choice[0] == '1') {
        printw("\nEnter new session name: ");
        refresh(); echo(); getstr(sessionName); noecho();
    } else if (choice[0] == '2') {
        printw("\nEnter new session length (seconds): ");
        refresh(); echo(); getstr(sessionLength); noecho();
    } else if (choice[0] == '3') {
        printw("\nEnter new buffer length (samples): ");
        refresh(); echo(); getstr(bufferLength); noecho();
    } else if (choice[0] == '4') {
        printw("\nEnter new recording directory (e.g., recordings): ");
        refresh(); echo(); getstr(recordDir); noecho();
    } else if (choice[0] == '5') {
        printw("\nEnter new export directory (e.g., exports): ");
        refresh(); echo(); getstr(exportDir); noecho();
    }

    printw("\nSettings updated! Press any key to continue...\n");
    refresh();
    getch();
}

void showDAWInterface(char* sessionName, char* sessionLength, char* bufferLength, char* recordDir, char* exportDir) {
    int numTracks = 4;
    int timelinePos = 0;
    int selectedTrack = 0;
    bool isPlaying = false;
    bool isRecording = false;
    int takeCounter = 1;
    int maxTime = atoi(sessionLength);
    int timelineWidth = maxTime * 5;

    std::vector<std::vector<Segment>> trackSegments(numTracks);

    int recStartPos = -1;
    int recTrackIndex = -1;
    std::string recFile;
    int lastPlayedPos = -1;

    nodelay(stdscr, TRUE);

    std::vector<std::vector<char>> trackData(numTracks, std::vector<char>(timelineWidth, ' '));

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
        printw("Time: %.1f s\n", float(timelinePos)/5);
        
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
                printw("%c", trackData[i][j]);
            }
            printw("|\n");
        }
        
        printw("\n%s\n", trackLine.c_str());
        
        printw("\nControls:\n");
        printw("  Space   - Play/Pause\n");
        printw("  R       - Record\n");
        printw("  S       - Stop\n");
        printw("  Up/Down - Select track\n");
        printw("  Left/Right - Move timeline\n");
        printw("  M       - Mute selected track\n");
        printw("  E       - Export mixdown\n");
        printw("  +/-     - Add/Remove track\n");
        printw("  Q       - Quit to menu\n");
        
        refresh();
        
        int ch = getch();
        
        if (ch != ERR) {
            switch(ch) {
                case ' ':
                    isPlaying = !isPlaying;
                    if (!isPlaying) {
                        stop_playback();
                    }
                    break;
                case 'r':
                case 'R':
                    if (!isRecording) {
                        ensureDir(recordDir);
                        std::string fname = joinPath(recordDir, std::string(sessionName) +
                                            "_track" + std::to_string(selectedTrack + 1) +
                                            "_take" + std::to_string(takeCounter) + ".wav");
                        ma_result res = start_recording(fname.c_str(), ma_format_s16, 2, 44100);
                        if (res == MA_SUCCESS) {
                            isRecording = true;
                            takeCounter++;
                            recStartPos = timelinePos;
                            recTrackIndex = selectedTrack;
                            recFile = fname;
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
                    stop_playback();
                    if (isRecording) {
                        stop_recording();
                        isRecording = false;
                        if (recStartPos >= 0 && recTrackIndex >= 0) {
                            Segment seg{recStartPos, timelinePos - recStartPos, recFile};
                            trackSegments[recTrackIndex].push_back(seg);
                            recStartPos = -1;
                            recTrackIndex = -1;
                            recFile.clear();
                        }
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
                    if (numTracks < 8) {
                        numTracks++;
                        trackData.push_back(std::vector<char>(timelineWidth, ' '));
                        trackSegments.push_back(std::vector<Segment>());
                    }
                    break;
                case '-':
                    if (numTracks > 1) {
                        numTracks--;
                        if (!trackData.empty()) trackData.pop_back();
                        if (!trackSegments.empty()) trackSegments.pop_back();
                        if (selectedTrack >= numTracks) selectedTrack = numTracks - 1;
                    }
                    break;
                case 'e':
                case 'E': {
                    move(0, 0);
                    printw("Exporting...\n");
                    refresh();
                    ensureDir(exportDir);
                    std::string outPath = joinPath(exportDir, std::string(sessionName) + "_mixdown.wav");
                    bool ok = mixdownAllTracks(trackSegments, maxTime, outPath);
                    if (ok) {
                        printw("Exported: %s\n", outPath.c_str());
                    } else {
                        printw("Export failed.\n");
                    }
                    break;
                }
                case 'q':
                case 'Q':
                    if (isRecording) {
                        stop_recording();
                        isRecording = false;
                        if (recStartPos >= 0 && recTrackIndex >= 0) {
                            Segment seg{recStartPos, timelinePos - recStartPos, recFile};
                            trackSegments[recTrackIndex].push_back(seg);
                        }
                    }
                    stop_playback();
                    nodelay(stdscr, FALSE);
                    return;
            }
        }
        
        // recording with timeline logic (WIP)

        if (isPlaying || isRecording) {
            napms(200);
            if (timelinePos < timelineWidth - 1) {
                if (isRecording) {
                    trackData[selectedTrack][timelinePos] = 'x';
                }
                if (isPlaying && lastPlayedPos != timelinePos) {
                    for (const auto& seg : trackSegments[selectedTrack]) {
                        if (seg.startPos == timelinePos) {
                            start_playback(seg.filename.c_str());
                            break;
                        }
                    }
                    lastPlayedPos = timelinePos;
                }
                timelinePos++;
            } else {
                if (isRecording) {
                    stop_recording();
                    isRecording = false;
                    if (recStartPos >= 0 && recTrackIndex >= 0) {
                        Segment seg{recStartPos, timelinePos - recStartPos, recFile};
                        trackSegments[recTrackIndex].push_back(seg);
                        recStartPos = -1;
                        recTrackIndex = -1;
                        recFile.clear();
                    }
                }
                stop_playback();
                isPlaying = false;
            }
        } else {
            napms(50);
        }
    }
}

static bool mixdownAllTracks(const std::vector<std::vector<Segment>>& trackSegments, int maxTimeSeconds, const std::string& exportPath) {

    // later I will make this modifiable, can't be very hard.
    const int sampleRate = 44100;
    const int channels = 2;
    const int ticksPerSecond = 5;
    const int totalFrames = maxTimeSeconds * sampleRate;

    std::vector<float> mix(channels * totalFrames, 0.0f);

    for (size_t t = 0; t < trackSegments.size(); ++t) {
        for (const auto& seg : trackSegments[t]) {
            // start offset in frames
            int startFrame = (seg.startPos * sampleRate) / ticksPerSecond;
            ma_decoder dec;
            if (ma_decoder_init_file(seg.filename.c_str(), nullptr, &dec) != MA_SUCCESS) continue;
            std::vector<float> buf(1024 * channels);
            ma_uint64 totalRead = 0;

            while (true) {
                ma_uint64 got = 0;
                ma_result r = ma_decoder_read_pcm_frames(&dec, buf.data(), 1024, &got);
                if (r != MA_SUCCESS || got == 0) break;
                for (ma_uint64 i = 0; i < got; ++i) {
                    int dstFrame = startFrame + static_cast<int>(i);
                    if (dstFrame >= totalFrames) break;
                    for (int c = 0; c < channels; ++c) {
                        size_t dstIdx = static_cast<size_t>(dstFrame) * channels + c;
                        mix[dstIdx] += buf[i * channels + c];
                    }
                }
                totalRead += got;
                if (totalRead >= seg.length * sampleRate / ticksPerSecond) break;
            }
            ma_decoder_uninit(&dec);
        }
    }
    float maxAbs = 0.0f;
    for (float v : mix) maxAbs = std::max(maxAbs, std::fabs(v));
    float scale = (maxAbs > 1.0f) ? (1.0f / maxAbs) : 1.0f;

    ma_encoder_config encCfg = ma_encoder_config_init(ma_encoding_format_wav, ma_format_s16, channels, sampleRate);
    ma_encoder enc;
    if (ma_encoder_init_file(exportPath.c_str(), &encCfg, &enc) != MA_SUCCESS) {
        return false;
    }

    std::vector<int16_t> outBlock(1024 * channels);
    size_t frame = 0;
    while (frame < static_cast<size_t>(totalFrames)) {
        size_t chunk = std::min(outBlock.size() / channels, static_cast<size_t>(totalFrames) - frame);
        for (size_t i = 0; i < chunk; ++i) {
            for (int c = 0; c < channels; ++c) {
                float v = mix[(frame + i)*channels + c] * scale;
                v = std::max(-1.0f, std::min(1.0f, v));
                outBlock[i * channels + c] = static_cast<int16_t>(v * 32767.0f);
            }
        }
        ma_encoder_write_pcm_frames(&enc, outBlock.data(), chunk, nullptr);
        frame += chunk;
    }
    ma_encoder_uninit(&enc);
    return true;
}