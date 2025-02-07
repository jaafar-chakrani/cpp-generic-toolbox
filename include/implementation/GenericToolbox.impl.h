//
// Created by Nadrino on 28/08/2020.
//

#pragma once
#ifndef CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_IMPL_H
#define CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_IMPL_H

#include <utility>
#include <cmath>
#include <sys/stat.h>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <thread>
#include <numeric>
#include <regex>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "vector"

extern char* __progname;


// Displaying Tools
namespace GenericToolbox {

  namespace ProgressBar {

    // Parameters for the progress bar
    static bool debugMode = false;
    static bool enableRainbowProgressBar = PROGRESS_BAR_ENABLE_RAINBOW;
    static bool displaySpeed = PROGRESS_BAR_SHOW_SPEED;
    static int maxBarLength = PROGRESS_BAR_LENGTH;
    static size_t refreshRateInMilliSec = PROGRESS_BAR_REFRESH_DURATION_IN_MS;
    static std::string fillTag(PROGRESS_BAR_FILL_TAG);
    static std::ostream* outputStreamPtr{&std::cout};

    static int lastDisplayedPercentValue{-1};
    static int lastDisplayedValue{-1};
    static double lastDisplayedSpeed{0};
    static auto lastDisplayedTimePoint = std::chrono::high_resolution_clock::now();
    static std::thread::id _selectedThreadId_ = std::this_thread::get_id(); // get the main thread id
    static std::vector<std::string> rainbowColorList{"\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m",
                                                     "\033[1;35m", "\033[1;36m"};
  }

  template<typename T, typename TT> inline std::string generateProgressBarStr( const T& iCurrent_, const TT& iTotal_, const std::string &title_ ){

    int percentValue = int(round(double(iCurrent_) / double(iTotal_) * 100.));
    if( percentValue > 100 ){
      percentValue = 100;
    }
    else if( percentValue < 0 ){
      percentValue = 0;
    }

    std::stringstream ssPrefix;
    if( not title_.empty() ) ssPrefix << title_ << " ";

    std::stringstream ssTail;
    ssTail << GenericToolbox::padString(std::to_string(percentValue), 3, ' ') << "%";

    auto newTimePoint = std::chrono::high_resolution_clock::now();
    if (ProgressBar::displaySpeed) {
      ssTail << " (";
      double itPerSec =
        double(iCurrent_) - ProgressBar::lastDisplayedValue; // nb iterations since last print
        double timeInterval;
        if (int(itPerSec) < 0) itPerSec = 0;
        else {
          timeInterval = double(std::chrono::duration_cast<std::chrono::milliseconds>(
            newTimePoint - ProgressBar::lastDisplayedTimePoint
            ).count()) / 1000.;
          if( timeInterval != 0 ){
            itPerSec /= timeInterval; // Count per s
            ProgressBar::lastDisplayedSpeed = itPerSec;
          }
          else itPerSec = ProgressBar::lastDisplayedSpeed;
        }
        ssTail << GenericToolbox::padString(GenericToolbox::parseIntAsString(int(itPerSec)), 5, ' ');
        ssTail << " it/s)";
    }



    // test if the bar is too wide wrt the prompt width
    int displayedBarLength = ProgressBar::maxBarLength;

    auto termWidth = GenericToolbox::getTerminalWidth();
    if( termWidth != 0 ) { // terminal width is measurable

      size_t totalBarLength = GenericToolbox::getPrintSize(ssPrefix.str());
      if(displayedBarLength > 0) {
        totalBarLength += 2; // []
        totalBarLength += displayedBarLength;
        totalBarLength += 1; // space before tail
      }
      totalBarLength += ssTail.str().size();
      totalBarLength += 1; // 1 extra space is necessary to std::endl

      int remainingSpaces = termWidth;
      remainingSpaces -= int(totalBarLength);

      if( remainingSpaces < 0 ){
        if( displayedBarLength >= 0 ){
          // ok, can take some extra space in the bar
          displayedBarLength -= std::abs(remainingSpaces);
          if (displayedBarLength < 12) {
            displayedBarLength = 0;
            remainingSpaces += 2; // get back the [] of the pBar
          }
          remainingSpaces += (ProgressBar::maxBarLength - displayedBarLength );
        }
      }

      // if it's still to big, cut the title
      if ( remainingSpaces < 0) {
        std::string cutPrefix = ssPrefix.str().substr(0, int(ssPrefix.str().size()) - std::abs(remainingSpaces) - 3);
        ssPrefix.str("");
        ssPrefix << cutPrefix;
        ssPrefix << "\033[0m" << "...";
      }
    }
    else {
      displayedBarLength = 0;
    }

    std::stringstream ssProgressBar;
    ssProgressBar << ssPrefix.str();

    if (displayedBarLength > 0) {
      int nbTags = percentValue * displayedBarLength / 100;
      int nbSpaces = displayedBarLength - nbTags;

      std::string padProgressBar;
      for (int iTag = 0; iTag < nbTags; iTag++) {
        padProgressBar += ProgressBar::fillTag[iTag%ProgressBar::fillTag.size()];
      }
      padProgressBar += repeatString(" ", nbSpaces);

      if( ProgressBar::enableRainbowProgressBar ){
        padProgressBar = GenericToolbox::makeRainbowString(padProgressBar, false);
      }

      ssProgressBar << "[" << padProgressBar << "] ";
    }

    ssProgressBar << ssTail.str();

    ssProgressBar << std::endl; // always jump line to force flush on screen
    if( percentValue != 100 ){
      // those commands won't be flushed until a new print is called:
      // pull back to cursor on the line of the progress bar
      ssProgressBar << static_cast<char>(27) << "[1;1F";
      // Clear the line and add "\r" since a Logger might intercept it to trigger a print of a line header
      ssProgressBar << static_cast<char>(27) << "[1K" << "\r"; // trick to clear
    }

    ProgressBar::lastDisplayedPercentValue = percentValue;
    ProgressBar::lastDisplayedValue = iCurrent_;
    ProgressBar::lastDisplayedTimePoint = newTimePoint;

    if( ProgressBar::debugMode ){
      std::cout << "New timestamp: " << ProgressBar::lastDisplayedTimePoint.time_since_epoch().count() << std::endl;
      std::cout << "ProgressBar::lastDisplayedValue: " << ProgressBar::lastDisplayedValue << std::endl;
      std::cout << "ProgressBar::lastDisplayedPercentValue: " << ProgressBar::lastDisplayedPercentValue << std::endl;
    }

    return ssProgressBar.str();

  }
  template<typename T, typename TT> inline bool showProgressBar(const T& iCurrent_, const TT& iTotal_){

    //    if( // Only the main thread
    //      ProgressBar::_selectedThreadId_ != std::this_thread::get_id()
    //      ){
    //      return false;
    //    }

    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now() - ProgressBar::lastDisplayedTimePoint
      ).count();
    if( // REQUIRED TO PRINTOUT
    iCurrent_ == 0 // First call
    or ProgressBar::lastDisplayedPercentValue == -1 // never printed before
    or timeDiff >= ProgressBar::refreshRateInMilliSec
    or iCurrent_ + 1 >= iTotal_ // last entry (mandatory to print at least once: need to print endl)
    ){

      int percent = int(round(double(iCurrent_) / double(iTotal_) * 100.));

      if( percent >= 100 ){ percent = 100; }
      else if( percent < 0) percent = 0;

      if( // EXCLUSION CASES
      percent == ProgressBar::lastDisplayedPercentValue // already printed
      ){
        if( ProgressBar::debugMode ){
          std::cout << "Print PBar NOT Ok:" << std::endl;
          std::cout << "percent == ProgressBar::lastDisplayedPercentValue" << std::endl;
        }
        return false;
      }

      if( ProgressBar::debugMode ){
        std::cout << "Print PBar Ok:" << std::endl;
        std::cout << "percent = " << percent << std::endl;
        std::cout << "iCurrent_ = " << iCurrent_ << std::endl;
        std::cout << "iTotal_ = " << iTotal_ << std::endl;
        std::cout << "ProgressBar::lastDisplayedPercentValue = " << ProgressBar::lastDisplayedPercentValue << std::endl;
        std::cout << "ProgressBar::refreshRateInMilliSec = " << ProgressBar::refreshRateInMilliSec << std::endl;
        std::cout << "timeDiff = " << timeDiff << std::endl;
      }

      // OK!
      return true;
    }

