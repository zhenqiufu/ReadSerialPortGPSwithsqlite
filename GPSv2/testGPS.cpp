#include <sqlite_modern_cpp.h>
#include <cstdio>
#include <iostream>
#include "gps.h"
using namespace sqlite;
using namespace std;

int main() {
  try {
    run();

  } catch (exception& e) {
    cerr << "Unhandled Exception: " << e.what() << endl;
  }
}
