#ifndef Cure_H
#define Cure_H

#include<Definition.h>


class Cure{
public:
    Cure(unsigned int max_nreps, unsigned int k);
    ~Cure();
    K_CLUSTER make_k_cluster();
    HEAP make_minheap();
    void compute_all_clusters_distance(K_CLUSTER k_cluster);
    CLUSTER merge_clusters(CLUSTER u, CLUSTER v);
    K_CLUSTER cure();
    HEAP update_clusters_distance(CLUSTER w, HEAP &minheap, K_CLUSTER &k_cluster, int u_id, int v_id, int w_id);
    double distance_from_cluster_pair(CLUSTER u, CLUSTER v);

    map<string, int> product_name_to_id;
private:
    double clusters_dist[MAX_CLUSTER_NUM][MAX_CLUSTER_NUM];
    double users_dist[MAX_USER_NUM][MAX_USER_NUM];
    unsigned int max_nreps_;
    unsigned int k_;
};
#endif // Cure_H
