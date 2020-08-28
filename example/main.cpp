//
// Created by Nadrino on 28/08/2020.
//

#include <string>
#include <iostream>

#include <GenericToolbox.h>

using namespace std;

int main(){

  // String management
  string str("This is a simple message.");
  cout << "Does str contains \"simple message\"? " << GenericToolbox::doesStringContainsSubstring(str, "simple message") << endl;
  cout << "Remove the caps chars: " << GenericToolbox::toLowerCase(str);
  auto splitVec = GenericToolbox::splitString(str, " ");
  cout << "Those are the words in str: "; GenericToolbox::printVector(splitVec);
  cout << "Let's_join_the_vector_element:_" << GenericToolbox::joinVectorString(splitVec, "_") << endl;
  cout << GenericToolbox::formatString("I would %s to create a printf-style string %i, %p", "like", 4, &splitVec) << endl;


  // IO examples
  cout << "Current working directory: " << GenericToolbox::getCurrentWorkingDirectory() << endl;
  cout << "Is this a directory? " << GenericToolbox::doesPathIsFolder("../include") << endl;
  cout << "Is this a file? " << GenericToolbox::doesPathIsFile("../include/GenericToolbox.h") << endl;
  cout << "Its size: " << GenericToolbox::parseSizeUnits(GenericToolbox::getFileSizeInBytes("../include/GenericToolbox.h")) << endl;
  cout << "Hash of this file: " << GenericToolbox::getHashFile("../include/GenericToolbox.h") << endl;
  cout << "Are they identical?: " << GenericToolbox::doFilesAreTheSame("../include/GenericToolbox.h","../include/GenericToolbox_impl.h") << endl;


  // Hardware
  cout << "Process takes this amount of RAM: " << GenericToolbox::parseSizeUnits(GenericToolbox::getProcessMemoryUsage()) << endl;
  cout << "Process has taken this amount of RAM at max: " << GenericToolbox::parseSizeUnits(GenericToolbox::getProcessMaxMemoryUsage()) << endl;


  return EXIT_SUCCESS;
}
