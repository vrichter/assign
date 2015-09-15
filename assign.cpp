/*********************************************************************
* The MIT License (MIT)                                              *
*                                                                    *
* Copyright (c) 2015 Viktor Richter                                  *
*                                                                    *
* Permission is hereby granted, free of charge, to any person        *
* obtaining a copy of this software and associated documentation     *
* files (the "Software"), to deal in the Software without            *
* restriction, including without limitation the rights               *
* to use, copy, modify, merge, publish, distribute, sublicense,      *
* and/or sell copies of the Software, and to permit persons to whom  *
* the Software is furnished to do so, subject to the following       *
* conditions:                                                        *
*                                                                    *
* The above copyright notice and this permission notice shall be     *
* included in all copies or substantial portions of the Software.    *
*                                                                    *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,    *
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF *
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND              *
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT        *
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,       *
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER      *
* DEALINGS IN THE SOFTWARE.                                          *
*********************************************************************/

#include "hungarian/hungarian.hpp"
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <algorithm>

typedef unsigned int uint;
typedef std::string Id;
typedef std::vector<int> Preferences;
typedef std::pair<Id,Preferences> Entity;

std::vector<std::string> split(const std::string& data, char delimiter) {
  std::vector<std::string> result;
  std::stringstream stream(data);
  std::string element;
  while(std::getline(stream, element, delimiter)){
      result.push_back(element);
  }
  return result;
}

template<typename T>
T parse(const std::string& data){
  std::istringstream istream(data);
  T result;
  istream >> result;
  return result;
}

template<typename T>
std::vector<T> parse_vector(const std::vector<std::string>& data, uint start_at=0){
  std::vector<T> result;
  for(uint i = start_at; i < data.size(); ++i){
    result.push_back(parse<T>(data[i]));
  }
  return result;
}

Entity parseEntity(const std::string& csv){
  std::vector<std::string> vector = split(csv,',');
  Entity result;
  result.first = parse<std::string>(vector.front());
  result.second = parse_vector<int>(vector,1);
  return result;
}

int main(int arg_num, char** args) {

  // process command line arguments
  for(int i = 1; i < arg_num; ++i){
    std::string arg = args[i];
    if(arg == "--help" || arg == "-h"){
      std::cout << "This application uses preferences to sort entities into groups. Input data is read from stdin.\n"\
                << "Usage: assign < preferences.csv\n\n"
                << "Parameters:\n"
                << "\t -h | --help \t\t print this message and leave.\n"
                << "\t -e | --example \t print an example preferences document in the correct format and leave."
                << std::endl;
      std::exit(0);
    } else if (arg == "--example" || arg == "-e"){
      std::cout << "jack,1,5,6,2,3" << std::endl;
      std::cout << "jill,6,2,4,6,2" << std::endl;
      std::cout << "paul,3,3,5,2,1" << std::endl;
      std::cout << "will,2,9,7,4,3" << std::endl;
      std::cout << "jenn,5,6,3,7,1" << std::endl;
      std::exit(0);
    }
  }

  // process data from sidin into vectors
  std::vector<Id> entity_ids;
  std::vector<Preferences> entity_preferences;

  uint group_count = 0;
  uint line = 1;
  while(std::cin) {
    std::string entity_csv;
    std::getline(std::cin, entity_csv);
    if(std::cin.eof()) break;
    Entity entity = parseEntity(entity_csv);
    entity_ids.push_back(entity.first);
    entity_preferences.push_back(entity.second);
    if(line == 1){
      group_count = entity.second.size();
    } else  {
      if(entity.second.size() != group_count){
        std::cerr << "Error: Group count in line #" << line << " is defferent from previous. "
                  << entity.second.size() << " != " << group_count << "(previous)." << std::endl;
        std::exit(-1);
      }
    }
    ++line;
  }

  for(uint i = 0;  i < entity_ids.size(); ++i){
    std::cout << entity_ids[i] << ":\t";
    for (int j = 0; j < entity_preferences[i].size(); ++j){
      std::cout << entity_preferences[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // create cost matrix
  uint group_size = (entity_ids.size() / group_count);
  if(entity_ids.size() % group_count) ++group_size;
  std::vector<std::vector<int> > matrix;
  matrix.resize(entity_ids.size(),std::vector<int>(group_count*group_size,0));
  for(uint i = 0; i < entity_ids.size(); ++i){
    std::vector<int>& row = matrix[i];
    for(uint j = 0; j < group_size; ++j){
      std::copy(entity_preferences[i].begin(), entity_preferences[i].end(), row.begin()+(j*group_count));
    }
  }

  /* initialize the hungarian_problem using the cost matrix*/
  Hungarian hungarian(matrix, matrix.size(),matrix.front().size(), HUNGARIAN_MODE_MINIMIZE_COST) ;

  /* some output */
  fprintf(stderr, "cost-matrix:");
  hungarian.print_cost();

  /* solve the assignement problem */
  hungarian.solve();

  /* some output */
  fprintf(stderr, "assignment:");
  hungarian.print_assignment();


  return 0;
}

