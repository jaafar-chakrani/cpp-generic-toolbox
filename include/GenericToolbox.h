//
// Created by Nadrino on 28/08/2020.
//

#pragma once
#ifndef CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_H
#define CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_H

#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <functional>
#include <chrono>
#include <map>

// Index
namespace GenericToolbox{

  namespace Parameters{
    static int _verboseLevel_ = 0;
  }

  // Parameters for the progress bar
  namespace ProgressBar{

#ifndef PROGRESS_BAR_ENABLE_RAINBOW
#define PROGRESS_BAR_ENABLE_RAINBOW 0
#endif

#ifndef PROGRESS_BAR_LENGTH
#define PROGRESS_BAR_LENGTH 36
#endif

#ifndef PROGRESS_BAR_SHOW_SPEED
#define PROGRESS_BAR_SHOW_SPEED 1
#endif

#ifndef PROGRESS_BAR_REFRESH_DURATION_IN_MS
// 33 ms per frame = 0.033 seconds per frame = 1/30 sec per frame = 30 fps
#define PROGRESS_BAR_REFRESH_DURATION_IN_MS 33
#endif

#ifndef PROGRESS_BAR_FILL_TAG
// multi-char is possible
#define PROGRESS_BAR_FILL_TAG "#"
#endif

    static bool enableRainbowProgressBar = PROGRESS_BAR_ENABLE_RAINBOW;
    static bool displaySpeed = PROGRESS_BAR_SHOW_SPEED;
    static int barLength = PROGRESS_BAR_LENGTH;
    static int refreshRateInMilliSec = PROGRESS_BAR_REFRESH_DURATION_IN_MS;
    static std::string fillTag = PROGRESS_BAR_FILL_TAG;

    static int lastDisplayedPercentValue = -1;
    static int lastDisplayedValue = -1;
    static auto lastDisplayedTimePoint        = std::chrono::high_resolution_clock::now();
    static std::thread::id _selectedThreadId_ = std::this_thread::get_id(); // get the main thread id
    static std::vector<std::string> rainbowColorList = {"\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m", "\033[1;35m", "\033[1;36m"};

  }

  //! Displaying Tools
  inline void displayProgressBar(int iCurrent_, int iTotal_, std::string title_ = "", bool forcePrint_ = false);
  inline std::string parseIntAsString(int intToFormat_);
  template <typename T> inline std::string parseVectorAsString(const std::vector<T>& vector_);
  template <typename T> inline void printVector(const std::vector<T>& vector_);


  //! Vector management
  template <typename T> inline bool doesElementIsInVector( T element_, const std::vector<T>& vector_ );
  template <typename T, typename TT> inline std::vector<TT> convertVectorType( const std::vector<T>& vector_, std::function<TT(T)>& convertTypeFunction_ );
  inline bool doesElementIsInVector(const char* element_, const std::vector<std::string>& vector_);
  template <typename T> inline size_t findElementIndex( T element_, const std::vector<T>& vector_ );
  inline size_t findElementIndex( const char* element_, const std::vector<std::string>& vector_ );
  template <typename T> inline std::vector<size_t> getSortPermutation(const std::vector<T>& vectorToSort_, std::function<bool(const T, const T)> compareLambda_ );
  template <typename T> inline std::vector<T> applyPermutation(const std::vector<T>& vectorToPermute_, const std::vector<std::size_t>& sortPermutation_ );


  //! Map management
  template <typename K, typename T> inline bool doesKeyIsInMap( K key_, const std::map<K,T>& map_ );
  template <typename T1, typename T2> inline void appendToMap(std::map<T1, T2> &mapContainer_, const std::map<T1, T2> &mapToPushBack_, bool overwrite_ = true);
  template <typename T> inline std::map<std::string, T> getSubMap(const std::map<std::string, T>& map_, std::string keyStrStartWith_ );