    return false;
  }
  template<typename T, typename TT> inline std::string getProgressBarStr(const T& iCurrent_, const TT& iTotal_, const std::string &title_, bool forcePrint_ ){
    if(forcePrint_ or showProgressBar(iCurrent_, iTotal_) ){
      return generateProgressBarStr(iCurrent_, iTotal_, title_);
    }
    return {};
  }
  template<typename T, typename TT> inline void displayProgressBar(const T& iCurrent_, const TT& iTotal_, const std::string &title_, bool forcePrint_) {
    if(forcePrint_ or GenericToolbox::showProgressBar(iCurrent_, iTotal_) ){
      *ProgressBar::outputStreamPtr << GenericToolbox::generateProgressBarStr(iCurrent_, iTotal_, title_);
    }
  }
  inline void resetLastDisplayedValue(){
    std::cout << "resetLastDisplayedValue" << std::endl;
    ProgressBar::lastDisplayedValue = -1;
    ProgressBar::lastDisplayedPercentValue = -1;
  }

}


//! Printout Tools
namespace GenericToolbox{

  void waitProgressBar(unsigned int nbMilliSecToWait_, const std::string &progressTitle_) {

    auto anchorTimePoint = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds totalDurationToWait(nbMilliSecToWait_*1000);
    std::chrono::microseconds cumulatedDuration(0);
    std::chrono::microseconds loopUpdateMaxFrequency(nbMilliSecToWait_); // 1000x faster than the whole time

    GenericToolbox::displayProgressBar( 0, totalDurationToWait.count(), progressTitle_);
    while( true ){
      std::this_thread::sleep_for( loopUpdateMaxFrequency );
      cumulatedDuration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - anchorTimePoint);
      if( cumulatedDuration >= totalDurationToWait ){
        return;
      }
      else{
        GenericToolbox::displayProgressBar( cumulatedDuration.count(), totalDurationToWait.count(), progressTitle_);
      }
    }
    GenericToolbox::displayProgressBar( totalDurationToWait.count(), totalDurationToWait.count(), progressTitle_);

  }
  std::string parseIntAsString(int intToFormat_){
    if(intToFormat_ / 1000 < 10){
        return std::to_string(intToFormat_);
    }
    intToFormat_/=1000.; // in K
    if(intToFormat_ / 1000 < 10){
      return std::to_string(intToFormat_) + "K";
    }
    intToFormat_/=1000.; // in M
    if(intToFormat_ / 1000 < 10){
      return std::to_string(intToFormat_) + "M";
    }
    intToFormat_/=1000.; // in G
    if(intToFormat_ / 1000 < 10){
      return std::to_string(intToFormat_) + "G";
    }
    intToFormat_/=1000.; // in T
    if(intToFormat_ / 1000 < 10){
      return std::to_string(intToFormat_) + "T";
    }
    intToFormat_/=1000.; // in P
    return std::to_string(intToFormat_) + "P";
  }
  std::string highlightIf(bool condition_, const std::string& text_){
    std::stringstream ss;
    ss << ( condition_ ? ColorCodes::redBackGround : "" );
    ss << text_;
    ss << ( condition_ ? ColorCodes::resetColor : "" );
    return ss.str();
  }
  inline std::string makeRainbowString(const std::string& inputStr_, bool stripUnicode_){
    std::string outputString;
    std::string inputStrStripped;
    stripUnicode_ ? inputStrStripped = GenericToolbox::stripStringUnicode(inputStr_) : inputStrStripped = inputStr_;
    double nbCharsPerColor = double(inputStrStripped.size()) / double(ProgressBar::rainbowColorList.size());
    int colorSlot{0};
    for( int iChar = 0 ; iChar < inputStrStripped.size() ; iChar++ ){
      if( nbCharsPerColor < 1 or iChar == 0 or ( (iChar+1) / nbCharsPerColor) - colorSlot + 1 > 1 ){
        outputString += ProgressBar::rainbowColorList[colorSlot++];
      }
      outputString += inputStrStripped[iChar];
    }
    outputString += ColorCodes::resetColor;
    return outputString;
  }

}


//! Vector management
namespace GenericToolbox {

  template <typename T> bool doesElementIsInVector(T element_, const std::vector<T>& vector_){
    return std::find(vector_.begin(), vector_.end(), element_) != vector_.end();
  }
  template <typename T, typename TT> inline std::vector<TT> convertVectorType( const std::vector<T>& vector_, std::function<TT(T)>& convertTypeFunction_ ){
    std::vector<TT> outVec;
    for(const auto& element : vector_){
      outVec.emplace_back(convertTypeFunction_(element));
    }
    return outVec;
  }
  bool doesElementIsInVector(const char* element_, const std::vector<std::string>& vector_){
    return std::find(vector_.begin(), vector_.end(), element_) != vector_.end();
  }
  template <typename T> inline int findElementIndex(T element_, const std::vector<T>& vector_ ){ // test
    int outIndex = -1;
    auto it = std::find(vector_.begin(), vector_.end(), element_);
    if (it != vector_.end()){
      outIndex = std::distance(vector_.begin(), it);
    }
    return outIndex;
  }
  inline int findElementIndex(const char* element_, const std::vector<std::string>& vector_ ){
    int outIndex = -1;
    auto it = std::find(vector_.begin(), vector_.end(), element_);
    if (it != vector_.end()){
      outIndex = int(std::distance(vector_.begin(), it));
    }
    return outIndex;
  }
  template <typename T> inline double getAverage(const std::vector<T>& vector_){
    double outVal = 0;
    for( auto& element : vector_ ){
      outVal += element;
    }
    return outVal / vector_.size();
  }
  template <typename T> std::vector<T> getSubVector( const std::vector<T>& vector_, size_t beginIndex_, int endIndex_ ){
    if( endIndex_ < 0 ){ endIndex_ += vector_.size(); }
    if( beginIndex_ >= endIndex_ ){
      return std::vector<T> ();
    }
    return std::vector<T> ( &vector_[beginIndex_] , &vector_[endIndex_+1] );
  }
  template <typename T> std::vector<size_t> getSortPermutation(const std::vector<T>& vectorToSort_, std::function<bool(const T&, const T&)> compareLambda_ ){
    std::vector<size_t> p(vectorToSort_.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
              [&](size_t i, size_t j){ return compareLambda_(vectorToSort_.at(i), vectorToSort_.at(j)); });
    return p;
  }
  template <typename T> std::vector<size_t> getSortPermutation(const std::vector<T>& vectorToSort_, std::function<bool(const T, const T)> compareLambda_ ){
    std::vector<size_t> p(vectorToSort_.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
              [&](size_t i, size_t j){ return compareLambda_(vectorToSort_[i], vectorToSort_[j]); });
    return p;
  }
  template <typename T> std::vector<T> applyPermutation(const std::vector<T>& vectorToPermute_, const std::vector<std::size_t>& sortPermutation_ ){
    std::vector<T> sorted_vec(vectorToPermute_.size());
    std::transform(sortPermutation_.begin(), sortPermutation_.end(), sorted_vec.begin(),
                   [&](std::size_t i){ return vectorToPermute_[i]; });
    return sorted_vec;
  }
  template<typename T> inline void insertInVector(std::vector<T> &vector_, const std::vector<T> &vectorToInsert_, size_t insertBeforeThisIndex_){
    if( insertBeforeThisIndex_ > vector_.size() ){
      throw std::runtime_error("GenericToolBox::insertInVector error: insertBeforeThisIndex_ >= vector_.size()");
    }
    if( vectorToInsert_.empty() ){
      return;
    }
    if( vector_.empty() ){
      vector_ = vectorToInsert_;
      return;
    }

    int nElementToInsert = int(vectorToInsert_.size());
    for( int iElementToInsert = nElementToInsert-1 ; iElementToInsert >= 0 ; iElementToInsert-- ){
      vector_.insert(vector_.begin() + insertBeforeThisIndex_, vectorToInsert_[iElementToInsert]);
    }
  }
  template<typename T> inline void insertInVector(std::vector<T> &vector_, const T &elementToInsert_, size_t insertBeforeThisIndex_){
    std::vector<T> vectorToInsert(1, elementToInsert_);
    insertInVector(vector_, vectorToInsert, insertBeforeThisIndex_);
  }
  template <typename T> std::string parseVectorAsString(const std::vector<T>& vector_, bool enableLineJump_){
    std::stringstream ss;
    ss << "{ ";
    bool isFirst = true;
    for(const auto& element: vector_){
      if(not isFirst) ss << ", ";
      else isFirst = false;
      if(enableLineJump_) ss << std::endl;
      ss << element;
    }
    if(enableLineJump_) ss << std::endl << "}";
    else ss << " }";
    return ss.str();
  }
  template <typename T> void printVector(const std::vector<T>& vector_, bool enableLineJump_){
    std::cout << parseVectorAsString(vector_, enableLineJump_) << std::endl;
  }
  template<typename T> inline std::vector<size_t> indices(const std::vector<T> &vector_){
    std::vector<size_t> output(vector_.size(), 0);
    for( size_t iIndex = 0 ; iIndex < output.size() ; iIndex++ ){
      output.at(iIndex) = iIndex;
    }
    return output;
  }
  template<typename T> inline double getAveragedSlope(const std::vector<T> &yValues_){
    auto xValues = yValues_;
    for( size_t iVal = 0 ; iVal < yValues_.size() ; iVal++ ){
      xValues.at(iVal) = iVal;
    }
    return getAveragedSlope(yValues_, xValues);
  }
  template<typename T, typename TT> inline double getAveragedSlope(const std::vector<T> &yValues_, const std::vector<TT> &xValues_){
    if(xValues_.size() != yValues_.size()){
      throw std::logic_error("x and y values list do have the same size.");
    }
    const auto n    = xValues_.size();
    const auto s_x  = std::accumulate(xValues_.begin(), xValues_.end(), 0.0);
    const auto s_y  = std::accumulate(yValues_.begin(), yValues_.end(), 0.0);
    const auto s_xx = std::inner_product(xValues_.begin(), xValues_.end(), xValues_.begin(), 0.0);
    const auto s_xy = std::inner_product(xValues_.begin(), xValues_.end(), yValues_.begin(), 0.0);
    const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
    return a;
  }

