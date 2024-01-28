#pragma once
#include <vector>
#include <fstream>
#include <iostream>

struct SaveFile {
  std::vector<std::string> names;
  std::vector<int> scores;

  ~SaveFile() {
    std::cout << "inside dconstructor!" << std::endl;
  }

  const bool empty() { return names.empty(); }

  void writeToFile(std::string path) {
    std::ofstream outfile(path, std::ofstream::trunc);

    if (!outfile) { outfile.close(); return; }

    // create some fake highscores if first time playing
    if (names.empty()) {
      for (int i = 0; i < 10; i++) {
        for (auto letter : { 0,1,2 }) {
          outfile << "A";
        }
        
        outfile << (int)rand()%2000;
      }
    }
    else {
      // just update the records
      for (int i = 0; i < names.size(); i++) {
        for (auto letters : { 0,1,2 }) {
          outfile << names[i][letters];
        }

        outfile << scores[i];
      }
    }

    outfile.close();
  }

  void loadFromFile(std::string path) {
    names.clear();
    scores.clear();

    std::ifstream infile(path);
    char name[4];
    int score;

    if (!infile) { infile.close();  return; }
   
    while (infile) {
      for (auto letters : { 0,1,2 }) {
        infile >> name[letters];
      }

      name[3] = '\0';

      infile >> score;

      //std::cout << name << ", " << score << std::endl;
      names.push_back(name);
      scores.push_back(score);
    }

    infile.close();
  }
};