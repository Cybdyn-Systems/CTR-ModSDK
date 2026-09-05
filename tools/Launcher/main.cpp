#include "app.h"

#ifndef _DEBUG
#include <cstdio>
#include <exception>
#endif

int main(int argc, char* argv[])
{
  App app;
  app.Init();
#ifdef _DEBUG
  app.Run();
#else
  // Report what went wrong instead of exiting silently - a release build that
  // dies without a word is impossible to tell apart from a clean exit, which is
  // what made release and debug look like they behaved differently.
  try { app.Run(); }
  catch (const std::exception& e) { std::fprintf(stderr, "Fatal error: %s\n", e.what()); }
  catch (...) { std::fprintf(stderr, "Fatal error: unknown exception\n"); }
#endif
  app.Close();
  return 0;
}