  template<typename T, typename TT> inline T& getListEntry(std::list<T>& list_, TT index_){
    typename std::list<T>::iterator it = list_.begin();
    std::advance(it, index_);
    return *it;
  }
  template<typename T, typename TT> inline const T& getListEntry(const std::list<T>& list_, TT index_){
    typename std::list<T>::const_iterator it = list_.begin();
    std::advance(it, index_);
    return *it;
  }

}


//! Map management
namespace GenericToolbox {

  template <typename K, typename  T> inline bool doesKeyIsInMap( const K& key_, const std::map<K,T>& map_ ){
    return ( map_.find(key_) != map_.end() );
  }
  template <typename K, typename T> inline T* getElementPtrIsInMap( const K& key_, std::map<K,T>& map_ ){
    auto it = map_.find(key_);
    if( it == map_.end() ){
      return nullptr;
    }
    return &( it->second );
  }
  template <typename T1, typename T2> inline void appendToMap(std::map<T1, T2> &mapContainer_, const std::map<T1, T2> &mapToPushBack_, bool overwrite_) {
    for(const auto& newEntry : mapToPushBack_){
      if(not overwrite_ and doesKeyIsInMap(newEntry.first, mapContainer_)){
        continue;
      }
      mapContainer_[newEntry.first] = newEntry.second;
    }
  }
  template <typename T> inline std::map<std::string, T> getSubMap(const std::map<std::string, T>& map_, const std::string &keyStrStartWith_ ){
    std::map<std::string, T> outSubMap;
    for(const auto& mapPair : map_){
      if(GenericToolbox::doesStringStartsWithSubstring(mapPair.first, keyStrStartWith_)){
        outSubMap[mapPair.first] = mapPair.second;
      }
    }
    return outSubMap;
  }
  template <typename T1, typename T2> inline std::string parseMapAsString(const std::map<T1, T2>& map_, bool enableLineJump_){
    std::stringstream ss;
    ss << "{ ";
    bool isFirst = true;
    for(const auto& pair: map_){
      if(not isFirst) ss << ", ";
      else isFirst = false;
      if(enableLineJump_) ss << std::endl;
      ss << pair.first << " = " << pair.second;
    }
    if(enableLineJump_) ss << std::endl << "}";
    else ss << " }";
    return ss.str();
  }
  template <typename T1, typename T2> inline void printMap(const std::map<T1, T2>& map_, bool enableLineJump_){
    std::cout << parseMapAsString(map_, enableLineJump_) << std::endl;
  }

}


// String Management Tools
namespace GenericToolbox {

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
#else
  namespace StringManagementUtils{
    static std::regex ansiRegex("\033((\\[((\\d+;)*\\d+)?[A-DHJKMRcfghilmnprsu])|\\(|\\))");
  }
#endif

