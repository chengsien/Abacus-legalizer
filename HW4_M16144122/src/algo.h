#ifndef ALGO_H
#define ALGO_H

#include "datatype.h"
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

struct cluster
{
    // vector<MODULE *> modules;
    int start_index; // 在 row->modules 中的起始 index
    int end_index;   // 在 row->modules 中的結束 index

    double wc; // total width of cluster
    double qc; // total empty space in cluster
    double ec; // total extra space in cluster
    double xc; // x coordinate of cluster
    int n_first, n_last;
};
class abacus
{
public:
    abacus();
    ~abacus();

    // void sort();
    void algo();
    void PlaceRow(ROW *row);
    // void AddCell(cluster &C int idx);
    void Collapse(cluster &C, cluster &C_before, vector<cluster> &clusters, double row_min, double row_max);
    void slice_row();

private:
    // int n; // number of nodes
};
#endif