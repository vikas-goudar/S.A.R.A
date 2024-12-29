#include <iostream>
using namespace std;

enum p {
  a,b,c,d,e,
  f = 0
};

int main(){
  p var = a;
  cout << var;
  
  return 0;
}
