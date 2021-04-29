#ifndef __PCTSP_EXCEPTION__
#define __PCTSP_EXCEPTION__
// Custom exceptions for the TSP with Profits

#include <exception>
#include <string>

using namespace std;

struct NoGainVertexFoundException : public std::exception {
    const char* what() const throw() {
        return "Could not calculate the unitary gain for any vertices";
    }
};

class EdgeNotFoundException : public std::exception {
    std::string message;

public:
    EdgeNotFoundException(const std::string& first_vertex_str,
        const std::string& second_vertex_str)
        : message(std::string("No edge between vertices: ") + first_vertex_str +
            " and " + second_vertex_str) {}

    const char* what() const throw() { return message.c_str(); }
};

#endif