  bool doesStringContainsSubstring(std::string string_, std::string substring_, bool ignoreCase_) {
    if (substring_.empty()) return true;
    if (substring_.size() > string_.size()) return false;
    if (ignoreCase_) {
      string_ = toLowerCase(string_);
      substring_ = toLowerCase(substring_);
    }
    if (string_.find(substring_) != std::string::npos) return true;
    else return false;
  }
  bool doesStringStartsWithSubstring(std::string string_, std::string substring_, bool ignoreCase_) {
    if (substring_.empty()) return true;
    if (substring_.size() > string_.size()) return false;
    if (ignoreCase_) {
      string_ = toLowerCase(string_);
      substring_ = toLowerCase(substring_);
    }
    return (not string_.compare(0, substring_.size(), substring_));
  }
  bool doesStringEndsWithSubstring(std::string string_, std::string substring_, bool ignoreCase_) {
    if (substring_.empty()) return true;
    if (substring_.size() > string_.size()) return false;
    if (ignoreCase_) {
      string_ = toLowerCase(string_);
      substring_ = toLowerCase(substring_);
    }
    return (not string_.compare(string_.size() - substring_.size(), substring_.size(), substring_));
  }
  inline std::string toLowerCase(const std::string &inputStr_) {
    std::string output_str(inputStr_);
    std::transform(output_str.begin(), output_str.end(), output_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return output_str;
  }
  inline std::string stripStringUnicode(const std::string &inputStr_){
    std::string outputStr(inputStr_);

    if(GenericToolbox::doesStringContainsSubstring(outputStr, "\033")){
      // remove color
      std::string tempStr;
      auto splitOuputStr = GenericToolbox::splitString(outputStr, "\033");
      for(const auto& sliceStr : splitOuputStr){
        if(sliceStr.empty()) continue;
        if(tempStr.empty()){
          tempStr = sliceStr;
          continue;
        }
        // look for a 'm' char that determines the end of the color code
        bool mCharHasBeenFound = false;
        for(const char& c : sliceStr){
          if(not mCharHasBeenFound){
            if(c == 'm'){
              mCharHasBeenFound = true;
            }
          }
          else{
            tempStr += c;
          }
        }
      }
      outputStr = tempStr;
    }

    outputStr.erase(
      remove_if(
        outputStr.begin(), outputStr.end(),
        [](const char& c){return !isprint( static_cast<unsigned char>( c ) );}
      ),
      outputStr.end()
    );

    return outputStr;
  }
  inline size_t getPrintSize(const std::string& str_){
    if( str_.empty() ) return 0;
#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
// this is gcc 4.8 or earlier
// std::regex support is buggy, so don't use in this block
  return str_.size();
#else
// this is gcc 4.9 or later, or other compilers like clang
// hopefully std::regex support is ok here
    std::string::iterator::difference_type result = 0;
    std::for_each(
      std::sregex_token_iterator(str_.begin(), str_.end(), StringManagementUtils::ansiRegex, -1),
      std::sregex_token_iterator(),
      [&result](std::sregex_token_iterator::value_type const& e) {
        std::string tmp(e);
        result += std::count_if(tmp.begin(), tmp.end(), ::isprint);
      }
    );
    return result;
#endif
  }
  inline std::string repeatString(const std::string &inputStr_, int amount_){
    std::string outputStr;
    if(amount_ <= 0) return outputStr;
    for(int i_count = 0 ; i_count < amount_ ; i_count++){
      outputStr += inputStr_;
    }
    return outputStr;
  }
  inline std::string trimString(const std::string &inputStr_, const std::string &strToTrim_){
      std::string outputStr(inputStr_);
      while(GenericToolbox::doesStringStartsWithSubstring(outputStr, strToTrim_)){
          outputStr = outputStr.substr(strToTrim_.size(), outputStr.size());
      }
      while(GenericToolbox::doesStringEndsWithSubstring(outputStr, strToTrim_)){
          outputStr = outputStr.substr(0, outputStr.size() - strToTrim_.size());
      }
      return outputStr;
  }
  inline std::string padString(const std::string& inputStr_, unsigned int padSize_, const char& padChar){
    std::string outputString;
    int padDelta = int(inputStr_.size()) - int(padSize_);
    while( padDelta < 0 ){
      // add extra chars if needed
      outputString += padChar;
      padDelta++;
    }
    outputString += inputStr_;
    return outputString.substr(0, outputString.size() - padDelta);
  }
  inline std::string indentString(const std::string& inputStr_, unsigned int indentCount_, const std::string& indentChar){
    std::string outStr = inputStr_;
    GenericToolbox::indentInputString(outStr, indentCount_, indentChar);
    return outStr;
  }
  inline std::string removeRepeatedCharacters(const std::string &inputStr_, const std::string &repeatedChar_) {
    std::string outStr = inputStr_;
    GenericToolbox::removeRepeatedCharInsideInputStr(outStr, repeatedChar_);
    return outStr;
  }
  std::string joinVectorString(const std::vector<std::string> &string_list_, const std::string &delimiter_, int begin_index_, int end_index_) {

    std::string joined_string;
    if (end_index_ == 0) end_index_ = int(string_list_.size());

    // circular permutation -> python style : tab[-1] = tab[tab.size - 1]
    if (end_index_ < 0 and int(string_list_.size()) > std::fabs(end_index_))
      end_index_ = int(string_list_.size()) + end_index_;

    for (int i_list = begin_index_; i_list < end_index_; i_list++) {
      if (not joined_string.empty()) joined_string += delimiter_;
      joined_string += string_list_[i_list];
    }

    return joined_string;
  }
  std::string replaceSubstringInString(const std::string &input_str_, const std::string &substr_to_look_for_, const std::string &substr_to_replace_) {
    std::string stripped_str = input_str_;
    GenericToolbox::replaceSubstringInsideInputString(stripped_str, substr_to_look_for_, substr_to_replace_);
    return stripped_str;
  }
  std::string parseUnitPrefix(double val_, int maxPadSize_){
    std::stringstream ss;

    if( val_ < 0 ){
      ss << "-";
      val_ = -val_;
      maxPadSize_--;
    }

    if(maxPadSize_ > -1){
      ss << std::setprecision(maxPadSize_-1);
    }

    auto reducedVal = size_t(fabs(val_));
    if( reducedVal > 0 ){
      if     ( (reducedVal = (reducedVal / 1000)) == 0 ){
        ss << val_;
      }
      else if( (reducedVal = (reducedVal / 1000)) == 0 ){
        ss << val_/1E3 << "K";
      }
      else if( (reducedVal = (reducedVal / 1000)) == 0 ){
        ss << val_/1E6 << "M";
      }
      else if( (reducedVal = (reducedVal / 1000)) == 0 ){
        ss << val_/1E9 << "G";
      }
      else if( (reducedVal = (reducedVal / 1000)) == 0 ){
        ss << val_/1E12 << "T";
      }
      else {
        ss << val_/1E15 << "P";
      }
    } // K, M, G, T, P
    else{
      if( val_ < 1E-3 ){ // force scientific notation
        ss << std::scientific << val_;
      }
      else{
        ss << val_;
      }
    }


    return ss.str();
  }
  std::string parseSizeUnits(unsigned int sizeInBytes_){
    return parseUnitPrefix(sizeInBytes_) + "B";
  }
  std::vector<std::string> splitString(const std::string &inputString_, const std::string &delimiter_, bool removeEmpty_) {

    std::vector<std::string> outputSliceList;

    const char *src = inputString_.c_str();
    const char *next = src;

    std::string out_string_piece;

    while ((next = std::strstr(src, delimiter_.c_str())) != nullptr) {
      out_string_piece = "";
      while (src != next) {
        out_string_piece += *src++;
      }
      outputSliceList.emplace_back(out_string_piece);
      /* Skip the delimiter_ */
      src += delimiter_.size();
    }

    /* Handle the last token */
    out_string_piece = "";
    while (*src != '\0')
      out_string_piece += *src++;

    outputSliceList.emplace_back(out_string_piece);

    if(not removeEmpty_){
      return outputSliceList;
    }
    else{
      std::vector<std::string> strippedOutput;
      for(const auto& slice : outputSliceList){
        if(not slice.empty()){
          strippedOutput.emplace_back(slice);
        }
      }
      return strippedOutput;
    }


  }
  inline std::string formatString( const std::string& strToFormat_ ){
    return strToFormat_;
  }
  template<typename ... Args> std::string formatString(const std::string& strToFormat_, Args ... args) {
    size_t size = snprintf(nullptr, 0, strToFormat_.c_str(), args ...) + 1; // Extra space for '\0'
    if (size <= 0) { throw std::runtime_error("Error during formatting."); }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, strToFormat_.c_str(), args ...);
    return {buf.get(), buf.get() + size - 1}; // We don't want the '\0' inside
  }

  inline void replaceSubstringInsideInputString(std::string &input_str_, const std::string &substr_to_look_for_, const std::string &substr_to_replace_){
    size_t index = 0;
    while ((index = input_str_.find(substr_to_look_for_, index)) != std::string::npos) {
      input_str_.replace(index, substr_to_look_for_.length(), substr_to_replace_);
      index += substr_to_replace_.length();
    }
  }
  inline void removeRepeatedCharInsideInputStr(std::string &inputStr_, const std::string &doubledChar_){
    std::string doubledCharStr = doubledChar_+doubledChar_;
    std::string lastStr;
    do{
      lastStr = inputStr_;
      GenericToolbox::replaceSubstringInsideInputString(inputStr_, doubledCharStr, doubledChar_);
    } while( lastStr != inputStr_ );
  }
  inline void indentInputString(std::string& inputStr_, unsigned int indentCount_, const std::string& indentChar){
    int originalSize = int(inputStr_.size());
    for( int iChar = originalSize-1 ; iChar >= 0 ; iChar-- ){
      if( iChar == 0 or inputStr_[iChar] == '\n'){
        int offSet = 1;
        if( iChar == 0 ) offSet = 0;
        for( unsigned int iIndent = 0 ; iIndent < indentCount_ ; iIndent++ ){
          inputStr_.insert(iChar+offSet, indentChar);
        }
      }
    }
  }

}


// Conversion Tools
namespace GenericToolbox {

  template<typename T> std::string toHexString(T integerVal_, size_t nbDigit_){
    // filling the trailing digit with 0
    // but how many 0? -> 1 hex digit represent 4 bit -> sizeof returns the size in bytes = N x 8 bit
    // so # of digit of hex is: # = N x 8 / 4 = N x 2
    std::stringstream stream;
    stream << "0x" << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << integerVal_;
    if( nbDigit_ == 0 ) return stream.str();
    else return "0x" + stream.str().substr(2 + sizeof(T)*2 - nbDigit_, nbDigit_);
  }
  bool toBool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
  }

}


// OS Tools
namespace GenericToolbox{

