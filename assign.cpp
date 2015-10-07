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

void process_args(int arg_num, char** args, ProgArgs& prog_args_dst){
  // set default values
  for(int i = 1; i < arg_num; ++i){
    std::string arg = args[i];
    if(arg == "--help" || arg == "-h"){
      std::cout << "This application uses preferences to sort participants into groups. Input data is read from stdin.\n"\
                << "Usage: assign < preferences.csv\n\n"
                << "Parameters:\n"
                << "\t -h | --help \t\t print this message and leave.\n"
                << "\t -e | --example \t\t print an example preferences document in the correct format and leave.\n"
                << "\t -c       | --costs           \t print the calculated costs in addition to the assignment. costs are .\n"
                << "\t                              \t appended after all assignments.\n"
                << std::endl;
      std::exit(0);
    } else if (arg == "--example" || arg == "-e"){
      std::cout << "jack,1,5,6,2,3" << std::endl;
      std::cout << "jill,6,2,4,6,2" << std::endl;
      std::cout << "paul,3,3,5,2,1" << std::endl;
      std::cout << "mila,2,9,7,4,3" << std::endl;
      std::cout << "jenn,5,6,3,7,1" << std::endl;
      std::exit(0);
    } else if (arg == "--costs" || arg == "-c"){
      prog_args_dst["costs"] = "1";
    } else {
      ERROR(1,"Unknown command line parameter' " << arg);
    }
  }
}

int main(int arg_num, char** args) {

  ProgArgs prog_args;
  process_args(arg_num,args,prog_args);

  auto participants = Participant::fromCsv(std::cin);

  Assignment problem(participants);
  auto assignments = problem.solve();

  print_assignments_csv(assignments,!prog_args["costs"].empty());

  return 0;
}

