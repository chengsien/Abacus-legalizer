#ifndef _PARSER_h_
#define _PARSER_h_

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "datatype.h"
#endif

void parser_in(string block_file, string net_file, string pl_file);
void parser_block(string block_file);
void parser_net(string block_file);
void parser_pl(string block_file);
void parser_shape(string shape_file);
RECT parser_scl(string scl_file);
// void parser_matlab(string matlab_file);
void write_cell_dat(string outpifle);
void write_pad_dat(string outpifle);
void write_pad_pin_dat(string out_file);
void write_chip_dat(string out_file, RECT rect);
void write_plt(string out_file);
void writ_legal_pl(string out_file);
// 在 parser.h 的最後，write_legal_pl 的附近加入：
void write_matlab_file(string out_file);
