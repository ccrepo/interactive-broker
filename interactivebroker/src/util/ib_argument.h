//
// ib_argument.h
// interactivebroker
//

#ifndef IB_ARGUMENT_H
#define IB_ARGUMENT_H

#include <iostream>
#include <vector>
#include <atomic>
#include <memory>

namespace ib {
namespace util {

// This class is just enough for working/placeholder. Plenty of room left to improve.

class Argument {
  
public:
  inline bool terminate() const {
    return _terminate;
  }

  inline bool result() const {
    return _result;
  }

  static bool init(int argc, const char *argv[]);

  static const Argument* const argument() {
    return _argument.get();
  }

  static void help() {
    constexpr const char *helpText = ""
        "interactivebroker:"
        "\nclient "
        "\ninteractivebroker -h           # to display this information"
        "\n"
        "\nset environment variable IB_LOGLEVEL to one of "
        "\n'fatal', 'error', 'warn', 'info', 'debug', 'trace' "
        "\ndefaults to 'info'"
        "\n";

    std::cerr << helpText << std::endl;
  }

  virtual ~Argument() {
  }

private:
  Argument(bool terminate, int result,
      const std::vector<std::string> &arguments) :
      _terminate(terminate), _result(result), _arguments(arguments.begin(),
          arguments.end()) {
  }

private:
  const bool _terminate;
  const int _result;
  static std::unique_ptr<Argument> _argument;
  std::vector<std::string> _arguments;

private:
  Argument(const Argument&) = delete;
  Argument& operator=(const Argument&) = delete;

};

} // ns util
} // ns ib

#endif /* IB_ARGUMENT_H */

