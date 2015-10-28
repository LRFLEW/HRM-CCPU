//
//  extractsvg.cpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <zlib.h>

struct Point {
    uint16_t x, y;
};

struct LineData {
    uint32_t count;
    Point points[256];
};

enum DefineType:uint8_t {
    LABEL,
    COMMENT,
    INVALID,
};

static const std::string names[] = { "LABEL", "COMMENT" };

DefineType getType(std::string s) {
    if (s.compare("LABEL") == 0) return DefineType::LABEL;
    if (s.compare("COMMENT") == 0) return DefineType::COMMENT;
    return DefineType::INVALID;
}

std::vector<uint8_t> decodeBase64(std::istream &in) {
    static const int8_t base64map[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -3, -1, -2, -1, -1,
        -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };
    std::vector<uint8_t> data;
    int8_t temp[4];
    uint8_t filled = 0;
    
    while (in.good()) {
        uint8_t c = in.get();
        temp[filled] = base64map[c];
        if (temp[filled] == -1) std::exit(1);
        if (temp[filled] == -2) continue;
        if (temp[filled++] == -3) return data;
        switch (filled) {
            case 2:
                data.push_back((temp[0] << 2) | (temp[1] >> 4));
                break;
            case 3:
                data.push_back((temp[1] << 4) | (temp[2] >> 2));
                break;
            case 4:
                data.push_back((temp[2] << 6) | (temp[3] >> 0));
                filled = 0;
                break;
                
            default:
                break;
        }
    }
    std::exit(1);
}

int inflateLineData(std::vector<uint8_t> &in, LineData &out) {
    int ret;
    unsigned have;
    z_stream strm;
    
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    
    strm.next_in = in.data();
    strm.avail_in = (uInt) in.size();
    
    strm.avail_out = 1028;
    strm.next_out = (Bytef *) &out;
    ret = inflate(&strm, Z_FINISH);
    switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     /* and fall through */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
        case Z_STREAM_ERROR:
            (void)inflateEnd(&strm);
            return ret;
    }
    have = 1028 - strm.avail_out;
    if (strm.avail_out != 0)
        ret = Z_DATA_ERROR;
    
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

bool getData(std::istream &in, DefineType type, int index, LineData &data) {
    while (in.good()) {
        std::string s;
        in >> s;
        if (s.compare("DEFINE") == 0) {
            in >> s;
            if (s.compare(names[type]) == 0) {
                in >> s;
                int inind = std::stoi(s);
                if (inind == index) {
                    std::vector<uint8_t> input = decodeBase64(in);
                    return inflateLineData(input, data) == Z_OK;
                }
            }
            in.ignore(std::numeric_limits<std::streamsize>::max(), ';');
            continue;
        } else if (s.size() >= 2 && s[0] == '-' && s[1] == '-')
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return false;
}

void writeSVG(LineData &result, std::ostream &out) {
    Point &p = result.points[result.count - 1];
    if(p.x != 0 || p.y != 0) std::exit(1);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 196605 65535\">" << std::endl << "\t<path d=\"M";
    bool delim = false;
    for (uint32_t i=0; i < result.count - 1; ++i) {
        Point &p = result.points[i];
        if (p.x == 0 && p.y == 0) {
            out << "M";
            delim = false;
            continue;
        }
        if (delim) out << "L";
        out << +p.x * 3 << " " << +p.y;
        delim = true;
    }
    out << "\" stroke=\"black\" stroke-width=\"5888\" fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>" << std::endl << "</svg>";
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <LABEL/COMMENT> <Index> [Program] [Saved File]" << std::endl;
        return 0;
    }
    
    DefineType type = getType(argv[1]);
    if (type == DefineType::INVALID) {
        std::cerr << "Unknown type " << argv[1] << std::endl;
        return 1;
    }
    int index;
    try {
        index = std::stoi(argv[2]);
    } catch (std::exception &e) {
        std::cerr << "Invalid index " << argv[2] << std::endl;
        return 1;
    }
    
    LineData result;
    if (argc >= 4 && std::string(argv[3]).compare("--") != 0) {
        std::ifstream in(argv[3]);
        if (!in.good()) {
            std::cerr << "Unable to read file " << argv[3] << std::endl;
            return 1;
        }
        if (!getData(in, type, index, result)) return 1;
    } else {
        if (!getData(std::cin, type, index, result)) return 1;
    }
    
    if (argc >= 5 && std::string(argv[4]).compare("--") != 0) {
        std::ofstream out(argv[4]);
        if (!out.good()) {
            std::cerr << "Unable to write file " << argv[3] << std::endl;
            return 1;
        }
        writeSVG(result, out);
    } else {
        writeSVG(result, std::cout);
        std::cout << std::endl;
    }
    
    return 0;
}
