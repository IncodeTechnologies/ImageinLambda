//
//  utils.hpp
//  parse_umdfaces
//
//  Created by Alexey Golunov on 14/11/2016.
//  Copyright © 2016 Alexey Golunov. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

inline void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

inline std::string getStringID(int i, int sz) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(sz) << i;
    return ss.str();
}

#endif /* utils_h */
