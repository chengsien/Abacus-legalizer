#include "algo.h"
using namespace std;
abacus::abacus()
{
    // n = module_map.size();
}

abacus::~abacus()
{
}

double AlignToSite(double x, ROW *row)
{
    if (row->Sitespacing <= 0)
        return x; // 防呆
    if (ROWS.empty())
        return x;

    // [FIX 4] Use ROWS[0]->lb.x as global origin to ensure alignment with original rows
    double origin = ROWS[0]->lb.x;
    double sites = round((x - origin) / row->Sitespacing);
    double aligned_x = origin + sites * row->Sitespacing;

    return aligned_x;
}

void abacus::algo()
{
    vector<MODULE *> module_vector;
    module_vector.reserve(module_map.size());
    for (const auto &i : module_map)
    {
        if (i.second->fixed)
            continue;
        module_vector.push_back(i.second);
    }

    // 根據 x 座標排序
    sort(module_vector.begin(), module_vector.end(), [](MODULE *a, MODULE *b)
         { return a->lb.x < b->lb.x; });


    int search_range = 6;
    if(module_map.size() > 800000)
        search_range = 3;
    if(module_map.size() < 600000)
        search_range = 37;

    // 工作緩衝重複使用以降低分配成本
    vector<ROW *> total_subrow;
    vector<pair<MODULE *, double>> original_positions;

    for (const auto &m : module_vector)
    {
        double Cbest = DBL_MAX;
        double cost = 0;
        int best_row_index = -1; // 用來存最佳的列

        // 1. 計算範圍
        int raw_index = (m->lb.y - ROWS[0]->lb.y) / ROWS[0]->W_H.y;

        // Clamp
        int clamped_index = raw_index;
        if (clamped_index < 0)
            clamped_index = 0;
        if (clamped_index >= (int)ROWS.size())
            clamped_index = ROWS.size() - 1;

        // 搜尋範圍
        int start_search = max(0, clamped_index - search_range);
        int end_search = min((int)ROWS.size() - 1, clamped_index + search_range);

        // 收集 subrows
        total_subrow.clear();
        total_subrow.reserve((end_search - start_search + 1) * 10);
        for (int i = start_search; i <= end_search; i++)
        {
            const auto &sub_rows = ROWS[i]->sub_rows;
            total_subrow.insert(total_subrow.end(), sub_rows.begin(), sub_rows.end());
        }

        // 開始試算
        const size_t total_subrow_size = total_subrow.size();
        for (size_t i = 0; i < total_subrow_size; i++)
        {
            ROW *target_row = total_subrow[i];

            // 備份
            original_positions.clear();
            original_positions.reserve(target_row->modules.size());
            for (auto *mod : target_row->modules)
            {
                original_positions.push_back(make_pair(mod, mod->lb.x));
            }

            // 記錄 m 的原始位置
            double m_original_x = m->lb.x;
            double m_original_y = m->lb.y;

            // 放入試算
            target_row->modules.push_back(m);
            m->lb.y = target_row->lb.y;

            PlaceRow(target_row); // 排列並防重疊

            // 檢查是否超出 subrow 邊界
            bool is_overflow = false;
            const double limit = target_row->lb.x + target_row->W_H.x;
            if (!target_row->modules.empty())
            {
                const MODULE *last = target_row->modules.back();
                if (last->lb.x + last->W_H.x > limit + 0.001)
                {
                    is_overflow = true;
                }
            }

            if (!is_overflow)
            {
                // 只有沒爆掉才計算正常 Cost
                cost = 0;
                // 計算舊 Cells 位移
                for (const auto &record : original_positions)
                {
                    const MODULE *mod = record.first;
                    const double old_x = record.second;
                    cost += abs(mod->lb.x - old_x);
                }
                // 計算 m 位移
                const double m_dx = abs(m->lb.x - m_original_x);
                const double m_dy = abs(m->lb.y - m_original_y);
                cost += (m_dx + m_dy);

                if (cost < Cbest)
                {
                    Cbest = cost;
                    best_row_index = i;
                }
            }
            // 恢復狀態
            // 1. 移除 m
            auto it = find(target_row->modules.begin(), target_row->modules.end(), m);
            if (it != target_row->modules.end())
            {
                target_row->modules.erase(it);
            }
            // 2. 還原舊座標
            for (auto &record : original_positions)
            {
                record.first->lb.x = record.second;
            }
            // 3. 還原 m
            m->lb.x = m_original_x;
            m->lb.y = m_original_y;
        }

        // 5. 將 m 放入最終決定的最佳列
        if (best_row_index != -1)
        {
            ROW *best_subrow = total_subrow[best_row_index];
            best_subrow->modules.push_back(m);
            m->lb.y = best_subrow->lb.y;
            PlaceRow(best_subrow);
        }
        else
        {
            cout << "FATAL: Cell " << m->name << " cannot be placed anywhere!" << endl;
        }
    }
}
void abacus::Collapse(cluster &C, cluster &C_before, vector<cluster> &clusters, double row_min, double row_max)
{
    C.xc = C.qc / C.ec;
    if (C.xc < row_min)
        C.xc = row_min;
    if (C.xc + C.wc > row_max)
        C.xc = row_max - C.wc;
    if (C_before.xc + C_before.wc > C.xc)
    {
        // merge C_before and C
        C_before.end_index = C.end_index; // 關鍵修改
        // C_before.modules.insert(C_before.modules.end(), C.modules.begin(), C.modules.end());
        //  AddCluster(c-1,i)
        C_before.n_last = C.n_last;
        C_before.qc = C_before.qc + C.qc - C_before.wc * C.ec;
        C_before.ec = C_before.ec + C.ec;
        C_before.wc = C_before.wc + C.wc;
        // Collapse(c-1)
        clusters.pop_back(); // Remove last cluster

        if (clusters.size() > 1)
        {
            Collapse(clusters.back(), clusters[clusters.size() - 2], clusters, row_min, row_max);
        }
    }
}

