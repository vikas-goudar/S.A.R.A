#include <iostream>
using namespace std;

enum A {
  a, b
};

constexpr A operator+(A a, A b) { return A(int(a) + int(b)); }


int main() {
  constexpr A var = a;
  constexpr A varr = b;
  constexpr A result = var + varr;


  return 0;
}
