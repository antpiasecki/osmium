#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include <osmium-html/parser.hh>

int main(int /*argc*/, char *argv[]) {
  std::ifstream file(argv[1]);
  assert(file.good());

  std::stringstream ss;
  ss << file.rdbuf();

  std::cout << parse(ss.str())->dump(0) << std::endl;
}