  namespace Internals {
    inline char * getEnvironmentVariable(char const envVarName_[]){
#if defined _WIN32 // getenv() is deprecated on Windows
      char *buf{nullptr};
    size_t sz;
    std::string val;
    if (_dupenv_s(&buf, &sz, envVarName_) || buf == nullptr) return val;
    val = buf;
    free(buf);
    return val;
#else
      return getenv(envVarName_);
#endif
    }
    inline bool expandEnvironmentVariables(const char *inputFilePath_, char *extendedFilePath_) {
      int ier, iter, lx, ncopy;
      char *inp, *out, *x, *t, *charBuffer;
      const char *b, *c, *e;
      const char *expandedPathCharArray;
      int bufferSize_ = 8192; // max path length
      charBuffer = new char[bufferSize_ * 4];

      iter = 0;
      extendedFilePath_[0] = 0;
      inp = charBuffer + bufferSize_;
      out = inp + bufferSize_;
      inp[-1] = ' ';
      inp[0] = 0;
      out[-1] = ' ';
      c = inputFilePath_ + strspn(inputFilePath_, " \t\f\r");
      //VP  if (isalnum(c[0])) { strcpy(inp, WorkingDirectory()); strcat(inp, "/"); } // add $cwd

//      strlcat(inp, c, bufferSize_);
      strncat(inp, c, bufferSize_);

      again:
      iter++; c = inp; ier = 0;
      x = out; x[0] = 0;

      expandedPathCharArray = nullptr;
      e = nullptr;
      if (c[0] == '~' && c[1] == '/') { // ~/ case
        std::string homeDirStr = getHomeDirectory();
        expandedPathCharArray = homeDirStr.c_str();
        e = c + 1;
        if (expandedPathCharArray) {                         // we have smth to copy
//          strlcpy(x, expandedPathCharArray, bufferSize_);
          strncpy(x, expandedPathCharArray, bufferSize_);
          x += strlen(expandedPathCharArray);
          c = e;
        } else {
          ++ier;
          ++c;
        }
      }
      else if (c[0] == '~' && c[1] != '/') { // ~user case
        int n = int(strcspn(c+1, "/ "));
//        assert((n+1) < bufferSize_ && "This should have been prevented by the truncation 'strlcat(inp, c, bufferSize_)'");
//        assert((n+1) < bufferSize_ && "This should have been prevented by the truncation 'strncat(inp, c, bufferSize_)'");
        // There is no overlap here as the buffer is segment in 4 strings of at most bufferSize_
//        (void)strlcpy(charBuffer, c + 1, n + 1); // strlcpy copy 'size-1' characters.
        (void)strncpy(charBuffer, c + 1, n + 1); // strncpy copy 'size-1' characters.
        std::string homeDirStr = getHomeDirectory();
        e = c+1+n;
        if (!homeDirStr.empty()) {                   // we have smth to copy
          expandedPathCharArray = homeDirStr.c_str();
//          strlcpy(x, expandedPathCharArray, bufferSize_);
          strncpy(x, expandedPathCharArray, bufferSize_);
          x += strlen(expandedPathCharArray);
          c = e;
        } else {
          x++[0] = c[0];
          //++ier;
          ++c;
        }
      }

      for ( ; c[0]; c++) {

        expandedPathCharArray = nullptr; e = nullptr;

        if (c[0] == '.' && c[1] == '/' && c[-1] == ' ') { // $cwd
          std::string workDirStr = getCurrentWorkingDirectory();
//          strlcpy(charBuffer, workDirStr.c_str(), bufferSize_);
          strncpy(charBuffer, workDirStr.c_str(), bufferSize_);
          expandedPathCharArray = charBuffer;
          e = c + 1;
        }
        if (expandedPathCharArray) {                          // we have smth to copy */
//          strlcpy(x, expandedPathCharArray, bufferSize_); x += strlen(expandedPathCharArray); c = e - 1; continue;
          strncpy(x, expandedPathCharArray, bufferSize_); x += strlen(expandedPathCharArray); c = e - 1; continue;
        }

        if (c[0] != '$') {                // not $, simple copy
          x++[0] = c[0];
        } else {                          // we have a $
          b = c+1;
          if (c[1] == '(') b++;
          if (c[1] == '{') b++;
          if (b[0] == '$')
            e = b+1;
          else
            for (e = b; isalnum(e[0]) || e[0] == '_'; e++) ;
          charBuffer[0] = 0; strncat(charBuffer, b, e - b);
          expandedPathCharArray = getEnvironmentVariable(charBuffer);
          if (!expandedPathCharArray) {                      // too bad, try UPPER case
            for (t = charBuffer; (t[0] = static_cast<char>(toupper(t[0]))); t++) ;
            expandedPathCharArray = getEnvironmentVariable(charBuffer);
          }
          if (!expandedPathCharArray) {                      // too bad, try Lower case
            for (t = charBuffer; (t[0] = static_cast<char>(tolower(t[0]))); t++) ;
            expandedPathCharArray = getEnvironmentVariable(charBuffer);
          }
          if (!expandedPathCharArray && !strcmp(charBuffer, "cwd")) { // it is $cwd
            std::string wd = getCurrentWorkingDirectory();
//            strlcpy(charBuffer, wd.c_str(), bufferSize_);
            strncpy(charBuffer, wd.c_str(), bufferSize_);
            expandedPathCharArray = charBuffer;
          }
          if (!expandedPathCharArray && !strcmp(charBuffer, "$")) { // it is $$ (replace by GetPid())
            snprintf(charBuffer, bufferSize_ * 4, "%d", getpid());
            expandedPathCharArray = charBuffer;
          }
          if (!expandedPathCharArray) {                      // too bad, nothing can help
#ifdef WIN32
            // if we're on windows, we can have \\SomeMachine\C$ - don't
             // complain about that, if '$' is followed by nothing or a
             // path delimiter.
             if (c[1] && c[1]!='\\' && c[1]!=';' && c[1]!='/')
                ier++;
#else
            ier++;
#endif
            x++[0] = c[0];
          } else {                       // It is OK, copy result
            int lp = int(strlen(expandedPathCharArray));
            if (lp >= bufferSize_) {
              // make sure lx will be >= bufferSize_ (see below)
//              strlcpy(x, expandedPathCharArray, bufferSize_);
              strncpy(x, expandedPathCharArray, bufferSize_);
              x += bufferSize_;
              break;
            }
            strcpy(x, expandedPathCharArray);
            x += lp;
            c = (b==c+1) ? e-1 : e;
          }
        }
      }

      x[0] = 0; lx = x - out;
//      if (ier && iter < 3) { strlcpy(inp, out, bufferSize_); goto again; }
      if (ier && iter < 3) { strncpy(inp, out, bufferSize_); goto again; }
      ncopy = (lx >= bufferSize_) ? bufferSize_ - 1 : lx;
      extendedFilePath_[0] = 0; strncat(extendedFilePath_, out, ncopy);

      delete[] charBuffer;

//    if (ier || ncopy != lx) {
//      ::Error("TSystem::expandEnvironmentVariables", "input: %s, output: %s", inputFilePath_, extendedFilePath_);
//      return true;
//    }

      return false;
    }
  }

  inline std::string getHomeDirectory(){
    struct passwd *pw = getpwuid(getuid());
    return {pw->pw_dir};
  }
  inline std::string getCurrentWorkingDirectory(){
#ifdef PATH_MAX
    char cwd[PATH_MAX];
#else
    char cwd[1024];
#endif
    if( getcwd(cwd, sizeof(cwd)) == nullptr ){
      throw std::runtime_error("getcwd() returned an invalid value.");
    }
    std::string output_cwd(cwd);
    return output_cwd;
  }
  inline std::string expandEnvironmentVariables(const std::string &filePath_){

//    char outputName[PATH_MAX];
    char outputName[8192];
    Internals::expandEnvironmentVariables(filePath_.c_str(), outputName);

    return {outputName};
  }
  inline std::string getExecutableName(){
    std::string outStr;
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__) //check defines for your setup
    std::ifstream("/proc/self/comm") >> outStr;
#elif defined(_WIN32)
    char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    outStr = buf;
#else
    outStr = __progname;
#endif
    return outStr;
  }

}

// FS Tools
namespace GenericToolbox{

