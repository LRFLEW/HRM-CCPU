//
//  main.cpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "commands.hpp"
#include "interpret.hpp"

const inline uint16_t L(const char c) {
    return (uint16_t) (c - 'A') + SpecialFlag;
}

const std::string getName(int16_t v) {
    if (v & SpecialFlag) {
        return std::string({(char) ((v & ~SpecialFlag) + 'A'), '\0'});
    } else {
        return std::to_string(v);
    }
}

int main(const int argc, const char * argv[]) {
    std::pair<Program, RunInfo> prog;
    if (argc >= 2 && strcmp(argv[1], "--") != 0) {
        std::ifstream in(argv[1]);
        if (!in.good()) {
            std::cerr << "Unable to read file: " << argv[1] << std::endl;
            return 1;
        }
        prog = setInstructions(in);
    } else {
        prog = setInstructions(std::cin);
    }
    Values output = execute(prog.first, prog.second);
    
    bool f = false;
    for (int16_t v : output) {
        if (f) std::cout << ", ";
        std::cout << getName(v);
        f = true;
    }
    std::cout << std::endl;
    
    return 0;
}
