#include <bits/stdc++.h>
#include <chrono>
using namespace std;

#include "parser.h"
#include <iostream>
#include <fstream>
#include "parser.h"
#include "datatype.h"
#include "algo.h"

bool loadFile(string aux_path)
{

    // loading_file parameter
    ifstream fin;
    stringstream ss;
    string in_line;
    string in_trash;

    // reference files
    string node_file;
    string net_file;
    string pl_file;
    string scl_file;
    string shape_file;

    size_t pos = aux_path.find_last_of('/');
    size_t pos1 = aux_path.find_last_of('.');
    if (pos == string::npos)
    {
        cerr << "Invalid path format" << endl;
        return 1;
    }
    cout << "Aux file path: " << aux_path << endl;

    cout << "pos: " << pos << ", pos1: " << pos1 << endl;

    // 取出前綴 (包含最後一個 '/')
    string prefix = aux_path.substr(0, pos + 1);
    string bmk = aux_path.substr(pos + 1, pos1 - (pos + 1));
    benchname = bmk; // 將 bmk 存入全域變數 benchname

    cout << "Prefix: " << prefix << endl;
    cout << "Benchmark name: " << bmk << endl;

    fin.open(aux_path);
    if (!fin)
    {
        // Try adding ../ prefix if file not found
        string alt_path = "../" + aux_path;
        fin.open(alt_path);
        if (fin)
        {
            aux_path = alt_path;
            cout << "Found aux file at: " << aux_path << endl;
        }
        else
        {
            cerr << "Error: cannot open aux file:" << aux_path << endl;
            return false;
        }
    }
    ss.clear();
    getline(fin, in_line); // 從檔案讀一整行到 in_line
    ss.str(in_line);       // 把 in_line 的內容丟進 ss
    ss >> in_trash >> in_trash >> node_file >> net_file >> in_trash >> pl_file >> scl_file >> shape_file;

    parser_block(prefix + node_file);

    // Replace .pl with .gp.pl
    size_t pl_ext_pos = pl_file.find(".pl");
    if (pl_ext_pos != string::npos)
    {
        pl_file.replace(pl_ext_pos, 3, ".gp.pl");
    }
    cout << "prefix + pl_file: " << prefix + pl_file << endl;
    parser_pl(prefix + pl_file);

    parser_net(prefix + net_file);
    parser_shape(prefix + shape_file);
    RECT rect = parser_scl(prefix + scl_file);

    // writ_legal_pl("legal/" + bmk + ".legal.pl");
    //  write_cell_dat("dat/"+bmk+"/"+bmk+"_cell.dat");
    //  write_pad_dat("dat/"+bmk+"/"+bmk+"_pad.dat");
    //  write_pad_pin_dat("dat/"+bmk+"/"+bmk+"_pad_pin.dat");
    //  write_chip_dat("dat/"+bmk+"/"+bmk+"_chip.dat",rect);
    //  write_plt("plt/"+bmk+"/"+bmk+".plt");

    return true;
}

int main(int argc, char *argv[])
{
    // 開始計時
    auto start_time = chrono::high_resolution_clock::now();
    
    if (argc != 2)
    {
        cerr << "Usage: " << "<exe_file>" << " <aux_file>\n";
        return 1;
    }
    loadFile(argv[1]);
    //write_matlab_file("debug_before_legal.m");
    abacus ab;
    ab.slice_row();
    ab.algo();
    writ_legal_pl("legal/" + benchname + ".legal.pl");

    // write_cell_dat("dat/bmk/bmk_cell.dat");
    // write_pad_dat("dat/bmk/bmk_pad.dat");
    // write_pad_pin_dat("dat/bmk/bmk_pad_pin.dat");
    //write_matlab_file("debug_after_legal.m");
    
    // 結束計時並輸出執行時間
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    
    cout << "\n========================================" << endl;
    cout << "Total Execution Time: " << duration.count() << " ms" << endl;
    cout << "Total Execution Time: " << duration.count() / 1000.0 << " seconds" << endl;
    cout << "========================================" << endl;
    
    return 0;
}

// int main(int argc, char *argv[]) {
//     if (argc < 2) {
//         cerr << "Usage: " << argv[0] << " <node_file>\n";
//         return 1;
//     }
//     parser_block(argv[1]);
//     parser_pl("../benchmarks/bmk/bmk.pl");
//     parser_net("../benchmarks/bmk/bmk.nets");
//     RECT rect;
//     //parser_pl("../benchmarks/bmk/bmk.pl");
//     rect = parser_scl("../benchmarks/bmk/bmk.scl");

//     write_cell_dat("dat/bmk/bmk_cell.dat");
//     write_pad_dat("dat/bmk/bmk_pad.dat");
//     write_pad_pin_dat("dat/bmk/bmk_pad_pin.dat");
//     write_chip_dat("dat/bmk/bmk_chip.dat",rect);
//     return 0;
// }
