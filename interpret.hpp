//
//  interpret.hpp
//  HRM-CCPU
//
//  Copyright © 2015 Lewis Fox.
//  Licenced under The MIT License (MIT)
//

#ifndef interperate_hpp
#define interperate_hpp

#include <cstdint>
#include <vector>
#include "commands.hpp"

Values execute(const Program &coms, const RunInfo &rinfo);

#endif /* interperate_hpp */
