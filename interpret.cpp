//
//  interpret.cpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#include "interpret.hpp"
#include <cstdlib>

Values execute(const Program &coms, const RunInfo &rinfo) {
    std::vector<int16_t> out;
    int16_t cval = Empty;
    const Values &input = rinfo.input;
    Values state = rinfo.memory;
    std::vector<int16_t>::const_iterator icur = input.begin(), iend = input.end();
    std::vector<uint16_t>::const_iterator begin = coms.begin(), end = coms.end();
    for (std::vector<uint16_t>::const_iterator cur = begin; cur != end; ++cur) {
    jumpret:
        uint16_t ins = *cur;
        uint8_t com = ins & 0xF;
        uint16_t loc = ins >> 4;
        if (ins >= Commands::COPYTO && ins <= Commands::BUMPDN && loc & IsIndexOf) {
            if (state[loc] & SpecialFlag) std::exit(1);
            loc = state[loc];
        }
        switch (com) {
            case Commands::INBOX:
                if (icur == iend) return out;
                cval = *icur++;
                break;
                
            case Commands::OUTBOX:
                if (cval == Empty) std::exit(1);
                out.push_back(cval);
                cval = Empty;
                break;
                
            case Commands::COPYTO:
                if (cval == Empty) std::exit(1);
                state[loc] = cval;
                break;
                
            case Commands::COPYFROM:
                if (state[loc] == Empty) std::exit(1);
                cval = state[loc];
                break;
                
            case Commands::ADD:
                if (cval & SpecialFlag) std::exit(1);
                if (state[loc] & SpecialFlag) std::exit(1);
                cval += state[loc];
                if (cval > 999) std::exit(1);
                break;
                
            case Commands::SUB:
                if (cval == Empty) std::exit(1);
                if (state[loc] == Empty) std::exit(1);
                if ((cval & SpecialFlag) != (state[loc] & SpecialFlag)) std::exit(1);
                cval -= state[loc];
                if (cval < -999) std::exit(1);
                break;
                
            case Commands::BUMPUP:
                if (state[loc] & SpecialFlag) std::exit(1);
                cval = ++state[loc];
                if (cval == 1000) std::exit(1);
                break;
                
            case Commands::BUMPDN:
                if (state[loc] & SpecialFlag) std::exit(1);
                cval = --state[loc];
                if (cval == -1000) std::exit(1);
                break;
                
            case Commands::JUMP:
            jump:
                cur = begin + loc;
                if (cur >= end) return out;
                goto jumpret;
                
            case Commands::JUMPZ:
                if (cval == Empty) std::exit(1);
                if (cval == 0) goto jump;
                break;
                
            case Commands::JUMPN:
                if (cval == Empty) std::exit(1);
                if (cval < 0) goto jump;
                break;
                
            default:
                std::exit(1);
                break;
        }
    }
    return out;
}
