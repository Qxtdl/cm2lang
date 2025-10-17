#ifndef UTIL_H
#define UTIL_H

/* safe caller for strcmp, if first or second is a null pointer it doesnt call strcmp and returns */
int s_strcmp(const char *first, const char *second);
const char *itoa(int n);
#endif // UTIL_H