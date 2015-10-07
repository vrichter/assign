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

#pragma once

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

namespace assign {

typedef unsigned int uint;
typedef int GroupId;
typedef std::map<std::string,std::string> ProgArgs;

struct Participant {
  typedef int Cost;
  typedef std::string Id;

  Id id = "";
  std::vector<Cost> preferences;

  static Participant fromCsvLine(const std::string& csv);
  static std::vector<Participant> fromCsv(std::istream& csv_stream);
};

class Assignment{
public:

  struct ParticipantAssignment{
    Participant::Id participant = "";
    std::vector<Participant::Cost> costs;
    std::vector<GroupId> assigned_groups;
  };

  Assignment(std::vector<Participant> participants,
             uint assignment_split = 0,
             std::vector<std::pair<uint,uint>> forbidden_combinations = std::vector<std::pair<uint,uint>>());

  std::vector<ParticipantAssignment> solve() const;

private:
  const std::vector<Participant> m_Participants;
  const uint m_AssignmentSplit;
  const std::vector<std::pair<uint,uint>> m_ForbiddenCombinations;
};

std::vector<std::string> split(const std::string& data, char delimiter);
void print_assignments_csv(const std::vector<Assignment::ParticipantAssignment>& assignments, bool print_costs);

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

} // namespace assign
