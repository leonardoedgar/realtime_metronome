#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define BILLION 1000000000L
#define NEARLY_ZERO 1e-6

#define QNX_ID 1
#define QNX_TERMINAL_WIDTH 100
#define UBUNTU_ID 2
#define UBUNTU_TERMINAL_WIDTH 195
#define PLATFORM_ID QNX_ID

#if PLATFORM_ID == QNX_ID
#define TERMINAL_WIDTH QNX_TERMINAL_WIDTH
#else
#define TERMINAL_WIDTH UBUNTU_TERMINAL_WIDTH
#endif

/**
 * A data type that contains the metronome setting.
 */
typedef struct {
    int modeNum;
    int frequency;
} setting;

/**
 * A data type that represents a frequency limit.
 */
typedef struct {
    int min;
    int max;
} freqLimit;

/**
 * A function to read the keyboard arrow input from the user.
 * @return the int representation of the keyboard's key pressed
 */
int ReadArrow();

/**
 * A function to get the frequency limit of a metronome's mode.
 * @return the frequency limit
 */
freqLimit GetFreqLimit(int modeNum);

/**
 * A function to get the frequency to set to the metronome from the user.
 * @param metronomeSetting represents the metronome setting
 */
void GetFrequency(setting* metronomeSetting);

/**
 * A function to adjust the frequency of the metronome.
 * @param metronomeSetting represents the setting of the metronome
 * @return the new frequency of the metronome
 */
int AdjustFreq(setting metronomeSetting);

/**
 * A function to print all available tempo in the metronome.
 */
void PrintTempo();

/**
 * A function to spawn the user input thread.
 * @param arg contains the argument to be passed to the thread
 * @return NULL
 */
void* SpawnUserInputThread(void* defaultSetting);

/**
 * A function to spawn the audio thread.
 * @param arg contains the argument to be passed to the thread
 * @return NULL
 */
void* SpawnAudioThread(void* arg);

/**
 * A function to handle the Ctrl-C signal.
 * @param sigNum represents the Ctrl-C signal number
 */
void CtrlCHandler(int sigNum);

/**
 * A function to print the audio signal.
 * @param sigNum represents the signal number that calls this function
 */
void PrintAudio(int sigNum);

/**
 * A function to update the timer.
 * @param frequency indicates the new frequency to be set
 * @return -1 if failed to update the timer, otherwise success
 */
int UpdateTimer(int frequency);

/**
 * A function to print instructions for the user to adjust the metronome's frequency.
 * @param mode indicates the current mode of the metronome
 * @param frequency indicates the current frequency of the metronome
 * @param warningMessage indicates the warning message to be shown to the user
 */
void PrintAdjustFreqInstructions(int mode, int frequency, const char* warningMessage);

/**
 * A function to terminate the whole program in a safe manner.
 */
void TerminateProgram();

/**
 * A function to parse the command-line arguments.
 * @param argc represents the number of arguments
 * @param argv represents contains an array of arguments
 * @param filePath represents the file path passed as a argument
 */
void ParseArgs(int argc, char** argv, char** filePath);

/**
 * A function to parse the config file.
 * @param filePath indicates the path to the config file
 * @param metronomeSetting indicates the metronome setting written in the file
 */
void ParseConfigFile(char* filePath, setting* metronomeSetting);

/**
 * A function to check if a metronome mode number is valid.
 * @param modeNum indicates the mode number
 * @return true if valid, otherwise false
 */
bool IsMetronomeModeNumValid(int modeNum);

/**
 * A function to check if a metronome frequency valid within its mode.
 * @param modeNum indicates the mode number
 * @param frequency indicates the frequency
 * @return true if valid, otherwise false
 */
bool IsMetronomeFrequencyValid(int modeNum, int frequency);

/**
 * A function to save the metronome setting to a file.
 * @param filePath indicates the path to the file to save the settting to
 * @param metronomeSetting indicates the metronome setting to be saved
 * @return true for a successful saving, otherwise false
 */
bool SaveMetronomeSetting(char* filePath, setting* metronomeSetting);

/**
 * A function to get the default beats per minute based on the mode of the metronome.
 * @param modeNum indicates the mode number
 * @return the beats per minute
 */
int GetBPM(int modeNum);

/**
 * A function to get the tempo of the metronome.
 * @param modeNum indicates the mode number of the metronome
 * @param tempo indicates the tempo of the metronome
 */
void GetTempo(int modeNum, char** tempo);

/**
 * A function to check if a given string is a numerical value
 * @param str indicates the string to check
 * @return true if it is a numerical value, otherwise false
 */
bool IsNumeric(char* str);
