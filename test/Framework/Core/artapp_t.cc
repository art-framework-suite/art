extern "C" { int artapp(int argc, char* argv[]); }

#include <assert.h>

int main(int argc, char* argv[])
{
  int rc = artapp(argc, argv);
  assert (rc == 0);
}