  //! String Management Tools
  // -- Transformations
  inline bool doesStringContainsSubstring(std::string string_, std::string substring_, bool ignoreCase_ = false);
  inline bool doesStringStartsWithSubstring(std::string string_, std::string substring_, bool ignoreCase_ = false);
  inline bool doesStringEndsWithSubstring(std::string string_, std::string substring_, bool ignoreCase_ = false);
  inline std::string toLowerCase(const std::string &inputStr_);
  inline std::string stripStringUnicode(const std::string &inputStr_);
  inline std::string repeatString(std::string inputStr_, int amount_);
  inline std::string removeRepeatedCharacters(const std::string& inputStr_, std::string repeatedChar_);
  inline std::string joinVectorString(const std::vector<std::string> &string_list_, std::string delimiter_, int begin_index_ = 0, int end_index_ = 0);
  inline std::string replaceSubstringInString(const std::string &input_str_, std::string substr_to_look_for_, std::string substr_to_replace_);
  inline std::vector<std::string> splitString(const std::string& inputString_, std::string delimiter_);
  // -- Parsing
  inline std::string parseSizeUnits(unsigned int sizeInBytes_);
  template<typename ... Args> inline std::string formatString( std::string format, Args ... args );


  //! Conversion Tools
  inline bool toBool(std::string str);


  //! FS Tools
  // -- without IO dependencies
  inline bool doesFilePathHasExtension(const std::string &filePath_, std::string ext_);
  inline std::string getFolderPathFromFilePath(const std::string &filePath_);
  // -- with direct IO dependencies
  inline bool doesPathIsFile(std::string filePath_);
  inline bool doesPathIsFolder(std::string folderPath_);
  inline bool doFilesAreTheSame(std::string filePath1_, std::string filePath2_);
  inline bool mkdirPath(std::string newFolderPath_);
  inline bool deleteFile(std::string filePath_);
  inline bool copyFile(std::string source_file_path_, std::string destination_file_path_, bool force_ = false);
  inline bool mvFile(std::string sourceFilePath_, std::string destinationFilePath_, bool force_ = false);
  inline size_t getHashFile(std::string filePath_);
  inline long int getFileSizeInBytes(const std::string &filePath_);
  inline std::string getCurrentWorkingDirectory();
  inline void dumpStringInFile(std::string outFilePath_, std::string stringToWrite_);
  inline std::string dumpFileAsString(std::string filePath_);
  inline std::vector<std::string> dumpFileAsVectorString(std::string filePath_);
  inline std::vector<std::string> getListOfEntriesInFolder(std::string folderPath_, std::string entryNameRegex_ = "");
  inline std::vector<std::string> getListOfSubfoldersInFolder(std::string folderPath_, std::string entryNameRegex_ = "");
  inline std::vector<std::string> getListOfFilesInFolder(std::string folderPath_, std::string entryNameRegex_ = "");
  // -- with indirect IO dependencies
  inline bool doesFolderIsEmpty(std::string folderPath_);
  inline std::vector<std::string> getListFilesInSubfolders(const std::string &folderPath_);


  //! Hardware Tools
  inline size_t getProcessMemoryUsage();
  inline size_t getProcessMaxMemoryUsage();
  inline int getTerminalWidth();
  inline int getTerminalHeight();
  inline std::string getElapsedTimeSinceLastCallStr(int instance_ = -1);
  inline long long getElapsedTimeSinceLastCallInMicroSeconds(int instance = -1);



  //! Misc Tools
  inline std::string getClassName(const std::string& PRETTY_FUNCTION__); // When calling this functions, provide __PRETTY_FUNCTION__ macro
#define __CLASS_NAME__ GenericToolbox::getClassName(__PRETTY_FUNCTION__)
  inline std::string getMethodName(const std::string& PRETTY_FUNCTION__);
#define __METHOD_NAME__ GenericToolbox::getMethodName(__PRETTY_FUNCTION__)

#define GET_VAR_NAME_VALUE(var) ( ((std::stringstream&) (std::stringstream() << #var << " = " << (var)) ).str() )
#define GET_VAR_NAME(var) std::string(#var)

  // Not intended to be managed by the user
  namespace Internals{

    static std::map<int, std::chrono::high_resolution_clock::time_point> _lastTimePointMap_;

  }

}

#include "GenericToolbox.impl.h"

#endif //CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_H
