#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

void parser_block(string node_file)
{
    ifstream infile(node_file);
    if (!infile.is_open())
    {
        cerr << "Error opening file.\n";
    }
    string line;
    getline(infile, line);
    // unsigned int numSoft = 0, numHard = 0, numTerm = 0;
    unsigned int numNodes = 0, numTerminals = 0;
    int dick = 0;
    while (getline(infile, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        if (line.find("NumNodes") != string::npos)
        {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numNodes;
            cout << numNodes << endl;
            continue;
        }
        else if (line.find("NumTerminals") != string::npos)
        {
            istringstream iss(line);
            string tmp;
            iss >> tmp >> tmp >> numTerminals;
            cout << numTerminals << endl;
            continue;
        } // break;  // done with header
        istringstream iss(line);
        string tmp, name;
        double w, h;
        // if (!(iss >> tmp >> tmp >> tmp)) continue;
        iss >> name >> w >> h >> tmp;
        // cout << tmp << endl;
        // cout << name;
        // if (name[0] == 'o') {
        MODULE *module = new MODULE;
        module_map[name] = module;
        module_map[name]->name = name;
        module_map[name]->W_H.x = w;
        module_map[name]->W_H.y = h;
        if (tmp == "terminal" || tmp == "terminal_NI")
        {
            module_map[name]->fixed = true;
            dick++;
        }
        // MODULES.push_back(cell);
        // cout << "name: " << module_map[name]->name << " width: " << module_map[name]->W_H.x << " Height: " << module_map[name]->W_H.y  << "Terimal: " << module_map[name]->fixed << endl;

        //}
    }
    cout << "Total Modules: " << module_map.size() << endl;
    cout << "Total Terminals: " << dick << endl;
    return;
}

void parser_shape(string shape_file)
{
    ifstream infile(shape_file);
    if (!infile.is_open())
    {
        cerr << "Error opening shapes file: " << shape_file << "\n";
        return;
    }
    
    string line;
    string current_node_name = "";
    int num_rectangles = 0;
    
    while (getline(infile, line))
    {
        // 跳過空行和註釋
        if (line.empty() || line[0] == '#')
            continue;
            
        // 跳過 header 行
        if (line.find("shapes") != string::npos || 
            line.find("NumNonRectangularNodes") != string::npos)
            continue;
            
        istringstream iss(line);
        string first_word;
        iss >> first_word;
        
        // 檢查是否為形狀定義（Shape_X 開頭）
        if (first_word.find("Shape_") == 0 && !current_node_name.empty())
        {
            // 讀取矩形定義: Shape_X x y width height
            double x, y, width, height;
            
            if (iss >> x >> y >> width >> height)
            {
                // 創建 RECT 並加入到模組的 rects 向量中
                RECT rect;
                rect.lb.x = x;
                rect.lb.y = y;
                rect.ur.x = x + width;
                rect.ur.y = y + height;
                
                module_map[current_node_name]->rects.push_back(rect);
            }
        }
        else
        {
            // 檢查是否為新的節點定義（格式: "nodename : count"）
            string colon;
            iss >> colon;
            
            if (colon == ":")
            {
                // 格式: "nodename : num_rectangles"
                current_node_name = first_word;
                iss >> num_rectangles;
                
                // 檢查該模組是否存在
                if (module_map.find(current_node_name) == module_map.end())
                {
                    cerr << "Warning: Node " << current_node_name << " in .shapes file not found in module_map\n";
                    current_node_name = "";
                    continue;
                }
            }
        }
    }
    
    // 統計有多少個非矩形模組
    int nr_count = 0;
    int total_rects = 0;
    for (const auto& item : module_map)
    {
        if (!item.second->rects.empty())
        {
            nr_count++;
            total_rects += item.second->rects.size();
        }
    }
    
    cout << "Loaded " << total_rects << " shapes for " << nr_count << " non-rectangular modules\n";
    infile.close();
}

void parser_net(string net_file)
{
    ifstream infile(net_file);
    if (!infile.is_open())
    {
        cerr << "Error opening file: " << net_file << "\n";
        return;
    }
    string line;
    getline(infile, line);
    getline(infile, line);
    getline(infile, line);

    int Num_nets = 0;
    wHPWL = 0.0;
    while (getline(infile, line))
    {
        if (line != "")
        {
            istringstream iss(line);
            string name = "";
            string trash = "";
            string num;
            iss >> name >> trash >> num;
            cout << name << endl;
            if (name == "NumNets")
            {
                Num_nets = stoi(num);
                cout << "NumNets :" << Num_nets << endl;
                // exit(1);
                break;
            }
        }
    }
    getline(infile, line);
    while (getline(infile, line))
    {
        if (line != "")
        {
            istringstream iss(line);
            string type_name;
            string trash = "";
            string node_num;
            string net_name;
            // cout << line << endl;
            iss >> type_name >> trash >> node_num >> net_name;
            NET *net = new NET;
            net->name = net_name;
            int times = stoi(node_num);
            vector<PIN *> PINS;
            double max_x = -DBL_MAX;
            double max_y = -DBL_MAX;
            double min_x = DBL_MAX;
            double min_y = DBL_MAX;

            for (int times_pass = 0; times_pass < times; times_pass++)
            {
                getline(infile, line);
                istringstream iss1(line);
                string name = "";
                string direction = "";
                double of_x;
                double of_y;
                // string token;
                iss1 >> name >> direction >> trash >> of_x >> of_y;
                PIN *pin = new PIN;
                pin->name = name;
                pin->offset.x = of_x;
                pin->offset.y = of_y;
                pin->module = module_map[name];
                // 絕對座標
                double abs_x = pin->module->lb.x + 0.5 * pin->module->W_H.x + pin->offset.x;
                double abs_y = pin->module->lb.y + 0.5 * pin->module->W_H.y + pin->offset.y;
                // 更新 min/max
                max_x = max(max_x, abs_x);
                min_x = min(min_x, abs_x);
                max_y = max(max_y, abs_y);
                min_y = min(min_y, abs_y);
                PINS.push_back(pin);
            }

            net->pins = PINS;
            net_map[net->name] = net;
            double hpwl = 0;
            hpwl = (max_x - min_x) + (max_y - min_y);
            wHPWL += hpwl; // ← 加總
        }
    }
}

void parser_pl(string pl_file)
{
    ifstream infile(pl_file);
    if (!infile.is_open())
    {
        cerr << "Error opening file: " << pl_file << "\n";
        return;
    }

    string line;
    getline(infile, line); // 跳過 header
    max_x = -DBL_MAX;
    max_y = -DBL_MAX;

    while (getline(infile, line))
    {
        // 跳過空白與註解
        if (line.find_first_not_of(" \t\r\n") == string::npos)
            continue;
        if (line[0] == '#')
            continue;

        istringstream iss(line);
        string name, colon, orient, fixed;
        double x, y = 0;
        // 至少要有 name, x, y, :, orient
        if (!(iss >> name >> x >> y >> colon >> orient))
        {
            cerr << "[WARN] Skip malformed line: " << line << endl;
            continue;
        }

        // 可選的 "/FIXED"
        iss >> fixed; // 如果有就讀，沒有就空

        // 檢查 module 是否存在
        if (module_map.find(name) == module_map.end())
        {
            cerr << "[WARN] Module not found for placement: " << name << endl;
            continue;
        }

        // 檢查座標是否是數字
        // if (!all_of(x.begin(), x.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; }) ||
        //     !all_of(y.begin(), y.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; })) {
        //     cerr << "[WARN] Invalid coordinates in line: " << line << endl;
        //     continue;
        // }

        module_map[name]->lb.x = x;
        // cout << x << endl;
        module_map[name]->lb.y = y;
        max_x = max<double>(max_x, module_map[name]->lb.x + module_map[name]->W_H.x);
        max_y = max<double>(max_y, module_map[name]->lb.y + module_map[name]->W_H.y);
        // 設定 orientation
        if (orient == "N")
            module_map[name]->orientation = 0;
        else if (orient == "S")
            module_map[name]->orientation = 1;
        else if (orient == "W")
            module_map[name]->orientation = 2;
        else if (orient == "E")
            module_map[name]->orientation = 3;
        else if (orient == "FN")
            module_map[name]->orientation = 4;
        else if (orient == "FW")
            module_map[name]->orientation = 5;
        else if (orient == "FS")
            module_map[name]->orientation = 6;
        else if (orient == "FE")
            module_map[name]->orientation = 7;
        else
            module_map[name]->orientation = -1; // unknown

        // 可選的 /FIXED 標記
        if (fixed == "/FIXED")
        {
            module_map[name]->fixed = true; // 假設 MODULE struct 有 fixed 欄位
        }
    }
    cout << "max_x: " << max_x << endl;
    cout << "max_y: " << max_y << endl;

    infile.close();
}

RECT parser_scl(string scl_file)
{
    ifstream infile(scl_file);
    if (!infile.is_open())
    {
        cerr << "Error opening file: " << scl_file << "\n";
    }

    string line;
    int site_h = -1, site_w = -1;
    int coordinate = -1, subrowOrigin = -1, numSites = -1;
    int NumRows = -1;

    RECT rect;
    rect.lb.x = DBL_MAX;
    rect.lb.y = DBL_MAX;
    rect.ur.x = DBL_MIN;
    rect.ur.y = DBL_MIN;

    while (getline(infile, line))
    {
        if (line.find_first_not_of(" \t\r\n") == string::npos)
            continue;
        if (line[0] == '#')
            continue;

        if (line.find("NumRows") != string::npos)
        {
            istringstream iss(line);
            string key;
            char colon;
            iss >> key >> colon >> NumRows;
        }

        if (line.find("Height") != string::npos)
        {
            istringstream iss(line);
            string key;
            char colon;
            iss >> key >> colon >> site_h;
        }
        else if (line.find("Sitewidth") != string::npos)
        {
            istringstream iss(line);
            string key;
            char colon;
            iss >> key >> colon >> site_w;
        }
        else if (line.find("Coordinate") != string::npos)
        {
            istringstream iss(line);
            string key;
            char colon;
            iss >> key >> colon >> coordinate;
        }
        else if (line.find("SubrowOrigin") != string::npos)
        {
            istringstream iss(line);
            string key1, key2;
            char colon;
            iss >> key1 >> colon >> subrowOrigin >> key2 >> colon >> numSites;

            // 每遇到一個 subrow 就更新 rect
            double x1 = subrowOrigin;
            double x2 = subrowOrigin + numSites * site_w;
            double y1 = coordinate;
            double y2 = coordinate + site_h; // 注意 row 是水平帶狀，height 向下佔一格

            rect.lb.x = min<double>(rect.lb.x, x1);
            rect.lb.y = min<double>(rect.lb.y, y1);
            rect.ur.x = max<double>(rect.ur.x, x2);
            rect.ur.y = max<double>(rect.ur.y, y2);
            ROW *row = new ROW;
            row->lb.x = subrowOrigin;
            row->lb.y = coordinate;
            row->W_H.x = numSites * site_w;
            row->W_H.y = site_h;
            row->NumSites = numSites;
            row->Sitespacing = site_w;
            // for(const auto& kv : module_map){
            //     if(!kv.second->fixed){
            //         if(kv.second->lb.y == coordinate){
            //             row->modules.push_back(kv.second);
            //         }
            //     }
            // }
            ROWS.push_back(row);
        }
    }
    infile.close();
    chip_lb.x = rect.lb.x;
    chip_lb.y = rect.lb.y;
    chip_ur.x = rect.ur.x;
    chip_ur.y = rect.ur.y;
    cout << "左下角: (" << rect.lb.x << "," << rect.lb.y << ")\n";
    cout << "右上角: (" << rect.ur.x << "," << rect.ur.y << ")\n";
    cout << "numrows: " << NumRows << "\n";
    cout << "row vector size: " << ROWS.size() << "\n";
    return rect;
}

void writ_legal_pl(string out_file)
{
    ofstream fout(out_file);
    if (!fout.is_open())
    {
        cerr << "Error: cannot write " << out_file << "\n";
        return;
    }

    for (const auto &kv : module_map)
    {
        MODULE *m = kv.second;
        fout << m->name << " " << m->lb.x << " " << m->lb.y << " : ";
        // orientation
        switch (m->orientation)
        {
        case 0:
            fout << "N";
            break;
        case 1:
            fout << "S";
            break;
        case 2:
            fout << "W";
            break;
        case 3:
            fout << "E";
            break;
        case 4:
            fout << "FN";
            break;
        case 5:
            fout << "FW";
            break;
        case 6:
            fout << "FS";
            break;
        case 7:
            fout << "FE";
            break;
        default:
            fout << "N";
            break; // unknown, default to N
        }
        if (m->fixed)
        {
            fout << " /FIXED";
        }
        fout << "\n";
    }

    fout.close();
    cout << "[INFO] Wrote legal placement to " << out_file << endl;
}
// parser.cpp

void write_matlab_file(string out_file)
{
    ofstream fout(out_file);
    if (!fout.is_open())
    {
        cerr << "Error: cannot open " << out_file << "\n";
        return;
    }

    fout << "% MATLAB Script for Placement Visualization\n";
    fout << "figure('Name', '" << out_file << "', 'NumberTitle', 'off');\n";
    fout << "clf; hold on; axis equal;\n";
    fout << "title('Placement Result');\n";
    fout << "xlabel('X (microns)'); ylabel('Y (microns)');\n";

    // 1. 繪製晶片邊界 (Chip Boundary)
    fout << "% Chip Boundary\n";
    fout << "rectangle('Position', ["
         << chip_lb.x << ", " << chip_lb.y << ", "
         << (chip_ur.x - chip_lb.x) << ", " << (chip_ur.y - chip_lb.y)
         << "], 'EdgeColor', 'k', 'LineWidth', 2, 'LineStyle', '--');\n";

    // 2. 分類固定元件與可移動元件
    vector<MODULE *> fixed_modules;
    vector<MODULE *> movable_modules;
    for (const auto &kv : module_map)
    {
        if (kv.second->fixed)
            fixed_modules.push_back(kv.second);
        else
            movable_modules.push_back(kv.second);
    }

    // Helper Lambda: 輸出 MATLAB 陣列與繪圖指令
    auto write_matlab_arrays = [&](string prefix, const vector<MODULE *> &modules, string color, bool fill)
    {
        if (modules.empty())
            return;

        fout << "% " << prefix << " Modules\n";
        fout << prefix << "_x = [";
        for (auto *m : modules)
            fout << m->lb.x << " ";
        fout << "];\n";

        fout << prefix << "_y = [";
        for (auto *m : modules)
            fout << m->lb.y << " ";
        fout << "];\n";

        fout << prefix << "_w = [";
        for (auto *m : modules)
            fout << m->W_H.x << " ";
        fout << "];\n";

        fout << prefix << "_h = [";
        for (auto *m : modules)
            fout << m->W_H.y << " ";
        fout << "];\n";

        // MATLAB Loop to draw rectangles
        fout << "for i = 1:length(" << prefix << "_x)\n";
        fout << "    rectangle('Position', ["
             << prefix << "_x(i), " << prefix << "_y(i), "
             << prefix << "_w(i), " << prefix << "_h(i)], "
             << "'EdgeColor', '" << color << "'";

        if (fill) // 固定元件填色
            fout << ", 'FaceColor', '" << color << "', 'FaceAlpha', 0.5";

        fout << ");\n";
        fout << "end\n";
    };

    // 3. 寫入資料
    // 固定元件：紅色 (Red)，填色
    write_matlab_arrays("fixed", fixed_modules, "r", true);

    // 可移動元件：藍色 (Blue)，只有邊框
    write_matlab_arrays("movable", movable_modules, "b", false);

    fout << "hold off;\n";
    fout.close();
    cout << "[INFO] Wrote MATLAB visualization to " << out_file << endl;
}

// void write_cell_dat(string out_file) {
//     ofstream fout(out_file);
//     if (!fout.is_open()) {
//         cerr << "Error: cannot write " << out_file << "\n";
//         return;
//     }

//     // FENCE 預設，通常是設計邊界 (這裡先隨便放個 (0,0))
//     fout << "# FENCE DEFAULT\n";
//     fout << "0, 0\n\n";

//     for (const auto& kv : module_map) {
//         if(!kv.second->fixed){
//             MODULE* m = kv.second;

//             double x0 = m->lb.x;
//             double y0 = m->lb.y;
//             double x1 = x0 + m->W_H.x;
//             double y1 = y0 + m->W_H.y;

//             fout << "# " << m->name << "\n";
//             fout << x0 << ", " << y0 << "\n";
//             fout << x1 << ", " << y0 << "\n";
//             fout << x1 << ", " << y1 << "\n";
//             fout << x0 << ", " << y1 << "\n";
//             fout << x0 << ", " << y0 << "\n\n";  // 閉合矩形
//         }
//     }

//     fout.close();
//     cout << "[INFO] Wrote cell  to " << out_file << endl;
// }

// void write_pad_dat(string out_file) {
//     ofstream fout(out_file);
//     if (!fout.is_open()) {
//         cerr << "Error: cannot write " << out_file << "\n";
//         return;
//     }

//     // FENCE 預設，通常是設計邊界 (這裡先隨便放個 (0,0))
//     fout << "# FENCE DEFAULT\n";
//     fout << "0, 0\n\n";

//     for (const auto& kv : module_map) {
//         if(kv.second->fixed){
//             MODULE* m = kv.second;

//             double x0 = m->lb.x;
//             double y0 = m->lb.y;
//             double x1 = x0 + m->W_H.x;
//             double y1 = y0 + m->W_H.y;
//             double x2 = x0 + 0.75*m->W_H.x;
//             double y2 = y0 +0.4*m->W_H.y;
//             double x3 = x0 + 0.65*m->W_H.x;
//             double y3 = y0 +0.8*m->W_H.y;
//             double x4 = x0 + 0.85*m->W_H.x;

//             fout << "# " << m->name << "\n";
//             fout << x0 << ", " << y0 << "\n";
//             fout << x1 << ", " << y0 << "\n";
//             fout << x1 << ", " << y1 << "\n";
//             fout << x0 << ", " << y1 << "\n";
//             fout << x0 << ", " << y0 << "\n\n";  // 閉合矩形

//             fout << "#arrow" << "\n";
//             fout << x2 << ", " << y1 << "\n";
//             fout << x2 << ", " << y2 << "\n";
//             fout << x2 << ", " << y1 << "\n";
//             fout << x3 << ", " << y3 << "\n";
//             fout << x2 << ", " << y1 << "\n";
//             fout << x4 << ", " << y3 << "\n\n";
//         }
//     }

//     fout.close();
//     cout << "[INFO] Wrote cell  to " << out_file << endl;
// }

// void write_pad_pin_dat(string out_file) {
//     ofstream fout(out_file);
//     if (!fout.is_open()) {
//         cerr << "Error: cannot write " << out_file << "\n";
//         return;
//     }

//     // FENCE 預設，通常是設計邊界 (這裡先隨便放個 (0,0))
//     fout << "# FENCE DEFAULT\n";
//     fout << "0, 0\n\n";

//     for (const auto& kv : net_map) {
//         for (const auto& pin : kv.second->pins) {
//             if (!pin->module) {
//                 cerr << "[DEBUG] pin " << pin->name << " has null module\n";
//                 continue;
//             }
//             if(pin->module->fixed){
//                 MODULE* m = pin->module;

//                 double x0 = m->lb.x + m->W_H.x/2 + pin->offset.x;
//                 double y0 = m->lb.y + m->W_H.y/2 + pin->offset.y;
//                 double x1 = x0 + 3;
//                 double y1 = y0 + 3;

//                 fout << "# " << m->name << "\n";
//                 fout << x0 << ", " << y0 << "\n";
//                 fout << x1 << ", " << y0 << "\n";
//                 fout << x1 << ", " << y1 << "\n";
//                 fout << x0 << ", " << y1 << "\n";
//                 fout << x0 << ", " << y0 << "\n\n";  // 閉合矩形
//             }
//         }
//     }

//     fout.close();
//     cout << "[INFO] Wrote cell  to " << out_file << endl;
// }

// void write_chip_dat(string out_file, RECT rect) {
//     ofstream fout(out_file);
//     if (!fout.is_open()) {
//         cerr << "Error: cannot write " << out_file << "\n";
//         return;
//     }
//     fout << "# FENCE DEFAULT\n";
//     fout << "0, 0\n\n";
//     fout << "# " << "outline" << "\n";
//     fout << 0 << ", " << 0 << "\n";
//     fout << max_x << ", " << 0 << "\n";
//     fout << max_x << ", " << max_y << "\n";
//     fout << 0 << ", " << max_y << "\n";
//     fout << 0 << ", " <<  0 << "\n\n";  // 閉合矩形

//     // FENCE 預設，通常是設計邊界 (這裡先隨便放個 (0,0))
//     fout << "# FENCE DEFAULT\n";
//     fout << "0, 0\n\n";
//     fout << "# " << "chip" << "\n";
//     fout << rect.lb.x << ", " << rect.lb.y << "\n";
//     fout << rect.ur.x << ", " << rect.lb.y << "\n";
//     fout << rect.ur.x << ", " << rect.ur.y << "\n";
//     fout << rect.lb.x << ", " << rect.ur.y << "\n";
//     fout << rect.lb.x<< ", " <<  rect.lb.y << "\n\n";  // 閉合矩形
//     fout.close();
//     cout << "[INFO] Wrote chip  to " << out_file << endl;
// }

// void write_plt(string out_file) {
//     ofstream fout(out_file);

//     // ====== 擷取 bmk ======
//     // 先找到最後一個 '/'，那是檔名的開始
//     size_t lastSlash = out_file.find_last_of('/');
//     string filename = out_file.substr(lastSlash + 1); // e.g. "adaptec1.plt"

//     // 去掉副檔名 ".plt"
//     size_t dotPos = filename.find_last_of('.');
//     string bmk = filename.substr(0, dotPos); // e.g. "adaptec1"

//     // ====== 正常輸出 ======
//     fout << "set nokey\n";
//     fout << "set term pngcairo font 'qt'\n";
//     fout << "set output 'plt/" << bmk << "/" << bmk << ".png'\n";

//     fout << "set size ratio -1\n";
//     fout << "set xrange [0:" << max_x << "]\n";
//     fout << "set yrange [0:" << max_y << "]\n";

//     fout << "plot[:][:] 'dat/" << bmk << "/" << bmk << "_cell.dat' w l lc 6, "
//          << "'dat/" << bmk << "/" << bmk << "_pad.dat' w l lc 4, "
//          << "'dat/" << bmk << "/" << bmk << "_pad_pin.dat' w l lc -1, "
//          << "'dat/" << bmk << "/" << bmk << "_chip.dat' w l lc -1\n";

//     fout << "set title '" << bmk
//          //<< "  MODULE=" << module_map.size() << ", "
//          << "  NET=" << net_map.size() << ", "
//          << "  HPWL=" << wHPWL
//          << "' font 'Times, 22'\n";

//     fout << "set term qt\n";
//     fout << "set key\n";

//     fout << "set size ratio -1\n";
//     fout << "plot[:][:] 'dat/" << bmk << "/" << bmk << "_cell.dat' w l lc 6, "
//          << "'dat/" << bmk << "/" << bmk << "_pad.dat' w l lc 4, "
//          << "'dat/" << bmk << "/" << bmk << "_pad_pin.dat' w l lc -1, "
//          << "'dat/" << bmk << "/" << bmk << "_chip.dat' w l lc -1\n";

//     fout << "pause -1 'Press any key'\n";
// }
