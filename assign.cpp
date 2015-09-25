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

#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include "munkres.h"
#include "matrix.h"

#define WARNING(TXT) { std::cout << "WARNING: " << TXT<< std::endl; }
#define ERROR(ERRVAL,TXT) { std::cout << "ERROR: " << TXT << std::endl; exit(ERRVAL); }

struct Participant {
  std::string id = "";
  std::vector<int> preferences;
};

typedef unsigned int uint;
typedef int Cost;
typedef std::string Id;
typedef int GroupId;
typedef std::map<std::string,std::string> ProgArgs;

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

Participant parseParticipant(const std::string& csv){
  std::vector<std::string> vector = split(csv,',');
  Participant result;
  if(vector.size() > 1){
    result.id = parse<std::string>(vector[0]);
    result.preferences = parse_vector<int>(vector,1);
  }
  return result;
}

void process_args(int arg_num, char** args, ProgArgs& prog_args_dst){
  // set default values
  for(int i = 1; i < arg_num; ++i){
    std::string arg = args[i];
    if(arg == "--help" || arg == "-h"){
      std::cout << "This application uses preferences to sort participants into groups. Input data is read from stdin.\n"\
                << "Usage: assign < preferences.csv\n\n"
                << "Parameters:\n"
                << "\t -h | --help \t\t print this message and leave.\n"
                << "\t -e | --example \t\t print an example preferences document in the correct format and leave."
                << std::endl;
      std::exit(0);
    } else if (arg == "--example" || arg == "-e"){
      std::cout << "jack,1,5,6,2,3" << std::endl;
      std::cout << "jill,6,2,4,6,2" << std::endl;
      std::cout << "paul,3,3,5,2,1" << std::endl;
      std::cout << "jill,2,9,7,4,3" << std::endl;
      std::cout << "jenn,5,6,3,7,1" << std::endl;
      std::exit(0);
    }
  }
}

std::vector<Participant> parse_csv_data(){
  std::vector<Participant> result;
  uint preferences_count = 0;
  uint line = 1;
  while(std::cin) {
    std::string participant_csv;
    std::getline(std::cin, participant_csv);
    if(std::cin.eof()) break;
    result.push_back(parseParticipant(participant_csv));
    if(line == 1){
      preferences_count = result.front().preferences.size();
    } else  {
      if(result.back().preferences.size() != preferences_count){
        ERROR(-1,"Preferences count in line #" << line << " is defferent from previous. "
                  << result.back().preferences.size() << " != " << preferences_count << "(previous).");
      }
    }
    ++line;
  }
  return result;
}

template<typename T>
std::string to_string(std::vector<T> vector){
  if(vector.size() == 0){
    return "[ ]";
  } else {
    std::stringstream stream;
    stream << "[ " << vector.front();
    for(uint i = 1; i < vector.size(); ++i){
      stream << ", " << vector[i];
    }
    stream << " ]";
    return stream.str();
  }
}

void print_data(const std::vector<Participant>& participants, std::ostream& dst){
  for(auto participant : participants){
    dst << "Participant: " << participant.id << std::endl;
    dst << "\tPreferences: " << to_string(participant.preferences) << "\n";
  }
}

template<typename T>
void print_matrix(const Matrix<T>& matrix, std::ostream& stream){
  // Display solved matrix.
  for ( int row = 0 ; row < matrix.rows() ; row++ ) {
    for ( int col = 0 ; col < matrix.columns() ; col++ ) {
      stream << matrix(row,col) << ",";
    }
    stream << "\n";
  }
  stream << std::endl;
}

Matrix<double> create_matrix(std::vector<Participant>& participants, uint group_count){
  uint group_size = participants.size() / group_count;
  if(participants.size() % group_count) { ++group_size; } // rounding up
  uint columns = participants.size(); // true count of entities
  uint rows = group_count * group_size;

  Matrix<double> result(rows,columns);

  for(uint row = 0; row < rows; ++row){
    for(uint column = 0; column < columns; ++column){
      uint looped_row = row % group_count;
      result(row, column) = participants[column].preferences[looped_row]; // transposing
    }
  }
  return result;
}

void print_assignments_csv(const std::vector<Participant>& participants, const std::vector<GroupId>& groups){
  for(uint i = 0; i < participants.size(); ++i){
    std::cout << participants.at(i).id << "," << groups.at(i) << std::endl;
  }
}

void print_assignments_json(const std::vector<Participant>& participants, const std::vector<GroupId>& groups){
  std::cout << "{\n    \"assignment\": [\n";
  for(uint i = 0; i < participants.size(); ++i){
    std::cout << "        {\n"
              << "            \"id\":         \"" << participants.at(i).id         << "\",\n"
              << "            \"group\":      " << groups.at(i) << "\n        }";
    if(i+1 < participants.size()){
      std::cout << ",\n";
    } else {
      std::cout << "\n";
    }
  }
  std::cout << "    ]\n}" << std::endl;
}

std::vector<GroupId> create_assignments(const std::vector<Participant>& participants, const uint group_count, const Matrix<double>& solution){
  std::vector<GroupId> result;
  result.reserve(participants.size());
  for(uint i = 0; i < participants.size(); ++i){
    uint column = i;
    std::vector<int> groups;
    for (uint row = 0; row < solution.rows(); ++row){
      if(solution(row,column) == 0){
        groups.push_back(row % group_count);
      }
    }
    if(groups.size() == 0){
      WARNING("Participant " << participants[i].id << " could not be assigned to a group");
      result.push_back(-1);
    } else {
      if(groups.size() > 1){
        WARNING("Participant " << participants[i].id << " was assigned to multiple groups: " << to_string(groups)
                << " will use the first.");
      }
      result.push_back(groups.front());
    }
  }
  return result;
}

int main(int arg_num, char** args) {

  ProgArgs prog_args;
  process_args(arg_num,args,prog_args);

  auto participants = parse_csv_data();

  uint group_count = participants.front().preferences.size();

  Matrix<double> cost_matrix = std::move(create_matrix(participants, group_count));
  Matrix<double> solution = cost_matrix;
  Munkres munkres;
  munkres.solve(solution);

  auto assignments = create_assignments(participants,group_count,solution);
  if(assignments.size() != participants.size()){
    ERROR(-1,"Assignment went wrong. The reason is probably bad programming.");
  }

  print_assignments_csv(participants,assignments);

  return 0;
}