void abacus::PlaceRow(ROW *row)
{
    auto cmp_module_x = [](MODULE *a, MODULE *b)
    { return a->lb.x < b->lb.x; };
    if (!is_sorted(row->modules.begin(), row->modules.end(), cmp_module_x))
    {
        sort(row->modules.begin(), row->modules.end(), cmp_module_x);
    }

    // int c = 0;
    // cluster C;
    vector<cluster> clusters;
    clusters.reserve(row->modules.size());
    double row_min = row->lb.x;
    double row_max = row->lb.x + row->W_H.x;

    // Align row_min UP to site grid
    if (row->Sitespacing > 0 && !ROWS.empty())
    {
        double spacing = row->Sitespacing;
        double origin = ROWS[0]->lb.x;
        double num = ceil((row_min - origin) / spacing);
        row_min = origin + num * spacing;
    }

    const size_t modules_size = row->modules.size();
    for (size_t i = 0; i < modules_size; i++)
    {
        // C = clusters[sizeof(clusters) - 1];
        // NO OVERLAP
        MODULE *m = row->modules[i];
        if (clusters.empty() || clusters.back().xc + clusters.back().wc <= m->lb.x)
        {
            cluster new_cluster;
            new_cluster.start_index = i;
            new_cluster.end_index = i;
            // new_cluster.modules.push_back(row->modules[i]);

            new_cluster.xc = m->lb.x;
            new_cluster.n_first = i;

            // AddCell(c,i)
            new_cluster.n_last = i;
            new_cluster.qc = m->lb.x;
            new_cluster.wc = m->W_H.x;
            new_cluster.ec = 1;
            // store new_cluster
            clusters.push_back(new_cluster);
        }
        else
        {
            cluster &C = clusters.back();
            // AddCell(c,i)
            C.end_index = i;
            C.n_last = i;
            C.ec = C.ec + 1;
            C.qc = C.qc + m->lb.x - C.wc;
            C.wc = C.wc + m->W_H.x;
            // cout << "cluste size before collapse: " << clusters.size() << endl;
            if (clusters.size() > 1)
            {
                Collapse(clusters.back(), clusters[clusters.size() - 2], clusters, row_min, row_max);
            }
            // Collapse(C, clusters[clusters.size() - 2], clusters);
        }
    }
    // 記錄目前這條 Row 最左邊能放的位置 (初始為 Row 起點) 寫回座標時的「防重疊機制」
    double last_placed_x = row_min;
    for (const auto &cl : clusters)
    {
        double x = cl.xc;
        // 初步边界检查
        if (x < row_min)
            x = row_min;
        if (x + cl.wc > row_max)
            x = row_max - cl.wc;

        // 对齐到 Site Grid
        double aligned_start = AlignToSite(x, row);

        // 防重叠：确保不与前一个 cluster 重叠
        if (aligned_start < last_placed_x)
        {
            aligned_start = AlignToSite(last_placed_x, row);
        }

        // 防超界：确保不超出 row 右边界
        if (aligned_start + cl.wc > row_max)
        {
            aligned_start = row_max - cl.wc;
            // 确保对齐
            aligned_start = AlignToSite(aligned_start, row);
            // 如果对齐后仍然重叠，优先保证不重叠
            if (aligned_start < last_placed_x)
            {
                aligned_start = last_placed_x;
            }
        }
        // [步驟 C] 寫回 Cluster 內的所有 Cells
        // 讓 Cluster 內的 Cells 緊密排列 (Abutment)
        double current_x = aligned_start;
        // 利用索引直接遍歷 row->modules，不用複製 vector
        for (int i = cl.start_index; i <= cl.end_index; i++)
        {
            row->modules[i]->lb.x = current_x;
            current_x += row->modules[i]->W_H.x;
        }
        last_placed_x = current_x;
    }
}

