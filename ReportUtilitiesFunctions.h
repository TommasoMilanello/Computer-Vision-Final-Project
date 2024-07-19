#ifndef REPORT_UTILITIES_FUNCTIONS_H
#define REPORT_UTILITIES_FUNCTIONS_H

#include <sstream>
#include <vector>
#include <string>

void writeFrameResults(std::stringstream& ss, int frameNumber, const std::vector<float>& results);

void finalizeAndWriteToFile(const std::string& filename, const std::stringstream& ss);

#endif // REPORT_UTILITIES_FUNCTIONS_H