  // -- without IO dependencies (string parsing)
  bool doesFilePathHasExtension(const std::string &filePath_, const std::string &extension_){
    return doesStringEndsWithSubstring(filePath_, "." + extension_);
  }
  std::string getFolderPathFromFilePath(const std::string &filePath_){
    std::string folder_path;
    if(filePath_[0] == '/') folder_path += "/";
    auto splitted_path = splitString(filePath_, "/");
    folder_path += joinVectorString(
      splitted_path,
      "/",
      0,
      int(splitted_path.size()) - 1
    );
    return folder_path;
  }
  std::string getFileNameFromFilePath(const std::string &filePath_, bool keepExtension_){
    auto splitStr = GenericToolbox::splitString(filePath_, "/");
    if(not splitStr.empty()){
      return ( keepExtension_ ?
        splitStr[splitStr.size()-1]:
        GenericToolbox::splitString(splitStr[splitStr.size()-1], ".")[0]
      );
    }
    return {};
  }

  // -- with direct IO dependencies
  bool doesPathIsFile(const std::string &filePath_){
    struct stat info{};
    stat(filePath_.c_str(), &info);
    return S_ISREG(info.st_mode);
  }
  bool doesPathIsFolder(const std::string &folderPath_){
    struct stat info{};
    stat( folderPath_.c_str(), &info );
    return bool(S_ISDIR(info.st_mode));
  }
  bool doFilesAreTheSame(const std::string &filePath1_, const std::string &filePath2_){

    if( not doesPathIsFile(filePath1_) ) return false;
    if( not doesPathIsFile(filePath2_) ) return false;

    std::ifstream fileStream1(filePath1_);
    std::ifstream fileStream2(filePath2_);

    // Buffer size 1 Megabyte (or any number you like)
    size_t buffer_size = 1<<20;
    char *buffer1 = new char[buffer_size];
    char *buffer2 = new char[buffer_size];

    std::hash<std::string> hashBuffer1;
    std::hash<std::string> hashBuffer2;

    while (fileStream1 and fileStream2) {
      // Try to read next chunk of data
      fileStream1.read(buffer1, long(buffer_size));
      fileStream2.read(buffer2, long(buffer_size));

      // Get the number of bytes actually read
      if(fileStream1.gcount() != fileStream2.gcount()){
        return false;
      }

      size_t count = fileStream1.gcount();
      // If nothing has been read, break
      if( count == 0 ){
        break;
      }

      // Compare hash files
      if(hashBuffer1(buffer1) != hashBuffer2(buffer2))
        return false;

    }

    delete[] buffer1;
    delete[] buffer2;

    return true;
  }
  bool mkdirPath(const std::string &newFolderPath_){
    bool result = false;
    if(doesPathIsFolder(newFolderPath_)) return true;

    std::string current_level;
    std::string level;
    std::stringstream ss(newFolderPath_);

    // split path using slash as a separator
    while (std::getline(ss, level, '/')){
      current_level += level; // append folder to the current level
      if(current_level.empty()) current_level = "/";
      current_level = removeRepeatedCharacters(current_level, "/");
      // create current level
      if(not doesPathIsFolder(current_level)){
        ::mkdir(current_level.c_str(), 0777);
        result = true;
      }
      current_level += "/"; // don't forget to append a slash
    }

    return result;

  }
  bool deleteFile(const std::string &filePath_){
    if(not doesPathIsFile(filePath_)) return true;
    std::remove(filePath_.c_str());
    return not doesPathIsFile(filePath_);
  }
  bool copyFile(const std::string &source_file_path_, const std::string &destination_file_path_, bool force_){

    if( not doesPathIsFile(source_file_path_) ){
      return false;
    }

    if( doesPathIsFile(destination_file_path_) ){
      if( force_ ){
        deleteFile(destination_file_path_);
      }
      else{
        return false;
      }
    }

    std::ifstream  src(source_file_path_, std::ios::binary);
    std::ofstream  dst(destination_file_path_,   std::ios::binary);

    dst << src.rdbuf();

    return true;
  }
  bool mvFile(const std::string &sourceFilePath_, const std::string &destinationFilePath_, bool force_) {

    if( not doesPathIsFile(sourceFilePath_) ){
      return false;
    }

    if( doesPathIsFile(destinationFilePath_) ){
      if(force_){
        deleteFile(destinationFilePath_);
      }
      else{
        return false;
      }
    }
    else{
      std::string destination_folder_path = getFolderPathFromFilePath(destinationFilePath_);
      if(not doesPathIsFile(destination_folder_path)){
        mkdirPath(destination_folder_path);
      }
    }

    std::rename(sourceFilePath_.c_str(), destinationFilePath_.c_str());

    if(
      doesPathIsFile(destinationFilePath_)
      and not doesPathIsFile(sourceFilePath_)
      ) return true;
    else return false;
  }
  size_t getHashFile(const std::string &filePath_) {
    std::hash<std::string> hashString;
    return hashString(dumpFileAsString(filePath_));
  }
  long int getFileSizeInBytes(const std::string &filePath_){
    long int output_size = 0;
    if(doesPathIsFile(filePath_)){
      std::ifstream testFile(filePath_.c_str(), std::ios::binary);
      const auto begin = testFile.tellg();
      testFile.seekg (0, std::ios::end);
      const auto end = testFile.tellg();
      const auto fsize = (end-begin);
      output_size = fsize;
    }
    return output_size;
  }
  void dumpStringInFile(const std::string &outFilePath_, const std::string &stringToWrite_){
    std::ofstream out(outFilePath_.c_str());
    out << stringToWrite_;
    out.close();
  }
  std::string dumpFileAsString(const std::string &filePath_){
    std::string data;
    if(doesPathIsFile(filePath_)){
      std::ifstream input_file(filePath_.c_str(), std::ios::binary | std::ios::in );
      std::ostringstream ss;
      ss << input_file.rdbuf();
      data = ss.str();
      input_file.close();
    }
    return data;
  }
  std::vector<std::string> dumpFileAsVectorString(const std::string &filePath_){
    std::vector<std::string> lines;
    if(doesPathIsFile(filePath_)){
      std::string data = GenericToolbox::dumpFileAsString(filePath_);
      lines = GenericToolbox::splitString(data, "\n");
    }
    for(auto& line: lines){
      if(GenericToolbox::doesStringEndsWithSubstring(line, "\r")){
        line = line.substr(0, line.size()-1);
      }
    }

    return lines;
  }
  std::vector<std::string> getListOfEntriesInFolder(const std::string &folderPath_, const std::string &entryNameRegex_) {

    std::vector<std::string> entries_list;
    if(not doesPathIsFolder(folderPath_)) return entries_list;
    DIR* directory;
    directory = opendir(folderPath_.c_str()); //Open current-working-directory.
    if( directory == nullptr ) {
      std::cout << "Failed to open directory : " << folderPath_ << std::endl;
      return entries_list;
    }
    else {

      std::vector<std::string> nameElements;
      if(not entryNameRegex_.empty()){
        nameElements = GenericToolbox::splitString(entryNameRegex_, "*");
      }
      bool isValid;

      struct dirent* entry;
      while ( (entry = readdir(directory)) ) {
        isValid = true;

        if(std::string(entry->d_name) == "." or std::string(entry->d_name) == ".."){
          isValid = false;
        }
        else if(not entryNameRegex_.empty()){
          std::string entryCandidate = entry->d_name;

          for( int iElement = 0 ; iElement < int(nameElements.size()) ; iElement++ ){

            if( iElement == 0 ){
              if( not GenericToolbox::doesStringStartsWithSubstring(entryCandidate, nameElements[iElement]) ){
                isValid = false;
                break;
              }
            }
            else if( iElement+1 == int(nameElements.size()) ){
              if(not GenericToolbox::doesStringEndsWithSubstring(entryCandidate, nameElements[iElement]) ){
                isValid = false;
              }
            }
            else{
              if( not GenericToolbox::doesStringContainsSubstring(entryCandidate, nameElements[iElement])
                  ){
                isValid = false;
                break;
              }
            }

            if( iElement+1 != int(nameElements.size()) ){
              entryCandidate = GenericToolbox::splitString(entryCandidate, nameElements[iElement]).back();
            }
          }
        }
        if(isValid) entries_list.emplace_back(entry->d_name);
      }
      closedir(directory);
      return entries_list;
    }

  }
  std::vector<std::string> getListOfSubfoldersInFolder(const std::string &folderPath_, const std::string &entryNameRegex_) {
    std::vector<std::string> subfolders_list;
    if(not doesPathIsFolder(folderPath_)) return subfolders_list;
    DIR* directory;
    directory = opendir(folderPath_.c_str()); //Open current-working-directory.
    if( directory == nullptr ) {
      std::cout << "Failed to open directory : " << folderPath_ << std::endl;
      return subfolders_list;
    } else {

        std::vector<std::string> nameElements;
        if(not entryNameRegex_.empty()){
            nameElements = GenericToolbox::splitString(entryNameRegex_, "*");
        }
        bool isValid;

      struct dirent* entry;
      while ( (entry = readdir(directory)) ) {
          isValid = true;
        std::string folder_candidate = folderPath_ + "/" + std::string(entry->d_name);
        if(doesPathIsFolder(folder_candidate)){
            if(std::string(entry->d_name) == "." or std::string(entry->d_name) == ".."){
                isValid = false;
            }
            else if(not entryNameRegex_.empty()){
              std::string entryCandidate = entry->d_name;

              for( int iElement = 0 ; iElement < int(nameElements.size()) ; iElement++ ){

                if( iElement == 0 ){
                  if( not GenericToolbox::doesStringStartsWithSubstring(entryCandidate, nameElements[iElement]) ){
                    isValid = false;
                    break;
                  }
                }
                else if( iElement+1 == int(nameElements.size()) ){
                  if(not GenericToolbox::doesStringEndsWithSubstring(entryCandidate, nameElements[iElement]) ){
                    isValid = false;
                  }
                }
                else{
                  if( not GenericToolbox::doesStringContainsSubstring(entryCandidate, nameElements[iElement])
                      ){
                    isValid = false;
                    break;
                  }
                }

                if( iElement+1 != int(nameElements.size()) ){
                  entryCandidate = GenericToolbox::splitString(entryCandidate, nameElements[iElement]).back();
                }
              }
            }
            if(isValid) subfolders_list.emplace_back(entry->d_name);
        }
      }
      closedir(directory);
      return subfolders_list;
    }

  }
  std::vector<std::string> getListOfFilesInFolder(const std::string &folderPath_, const std::string &entryNameRegex_){
    std::vector<std::string> files_list;
    if(not doesPathIsFolder(folderPath_)) return files_list;
    DIR* directory;
    directory = opendir(folderPath_.c_str()); //Open current-working-directory.
    if( directory == nullptr ) {
      std::cout << "Failed to open directory : " << folderPath_ << std::endl;
      return files_list;
    } else {

        std::vector<std::string> nameElements;
        if(not entryNameRegex_.empty()){
            nameElements = GenericToolbox::splitString(entryNameRegex_, "*");
        }
        bool isValid;

      struct dirent* entry;
      while ( (entry = readdir(directory)) ) {
          isValid = true;
        std::string file_candidate = folderPath_ + "/" + std::string(entry->d_name);
        if(doesPathIsFile(file_candidate)){
            if(not entryNameRegex_.empty()){
              std::string entryCandidate = entry->d_name;

              for( int iElement = 0 ; iElement < int(nameElements.size()) ; iElement++ ){

                if( iElement == 0 ){
                  if( not GenericToolbox::doesStringStartsWithSubstring(entryCandidate, nameElements[iElement]) ){
                    isValid = false;
                    break;
                  }
                }
                else if( iElement+1 == int(nameElements.size()) ){
                  if(not GenericToolbox::doesStringEndsWithSubstring(entryCandidate, nameElements[iElement]) ){
                    isValid = false;
                  }
                }
                else{
                  if( not GenericToolbox::doesStringContainsSubstring(entryCandidate, nameElements[iElement])
                      ){
                    isValid = false;
                    break;
                  }
                }

                if( iElement+1 != int(nameElements.size()) ){
                  entryCandidate = GenericToolbox::splitString(entryCandidate, nameElements[iElement]).back();
                }
              }
            }
            if(isValid) files_list.emplace_back(entry->d_name);
        }
      }
      closedir(directory);
      return files_list;
    }
  }

