#pragma once

#include <cassert>
#include <stdlib.h>

#ifdef _DEBUG
#define Assert(x) assert(x)
#else
#define Assert(x) if (!(x)) exit(-1);
#endif