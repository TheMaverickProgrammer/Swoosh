#pragma once
#include <sstream>
#include <cstring>

#include <iostream>

namespace swoosh {
  namespace glsl{

    /*
    On VS you need to add _CRT_SECURE_NO_WARNINGS to your preprocessor definitions
    */
    static std::string formatGLSL(const char* glsl) {
      std::stringstream ss;


      char* input = new char[strlen(glsl) + 1];
      char delim[] = ";";
      strcpy(input, glsl);

      char* line = strtok(input, delim);

      while (line != 0)
      {
        ss << line << ";\n";
        line = strtok(0, delim);
      }

      delete[] input;



      std::string output = ss.str(); //Get the string stream as a std::string
      std::size_t found = output.find('\n'); // Find the first line break, this is the #version decl
      output.erase(found + 1, 1); // Erase this quote char
      output.erase(output.length() - 3, 2); // Remove the quote and last delim char from macro expansion
      return output;
    }
  }
}

#define SWOOSH_EMBED_TO_STR(...) #__VA_ARGS__
#define GLSL(version, ...)  swoosh::glsl::formatGLSL("#version " #version "\n" SWOOSH_EMBED_TO_STR(#__VA_ARGS__))