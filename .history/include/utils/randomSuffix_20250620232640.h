#ifndef RANDOM_SUFFIX_H
#define RANDOM_SUFFIX_H
#include <string>

std::string generate_random_string(int length);
int generate_random(int start, int end);
std::string generate_command_from_config(const std::string &config_path, int i);

#endif