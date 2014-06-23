#ifndef Misc_H
#define Misc_H

#include<Definition.h>

class Misc{
public:
    void normalize_user_score();
    void undo_normalize_user_score();
    double* get_all_product_average_score(int nproduct);
    void show_result(K_CLUSTER k_cluster);

    map<string, pair<int, int> > tmp_max_min;
};

#endif // Misc_H