  // -- with direct IO dependencies
  bool doesFolderIsEmpty(const std::string &folderPath_){
    if(not doesPathIsFolder(folderPath_)) return false;
    return getListOfEntriesInFolder(folderPath_, std::string()).empty();
  }
  std::vector<std::string> getListFilesInSubfolders(const std::string &folderPath_) {

    // WARNING : Recursive function
    std::vector<std::string> output_file_paths = getListOfFilesInFolder(folderPath_);

    auto subfolder_names_list = getListOfSubfoldersInFolder(folderPath_);
    for(auto &subfolder_name : subfolder_names_list){
      std::string subfolder_full_path = folderPath_;
      subfolder_full_path += "/";
      subfolder_full_path += subfolder_name;
      auto subfile_names_list = getListFilesInSubfolders(subfolder_full_path);
      for(auto &subfile_name : subfile_names_list){
        std::string relative_subfile_path;
        relative_subfile_path += subfolder_name;
        relative_subfile_path += "/";
        relative_subfile_path += subfile_name;
        output_file_paths.emplace_back(removeRepeatedCharacters(relative_subfile_path, "/"));
      }
    }

    return output_file_paths;

  }

}



// Hardware Tools
#if defined(_WIN32)
// Windows
#include <windows.h>
#include <psapi.h>
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
// MacOS
#include <unistd.h>
#include <sys/resource.h>
#include <mach/mach.h>
#include <sys/ioctl.h>
#include <array>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
// Linux
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <sys/ioctl.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
// AIX and Solaris
#include <unistd.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <procfs.h>
#include <sys/ioctl.h>

#else
// Unsupported
#endif
namespace GenericToolbox{

  size_t getProcessMemoryUsage(){
    /**
     * Returns the current resident set size (physical memory use) measured
     * in bytes, or zero if the value cannot be determined on this OS.
     */
#if defined(_WIN32)
    // Windows
    PROCESS_MEMORY_COUNTERS memCounter;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter))
        return (size_t)memCounter.WorkingSetSize;
    return (size_t)0; /* get process mem info failed */

#elif defined(__APPLE__) && defined(__MACH__)
    // MacOS
    struct mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS)
      return (size_t)info.resident_size;
    return (size_t)0; /* query failed */

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    // Linux
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

#else
    // AIX, BSD, Solaris, and Unknown OS
    return (size_t)0L;          /* Unsupported. */

#endif
  }
  size_t getProcessMaxMemoryUsage(){
    /**
     * Returns the peak (maximum so far) resident set size (physical
     * memory use) measured in bytes, or zero if the value cannot be
     * determined on this OS.
     */
#if defined(_WIN32)
    // Windows
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.PeakWorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
    // MacOS
    struct mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS)
      return (size_t)info.resident_size_max;
    return (size_t)0; /* query failed */

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    // Linux
    struct rusage rusage;
    if (!getrusage(RUSAGE_SELF, &rusage))
        return (size_t)rusage.ru_maxrss;
    return (size_t)0; /* query failed */

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    // AIX and Solaris
    struct psinfo psinfo;
    int fd = -1;
    if ( (fd = open( "/proc/self/psinfo", O_RDONLY )) == -1 )
        return (size_t)0L;      /* Can't open? */
    if ( read( fd, &psinfo, sizeof(psinfo) ) != sizeof(psinfo) )
    {
        close( fd );
        return (size_t)0L;      /* Can't read? */
    }
    close( fd );
    return (size_t)(psinfo.pr_rssize * 1024L);
#else
    // Unknown OS
    return (size_t)0L;          /* Unsupported. */
#endif
  }
  long getProcessMemoryUsageDiffSinceLastCall(){
      size_t currentProcessMemoryUsage = getProcessMemoryUsage();
      long outVal = static_cast<long>(currentProcessMemoryUsage) - static_cast<long>(Hardware::lastProcessMemoryUsage);
      Hardware::lastProcessMemoryUsage = currentProcessMemoryUsage;
      return outVal;
  }
  int getTerminalWidth(){
    int outWith = 0;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    outWith = (int)(csbi.dwSize.X);
//    outWith = (int)(csbi.dwSize.Y);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__) \
    || (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__))) \
    || ( defined(__APPLE__) && defined(__MACH__) )
    struct winsize winSize{};
    ioctl(fileno(stdout), TIOCGWINSZ, &winSize);
    outWith = (int)(winSize.ws_col);
