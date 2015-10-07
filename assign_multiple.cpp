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
#include "common.h"
#include "munkres.h"
#include "matrix.h"

using namespace assign;

std::string read_next(int& i, int arg_num, char** args, const std::string& argname){
  if(i+1 >= arg_num){
    ERROR(1, "Passed parameter " << argname << " without argument.");
  }
  ++i;
  std::string value = args[i];
  if(!value.empty() && value.at(0) == '-'){
    ERROR(1, "Passed parameter " << argname << " without argument.");
  }
  return value;
}

void process_args(int arg_num, char** args, ProgArgs& prog_args_dst){
  // set default values
  for(int i = 1; i < arg_num; ++i){
    std::string arg = args[i];
    if(arg == "--help" || arg == "-h"){
      std::cout << "This application uses preferences to sort participants into groups. Input data is read from stdin.\n"\
                << "Usage: assign < preferences.csv\n\n"
                << "Parameters:\n"
                << "\t -h       | --help            \t print this message and leave.\n"
                << "\t -e       | --example         \t print an example preferences document in the correct format and leave.\n"
                << "\t -m <arg> | --multiple <arg>  \t Assign participants to multiple groups. Splits the assignment multiple \n"
                << "\t                              \t independent assignment problems. <arg> before which preference to split.\n"
                << "\t                              \t counting from 0.\n"
                << "\t                              \t integers determining where to split the preferences.\n"
                << "\t -x <arg> | --exclusive <arg> \t may be used in combination with -m to make assignments with mutualy \n"
                << "\t                              \t exclusive groups.\n"
                << std::endl;
      std::exit(0);
    } else if (arg == "--example" || arg == "-e"){
      std::cout << "jack,1,5,6,2,3" << std::endl;
      std::cout << "jill,6,2,4,6,2" << std::endl;
      std::cout << "paul,3,3,5,2,1" << std::endl;
      std::cout << "jill,2,9,7,4,3" << std::endl;
      std::cout << "jenn,5,6,3,7,1" << std::endl;
      std::exit(0);
    } else if (arg == "--multiple" || arg == "-m"){
      std::string next = read_next(i, arg_num, args, arg);
      prog_args_dst["multiple"] = next;
    } else if (arg == "--exclusive" || arg == "-x"){
      std::string next = read_next(i, arg_num, args, arg);
      prog_args_dst["exclusive"] = next;
    } else {
      ERROR(1,"Unknown command line parameter' " << arg);
    }
  }
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

  auto participants = Participant::fromCsv(std::cin);

  std::vector<std::string> excludeStrings = split(prog_args["exclusive"],',');
  std::vector<std::pair<uint,uint>> exclude;
  for (auto excludeString : excludeStrings){
    auto exclusion = split(excludeString,'-');
    if(exclusion.size() == 2){
      exclude.push_back(std::pair<uint,uint>(parse<uint>(exclusion.front()),
                                             parse<uint>(exclusion.back())));
    }
  }

  auto split = parse<uint>(prog_args["multiple"]);

  Assignment problem(participants,split,exclude);
  auto assignments = problem.solve();

  print_assignments_csv(assignments);

  return 0;
}