void abacus::slice_row()
{
    // 1. 收集所有的 Fixed Modules (障礙物)
    vector<MODULE *> obstacles;
    obstacles.reserve(module_map.size() / 4);
    for (const auto &item : module_map)
    {
        if (item.second->fixed)
        {
            obstacles.push_back(item.second);
        }
    }

    // 2. 對每一條 Row 進行獨立處理
    for (auto *row : ROWS)
    {
        // 先清除舊的 Sub-Rows (以防重複呼叫)
        for (auto *sub : row->sub_rows)
            delete sub;
        row->sub_rows.clear();

        // 用來暫存這條 Row 上的所有「被阻擋區間」
        vector<pair<double, double>> blocked_intervals;
        blocked_intervals.reserve(obstacles.size());

        const double row_y_min = row->lb.y;
        const double row_y_max = row->lb.y + row->W_H.y;
        // 使用極小值防止邊界誤判 (Off-by-one fix)
        const double epsilon = 1e-4;

        for (auto *obs : obstacles)
        {
            // [FIX] 對於非矩形模組，檢查所有的 rects
            if (!obs->rects.empty())
            {
                // 非矩形模組：逐個處理每個 rect
                for (const auto &rect : obs->rects)
                {
                    double rect_y_min = rect.lb.y;
                    double rect_y_max = rect.ur.y;

                    // 檢查此 rect 是否與 Row 在 Y 軸重疊
                    if (rect_y_min < row_y_max - epsilon && rect_y_max > row_y_min + epsilon)
                    {
                        double start_x = max(chip_lb.x, rect.lb.x);
                        double end_x = min(chip_ur.x, rect.ur.x);

                        if (start_x < end_x)
                        {
                            blocked_intervals.push_back({start_x, end_x});
                        }
                    }
                }
            }
            else
            {
                // 矩形模組：使用主矩形
                if (obs->lb.y < row_y_max - epsilon && (obs->lb.y + obs->W_H.y) > row_y_min + epsilon)
                {
                    double start_x = max(chip_lb.x, obs->lb.x);
                    double end_x = min(chip_ur.x, obs->lb.x + obs->W_H.x);

                    if (start_x < end_x)
                    {
                        blocked_intervals.push_back({start_x, end_x});
                    }
                }
            }
        }

        // 3. 區間合併 (Interval Merging) - 核心邏輯
        // 先依照起點排序
        sort(blocked_intervals.begin(), blocked_intervals.end());

        vector<pair<double, double>> merged;
        merged.reserve(blocked_intervals.size());
        for (const auto &interval : blocked_intervals)
        {
            if (merged.empty() || interval.first > merged.back().second)
            {
                // 如果 merged 為空，或是當前區間跟上一個區間沒重疊 -> 直接加入
                merged.push_back(interval);
            }
            else
            {
                // 如果重疊，則合併 (取最大的結束點)
                merged.back().second = max(merged.back().second, interval.second);
            }
        }

        // 4. 切割出 Sub-Rows (利用合併後的阻擋區間)
        // 修正後：使用該 Row 自己的邊界
        const double row_lb_x = row->lb.x;
        const double row_end_x = row->lb.x + row->W_H.x;
        double current_x = row_lb_x;

        for (const auto &block : merged)
        {
            // 如果當前位置 < 阻擋區間的起點，代表中間有空位，可以切出一個 Sub-Row
            if (block.first > current_x + epsilon)
            {
                ROW *sub = new ROW();
                const double row_lb_y = row->lb.y;
                const double row_W_H_y = row->W_H.y;
                const double row_Sitespacing = row->Sitespacing;
                sub->lb.y = row_lb_y;
                sub->W_H.y = row_W_H_y;

                // 設定 Sub-Row 的 X 範圍
                sub->lb.x = current_x;
                sub->W_H.x = block.first - current_x;

                // 複製並設定 Site 資訊
                sub->Sitespacing = row_Sitespacing;
                if (sub->Sitespacing > 1e-6)
                {
                    // 對齊 Site Grid
                    // 確保 Sub-Row 的起點落在合法的 Site 上
                    double rel_x = sub->lb.x - chip_lb.x;
                    double site_offset = ceil(rel_x / sub->Sitespacing) * sub->Sitespacing - rel_x;

                    // 如果起點需要調整，縮減寬度
                    sub->lb.x += site_offset;
                    sub->W_H.x -= site_offset;

                    sub->NumSites = floor(sub->W_H.x / sub->Sitespacing);
                }
                else
                {
                    sub->NumSites = 0; // 避免除以零
                }

                // 只有當剩餘寬度還夠放東西時才加入
                if (sub->W_H.x > 1e-6)
                {
                    row->sub_rows.push_back(sub);
                }
                else
                {
                    delete sub; // 太窄了，不要用
                }
            }
            // 移動 current_x 到阻擋區間的結束點 (跳過障礙物)
            current_x = max(current_x, block.second);
        }

        // 5. 處理最後一段 (最後一個障礙物到 Chip 右邊界)
        if (current_x < row_end_x - epsilon)
        {
            ROW *sub = new ROW();
            const double row_lb_y = row->lb.y;
            const double row_W_H_y = row->W_H.y;
            const double row_Sitespacing = row->Sitespacing;
            sub->lb.y = row_lb_y;
            sub->W_H.y = row_W_H_y;

            sub->lb.x = current_x;
            sub->W_H.x = row_end_x - current_x;

            sub->Sitespacing = row_Sitespacing;
            if (sub->Sitespacing > 1e-6)
            {
                double rel_x = sub->lb.x - chip_lb.x;
                double site_offset = ceil(rel_x / sub->Sitespacing) * sub->Sitespacing - rel_x;

                sub->lb.x += site_offset;
                sub->W_H.x -= site_offset;

                sub->NumSites = floor(sub->W_H.x / sub->Sitespacing);
            }
            else
            {
                sub->NumSites = 0;
            }

            if (sub->W_H.x > 1e-6)
            {
                row->sub_rows.push_back(sub);
            }
            else
            {
                delete sub;
            }
        }
    }
}
