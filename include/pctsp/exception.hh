#ifndef __PCTSP_EXCEPTION__
#define __PCTSP_EXCEPTION__
// Custom exceptions for the TSP with Profits

#include <exception>
#include <filesystem>
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

    EdgeNotFoundException(const unsigned long first_vertex, const unsigned long second_vertex)
        : EdgeNotFoundException(std::to_string(first_vertex), std::to_string(second_vertex)) {}

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
        const std::string message = "The step size given to the extension algorithm was larger than the tour that was given.";

    public:
        StepSizeIsTooBig() {};
        const char* what() const throw() {
            return message.c_str();
        }
};

class TargetVertexFound : public std::exception {
    private:
        const std::string message = "The target vertex has been found.";

    public:
        TargetVertexFound() {};
        const char* what() const throw() {
            return message.c_str();
        }
};

class FileDoesNotExistError : public std::filesystem::filesystem_error {
    private:
        const std::string message = "File does not exist: ";
    
    public:
        FileDoesNotExistError(const std::string& what_arg, const std::filesystem::path& p1, std::error_code ec) : std::filesystem::filesystem_error(what_arg, p1, ec) {};

        FileDoesNotExistError(std::filesystem::path& filepath): FileDoesNotExistError(
            message, filepath, std::make_error_code(std::errc::no_such_file_or_directory)
        ) {};            
        // const char* what() const throw() {
        //     return message.c_str();
        // }
};

#endif