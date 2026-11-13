#pragma once
// В Release отладочной печати нет совсем: забытый IC ничего не выведет и ничего не вычислит
#if DEBUG
#include "icecream.hpp"
#else
#define IC(...) ((void)0)
#endif
