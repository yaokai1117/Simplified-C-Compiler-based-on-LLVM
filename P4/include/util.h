#ifndef _UTIL_HH_
#define _UTIL_HH_

// debug info is in blue
// you can hide/show debug info in main.cc
#ifdef DEBUG
#define debug(format, args...) fprintf(stdout, "\033[36m" format "\033[0m", ##args)
#else
#define debug(format, args...)
#endif

// error info is in red
#define error(format, args...) fprintf(stderr, "\033[31merror: " format "\033[0m", ##args)

// warning info is in orange
#define warning(format, args...) fprintf(stderr, "\033[33mwarning: " format "\033[0m", ##args)

extern bool handle_opt(int argc, char** argv);

#endif
