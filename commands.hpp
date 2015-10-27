//
//  commands.hpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#ifndef commandsg_hpp
#define commandsg_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

const int16_t SpecialFlag = 0x4000;
const int16_t Empty       = 0x7FFF;

enum Commands:uint8_t {
    INBOX,
    OUTBOX,
    COPYTO,
    COPYFROM,
    ADD,
    SUB,
    BUMPUP,
    BUMPDN,
    JUMP,
    JUMPZ,
    JUMPN,
    COMMENT,
    LABEL,
    LCOMMENT,
    DEFINE,
    INVALID
};

const uint16_t IsIndexOf = (1 << 0xC);

typedef std::vector<uint16_t> Program;
typedef std::vector<int16_t> Values;

struct RunInfo {
    Values memory;
    Values input;
};

std::pair<Program, RunInfo> setInstructions(std::istream &in);

#endif /* commandsg_hpp */
