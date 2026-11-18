#ifndef UTIL_H
#define UTIL_H

// Helper utilities for clamping, string comparisons, and screen control
int clampint(int val, int minVal, int maxVal);
void trimwhitespace(char* str);
bool stringequals(const char* a, const char* b);
void printheader(const char* title);
void clearscreen();

#endif // UTIL_H
