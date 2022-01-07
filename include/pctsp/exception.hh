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

class VertexNotFoundException : public std::exception {
    std::string message;
public:
    VertexNotFoundException(const std::string& vertex_str) : message(std::string("Vertex not found: ") + vertex_str) {}
};

class NoSelfLoopFoundException : EdgeNotFoundException {
public:
    NoSelfLoopFoundException(const std::string& vertex_str)
        : EdgeNotFoundException(vertex_str, vertex_str) {}
};

class VertexInWrongSetException : public std::exception {
    std::string message;

public:
    VertexInWrongSetException(const std::string& vertex_str) : message(std::string("Vertex was found in the wrong set: ") + vertex_str) {}

    const char* what() const throw() { return message.c_str(); }

};

class VariableIsNullException : public std::exception {
    private:
        const std::string message = "SCIP variable pointer is NULL.";
    public:
        VariableIsNullException() {};
        const char* what() const throw() {
            return message.c_str();
        }
};

class StepSizeIsTooBig : public std::exception {
    private:
        const std::string message "The step size given to the extension algorithm was larger than the tour that was given.";

    public:
        StepSizeIsToBig() {};
        const char* what() const throw() {
            return message.c_str();
        }
}

#endif