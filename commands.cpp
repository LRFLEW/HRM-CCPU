//
//  commands.cpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#include "commands.hpp"
#include "commandsgen.ipp"
#include <cstdlib>
#include <string>
#include <unordered_map>

uint8_t getCommand(const std::string &str) {
    if (str[0] == '-' && str[1] == '-') return 0xD;
    if (str[str.size() - 1] == ':') return 0xC;
    const Instruction *i = InstructionLookup::get(str.c_str(), (unsigned int) str.size());
    if (i == nullptr) return 0xF;
    return i->cmd;
}

int16_t getIndex(const std::string &str) {
    std::string::const_iterator begin = str.begin(), end = str.end();
    bool deref = *begin == '[' && *(end - 1) == ']';
    if (deref) ++begin, --end;
    int16_t v = 0;
    for (std::string::const_iterator i = begin; i < end; ++i) {
        const char c = *i;
        if (c < '0' || c > '9') return -1;
        v *= 10;
        v += c - '0';
        if (v >= 0x800) return -1;
    }
    return (deref << 0xC) | v;
}

int16_t getIndexNoDeref(const std::string &str) {
    int16_t v = 0;
    for (const char c : str) {
        if (c < '0' || c > '9') return -1;
        v *= 10;
        v += c - '0';
        if (v >= 0x800) return -1;
    }
    return v;
}

int16_t getValue(const std::string &str) {
    if (str.size() == 1 && str[0] >= 'A' && str[0] <= 'Z') return SpecialFlag | (str[0] - 'A');
    std::string::const_iterator begin = str.begin(), end = str.end();
    bool neg = *begin == '-';
    if (neg) ++begin;
    int16_t v = 0;
    for (std::string::const_iterator i = begin; i < end; ++i) {
        const char c = *i;
        if (c < '0' || c > '9') return Empty;
        v *= 10;
        v += c - '0';
        if (v > 999) return Empty;
    }
    return (neg ? -1 : 1) * v;
}

bool verifyNumber(const std::string &str) {
    for (const char &c : str)
        if (c < '0' || c > '9')
            return false;
    return true;
}

std::pair<Program, RunInfo> setInstructions(std::istream &in) {
    Program coms;
    RunInfo rinfo;
    bool rinfowriten = false;
    std::unordered_map<std::string, Program::value_type> locations;
    std::unordered_map<std::string, Program::value_type> unfound;
    
    while (in.good()) {
        std::string s;
        in >> s;
        if (s.size() == 0) continue;
        uint8_t com = getCommand(s);
        if (com < 0xB) {
            int16_t index = 0;
            if (com >= 0x8) { // Jumps
                in >> s;
                if (locations.count(s)) index = locations.at(s);
                else {
                    if (unfound.count(s)) index = unfound.at(s);
                    else index = ~0;
                    unfound[s] = coms.size();
                }
            } else if (com >= 0x2) { // Indexed
                in >> s;
                index = getIndex(s);
                if (index < 0)
                    std::exit(1);
            }
            coms.push_back((((uint16_t) index) << 4) | ((uint16_t) com));
            if (coms.size() >= 0x1000) std::exit(1);
        } else { // Special
            switch (com) {
                case Commands::COMMENT:
                    in >> s;
                    if (!verifyNumber(s)) std::exit(1);
                    break;
                    
                case Commands::LABEL:
                    s = s.substr(0, s.size() - 1);
                    locations[s] = coms.size();
                    if (unfound.count(s)) {
                        uint16_t loc = unfound.at(s);
                        do {
                            uint16_t &ins = coms[loc];
                            loc = (ins >> 4);
                            ins = (coms.size() << 4) | (ins & 0xF);
                        } while (loc != ((uint16_t) ~0 >> 4));
                        unfound.erase(s);
                    }
                    break;
                    
                case Commands::LCOMMENT:
                    if (s.size() == 3 && s[2] == '#') { // RunInfo
                        if (rinfowriten) std::exit(1);
                        in >> s;
                        int16_t memsize = getIndexNoDeref(s);
                        if (memsize < 0) std::exit(1);
                        rinfo.memory = Values(memsize, Empty);
                        in >> s;
                        while (s.size() != 1 || s[0] != ':') {
                            if (s.compare("#--") == 0) goto done;
                            int16_t ind = getIndexNoDeref(s);
                            if (ind < 0) std::exit(1);
                            in >> s;
                            uint16_t val = getValue(s);
                            if (val == Empty) std::exit(1);
                            rinfo.memory[ind] = val;
                            in >> s;
                        }
                        in >> s;
                        while (s.compare("#--") != 0) {
                            uint16_t val = getValue(s);
                            if (val == Empty) std::exit(1);
                            rinfo.input.push_back(val);
                            in >> s;
                        }
                    done:
                        rinfo.input.shrink_to_fit();
                        rinfowriten = true;
                    }
                    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                    
                case Commands::DEFINE:
                    in.ignore(std::numeric_limits<std::streamsize>::max(), ';');
                    if (in.eof()) std::exit(1);
                    break;
                    
                default:
                    std::exit(1);
                    break;
            }
        }
    }
    if (unfound.size() != 0) std::exit(1);
    coms.shrink_to_fit();
    return std::pair<Program, RunInfo>(coms, rinfo);
}
