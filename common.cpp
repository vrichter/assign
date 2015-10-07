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

#include "common.h"

using namespace assign;

std::vector<std::string> assign::split(const std::string& data, char delimiter) {
  std::vector<std::string> result;
  std::stringstream stream(data);
  std::string element;
  while(std::getline(stream, element, delimiter)){
      result.push_back(element);
  }
  return result;
}

namespace {

const static uint HIGH_VALUE = 10000;

std::vector<Assignment::ParticipantAssignment> create_assignments(
    const std::vector<Participant>& participants, const uint group_count, const Matrix<double>& solution,
    const Matrix<double>& cost_matrix)
{
  std::vector<Assignment::ParticipantAssignment> result;
  result.reserve(participants.size());
  for(uint i = 0; i < participants.size(); ++i){
    uint column = i;
    std::vector<int> groups;
    for (uint row = 0; row < solution.rows(); ++row){
      if(solution(row,column) == 0){
//        groups.push_back(row % group_count);
        groups.push_back(row);
      }
    }
      result.push_back(Assignment::ParticipantAssignment());
      result.back().participant = participants[i].id;
    if(groups.size() == 0){
      WARNING("Participant " << participants[i].id << " could not be assigned to a group");
      result.back().assigned_groups.push_back(-1);
      result.back().costs.push_back(-1);
    } else {
      if(groups.size() > 1){
        WARNING("Participant " << participants[i].id << " was assigned to multiple groups: " << to_string(groups)
                << " will use the first.");
      }
      result.back().assigned_groups.push_back(groups.front() % group_count);
      result.back().costs.push_back(cost_matrix(groups.front(),i));
    }
  }
  return result;
}

template <typename T, typename U>
std::vector<std::pair<T,U>> swap(std::vector<std::pair<U,T>> vector){
  std::vector<std::pair<T,U>> result;
  result.reserve(vector.size());
  for(auto ut : vector){
    result.push_back(std::pair<T,U>(ut.second,ut.first));
  }
  return result;
}

Matrix<double> create_matrix(const std::vector<Participant>& participants, uint group_count){
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

std::vector<Assignment::ParticipantAssignment> mergeAssignments(
    const std::vector<Assignment::ParticipantAssignment>& first,
    const std::vector<Assignment::ParticipantAssignment>& second)
{
  assert(first.size() == second.size());
  auto result = first;
  for (uint i = 0; i < first.size(); ++i){
    assert(first.at(i).participant == second.at(i).participant);
    result.at(i).assigned_groups.insert(
          result.at(i).assigned_groups.end(),
          second.at(i).assigned_groups.begin(),
          second.at(i).assigned_groups.end());
    result.at(i).costs.insert(
          result.at(i).costs.end(),
          second.at(i).costs.begin(),
          second.at(i).costs.end());
  }
  return result;
}

const std::vector<Assignment::ParticipantAssignment>& findBestAssignment(
    const std::vector<std::vector<Assignment::ParticipantAssignment>>& options)
{
  uint best = 0;
  uint bestCost = std::numeric_limits<Participant::Cost>::max();
  std::vector<Participant::Cost> costs;
  for(uint i = 0; i < options.size(); ++i){
    costs.push_back(0);
    for (auto assignment : options.at(i)){
      for (Participant::Cost cost : assignment.costs){
        costs.back() += cost;
      }
    }
    if(costs.back() < bestCost){
      best = i;
      bestCost = costs.back();
    }
  }
  WARNING("Heuristic found assignments with costs: " << to_string(costs));
  return options.at(best);
}

std::vector<Assignment::ParticipantAssignment> simpleAssignment(const std::vector<Participant>& participants){
  uint group_count = participants.front().preferences.size();
  Matrix<double> cost_matrix = std::move(create_matrix(participants, group_count));
  Matrix<double> solution = cost_matrix;
  Munkres munkres;
  munkres.solve(solution);
  return create_assignments(participants,group_count,solution,cost_matrix);
}

std::vector<Assignment::ParticipantAssignment> assignHeuristicWithWeights(
    const std::vector<Participant>& first,
    std::vector<Participant> second,
    const std::vector<std::pair<uint,uint>>& forbidden)
{
   auto assignment = simpleAssignment(first);
   for(uint i = 0; i < assignment.size(); ++i){
     for(auto f : forbidden){
       if(assignment.at(i).assigned_groups.back() == f.first){
         second.at(i).preferences.at(f.second) += HIGH_VALUE;
       }
     }
   }
   return mergeAssignments(assignment,simpleAssignment(second));
}

std::vector<Assignment::ParticipantAssignment> heuristicAssignment(
    std::vector<Participant> first,
    std::vector<Participant> second,
    std::vector<std::pair<uint,uint>> forbidden)
{
  std::vector<std::vector<Assignment::ParticipantAssignment>> options;
  options.push_back(assignHeuristicWithWeights(first,second,forbidden));
  options.push_back(assignHeuristicWithWeights(second,first,swap(forbidden)));
  return findBestAssignment(options);
}

void splitParticipants(const std::vector<Participant>& src, uint split_before,
                       std::vector<Participant>& dst1, std::vector<Participant>& dst2)
{
  assert(split_before > 0);
  dst1.clear(); dst2.clear();
  dst1.resize(src.size()); dst2.resize(src.size());
  for(uint i = 0; i < src.size(); ++i){
    assert(split_before < src.at(i).preferences.size());
    dst1.at(i).id = src.at(i).id;
    dst2.at(i).id = src.at(i).id;
    dst1.at(i).preferences.reserve(split_before);
    dst2.at(i).preferences.reserve(src.at(i).preferences.size()-split_before);
    for(uint j = 0; j < src.at(i).preferences.size(); ++j){
      if(j  < split_before){
        dst1.at(i).preferences.push_back(src.at(i).preferences.at(j));
      } else {
        dst2.at(i).preferences.push_back(src.at(i).preferences.at(j));
      }
    }
  }
}

} // namespace

Participant Participant::fromCsvLine(const std::string& csv){
  std::vector<std::string> vector = split(csv,',');
  Participant result;
  if(vector.size() > 1){
    result.id = parse<std::string>(vector[0]);
    result.preferences = parse_vector<int>(vector,1);
  }
  return result;
}

std::vector<Participant> Participant::fromCsv(std::istream &csv_stream){
  std::vector<Participant> result;
  uint preferences_count = 0;
  uint line = 1;
  while(std::cin) {
    std::string participant_csv;
    std::getline(csv_stream, participant_csv);
    if(std::cin.eof()) break;
    result.push_back(Participant::fromCsvLine(participant_csv));
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

Assignment::Assignment(std::vector<Participant> participants, uint assignment_split,
                       std::vector<std::pair<uint,uint>> forbidden_combinations)
  : m_Participants(participants), m_AssignmentSplit(assignment_split), m_ForbiddenCombinations(forbidden_combinations)
{}

std::vector<Assignment::ParticipantAssignment> Assignment::solve() const {
  if(m_Participants.empty() || m_Participants.front().preferences.empty()){
    return std::vector<Assignment::ParticipantAssignment>();
  }

  if(m_AssignmentSplit < 1 || m_AssignmentSplit >= m_Participants.front().preferences.size()){ // simple assgnment
    return simpleAssignment(m_Participants);
  } else {
    std::vector<Participant> first,second;
    splitParticipants(m_Participants,m_AssignmentSplit,first,second);
    if(m_ForbiddenCombinations.empty()){
      return mergeAssignments(simpleAssignment(first), simpleAssignment(second));
    } else {
      return heuristicAssignment(first,second,m_ForbiddenCombinations);
    }
  }
}

void assign::print_assignments_csv(const std::vector<Assignment::ParticipantAssignment>& assignments){
  for (auto participant : assignments){
    std::cout << participant.participant;
    for(auto group : participant.assigned_groups){
      std::cout << "," << group;
    }
    for(auto cost : participant.costs){
      std::cout << "," << cost;
    }
    std::cout << std::endl;
  }
}