//    outWith = (int)(winSize.ws_row);
#endif // Windows/Linux
    return outWith;
  }
  int getTerminalHeight(){
    int outWith = 0;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
//    outWith = (int)(csbi.dwSize.X);
    outWith = (int)(csbi.dwSize.Y);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__) \
    || (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__))) \
    || ( defined(__APPLE__) && defined(__MACH__) )
    struct winsize w{};
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
//    outWith = (int)(w.ws_col);
    outWith = (int)(w.ws_row);
#endif // Windows/Linux
    return outWith;
  }

  std::string parseTimeUnit(long long nbMicroSec_){
    if(nbMicroSec_ / 1000 < 9 ){ // print in µs if less than 9ms
      return std::to_string(nbMicroSec_) + "µs";
    }
    nbMicroSec_ /= 1000; // in ms
    if(nbMicroSec_ / nbMicroSec_ < 3){ // print in ms if less than 3s
      return std::to_string(nbMicroSec_) + "ms";
    }
    nbMicroSec_ /= 1000; // in s
    if(nbMicroSec_ / 60 < 5){
      return std::to_string(nbMicroSec_) + "s";
    }
    nbMicroSec_ /= 60; // in min
    if(nbMicroSec_ / 60 < 3){ // print in min
      return std::to_string(nbMicroSec_) + "min";
    }
    nbMicroSec_ /= 60; // in hours
    if(nbMicroSec_ / 24 < 2){ // print in hours
      return std::to_string(nbMicroSec_) + "h";
    }
    nbMicroSec_ /= 24; // in days
    if(nbMicroSec_ / 365 < 2){ // print in hours
      return std::to_string(nbMicroSec_) + "d";
    }
    nbMicroSec_ /= 365; // in days
    return std::to_string(nbMicroSec_) + "y";
  }
  std::string getElapsedTimeSinceLastCallStr( const std::string& key_ ) {
    return GenericToolbox::parseTimeUnit(GenericToolbox::getElapsedTimeSinceLastCallInMicroSeconds(key_));
  }
  std::string getElapsedTimeSinceLastCallStr(int instance_)
  {
    return GenericToolbox::parseTimeUnit(getElapsedTimeSinceLastCallInMicroSeconds(instance_));
  }
  long long getElapsedTimeSinceLastCallInMicroSeconds( const std::string& key_ ) {
    auto newTimePoint = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
      newTimePoint - Internals::_lastTimePointMapStr_[key_]
    );
    Internals::_lastTimePointMapStr_[key_] = newTimePoint;
    return microseconds.count();
  }
  long long getElapsedTimeSinceLastCallInMicroSeconds(int instance_){
    auto newTimePoint = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
      newTimePoint - Internals::_lastTimePointMap_[instance_]
      );
    Internals::_lastTimePointMap_[instance_] = newTimePoint;
    return microseconds.count();
  }
  std::vector<std::string> getOutputOfShellCommand(const std::string& cmd_) {
    // Inspired from: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
    std::array<char, 128> buffer{};
    std::string resultStr;
    std::vector<std::string> output;
#if defined(_WIN32)
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd_.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd_.c_str(), "r"), pclose);
#endif
    if (!pipe) {
//      throw std::runtime_error("popen() failed!");
    }
    else{
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        resultStr += buffer.data();
      }
      output = GenericToolbox::splitString(resultStr, "\n", true);
    }
    return output;
  }

}


// Misc Tools
namespace GenericToolbox{

  std::string getClassName(const std::string& PRETTY_FUNCTION_){
    size_t colons = PRETTY_FUNCTION_.find("::");
    if (colons == std::string::npos)
      return "::";
    size_t begin = PRETTY_FUNCTION_.substr(0, colons).rfind(' ') + 1;
    size_t end = colons - begin;

    return PRETTY_FUNCTION_.substr(begin, end);
  }
  std::string getMethodName(const std::string& PRETTY_FUNCTION_){
    size_t colons = PRETTY_FUNCTION_.find("::");
    size_t begin = PRETTY_FUNCTION_.substr(0, colons).rfind(' ') + 1;
    size_t end = PRETTY_FUNCTION_.rfind('(') - begin;

    return PRETTY_FUNCTION_.substr(begin, end) + "()";
  }

}


// Macro Tools
#define GT_INTERNALS_VA_TO_STR(...) #__VA_ARGS__

#define GT_INTERNALS_ENUM_EXPANDER(enumName_, intOffset_, v1_, ...)\
  enum enumName_ { v1_ =  intOffset_, __VA_ARGS__, enumName_##_OVERFLOW };\
  namespace enumName_##EnumNamespace{\
    static const char *enumNamesAgregate = GT_INTERNALS_VA_TO_STR(v1_, __VA_ARGS__); \
    static const int enumOffSet = intOffset_;\
    static std::vector<std::string> enumNamesDict;\
    inline void buildDictionary(){\
      enumName_##EnumNamespace::enumNamesDict.clear(); enumName_##EnumNamespace::enumNamesDict.emplace_back(""); \
      bool isNameEnded = false;\
      for( unsigned long iChar = 0 ; iChar < strlen(enumNamesAgregate) ; iChar++ ){                             \
        if( enumNamesAgregate[iChar] == ',' ){ enumName_##EnumNamespace::enumNamesDict.emplace_back(""); isNameEnded = false; } \
        else if( enumNamesAgregate[iChar] == ' ' || enumNamesAgregate[iChar] == '=' ){                           \
          if( ! enumName_##EnumNamespace::enumNamesDict.back().empty() ) isNameEnded = true;                     \
        }\
        else if( ! isNameEnded ){ enumName_##EnumNamespace::enumNamesDict.back() += enumNamesAgregate[iChar]; } \
      }\
    }                                                              \
    inline std::vector<std::string>& getEnumNamesList(){           \
      if( enumName_##EnumNamespace::enumNamesDict.empty() ) enumName_##EnumNamespace::buildDictionary(); \
      return enumName_##EnumNamespace::enumNamesDict;              \
    }                                                              \
    inline std::vector<enumName_> getEnumList(){                   \
      std::vector<enumName_> output(enumName_##_OVERFLOW);         \
      for( int iIndex = intOffset_ ; iIndex < enumName_##_OVERFLOW ; iIndex++ ){     \
        output.at(iIndex) = (static_cast<enumName_>(iIndex));      \
      }                                                            \
      return output;\
    }\
    inline std::string toString(int enumValue_, bool excludeEnumName_ = false){      \
      if( enumName_##EnumNamespace::enumNamesDict.empty() ) enumName_##EnumNamespace::buildDictionary();         \
      if( excludeEnumName_ ) return enumName_##EnumNamespace::enumNamesDict[ enumValue_ - intOffset_ ];          \
      return #enumName_ + std::string("::") + enumName_##EnumNamespace::enumNamesDict[ enumValue_ - intOffset_ ];\
    }\
    inline std::string toString(enumName_ enumValue_, bool excludeEnumName_ = false){\
      return enumName_##EnumNamespace::toString(static_cast<int>(enumValue_), excludeEnumName_);       \
    }\
    inline int toEnumInt(const std::string& enumStr_, bool throwIfNotFound_ = false){\
      for( int enumIndex = intOffset_ ; enumIndex < enumName_::enumName_##_OVERFLOW ; enumIndex++ ){ \
        if( enumName_##EnumNamespace::toString(enumIndex) == enumStr_ ){ return enumIndex; } \
        if( enumName_##EnumNamespace::toString(enumIndex, true) == enumStr_ ){ return enumIndex; } \
      }                                                            \
      if( throwIfNotFound_ ){                                      \
        throw std::runtime_error( enumStr_ + " not found in " + #enumName_ );   \
      }                                                             \
      return intOffset_ - 1; /* returns invalid value */\
    }\
    inline enumName_ toEnum(const std::string& enumStr_, bool throwIfNotFound_ = false){                         \
      return static_cast<enumName_>(enumName_##EnumNamespace::toEnumInt(enumStr_, throwIfNotFound_));  \
    }\
  }

#endif //CPP_GENERIC_TOOLBOX_GENERICTOOLBOX_IMPL_H
