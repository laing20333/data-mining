#include "Cure.h"
#include<iostream>
#include<cmath>
#include<algorithm>
#include<memory.h>

using namespace std;

bool operator< (const struct CLUSTER_PAIR &a, const struct CLUSTER_PAIR &b){
	return (a.distance < b.distance);
}

Cure::Cure(unsigned int max_nreps, unsigned int k){
    max_nreps_ = max_nreps;
    k_ = k;
}

Cure::~Cure(){

}

K_CLUSTER Cure::make_k_cluster(){
    extern map<string, vector<double> > score;

    // put every user into a cluster
    K_CLUSTER k_cluster;
    int ncluster = 0;

    map<string, vector<double> > :: iterator it;
    for(it = score.begin(); it != score.end(); it++){
        USER user;
        CLUSTER tmp_cluster;
        user.name = it->first; user.mean = -1;
        tmp_cluster.users.push_back(user);
        double nproduct = 0;
        // calculate cluster's mean
        for(unsigned int product_id=0; product_id < it->second.size(); product_id++){
            double product_score = it->second[product_id];
            if(product_score + 1e-7 >= 0){
                tmp_cluster.mean += product_score;
                nproduct++;
            }
        }
        tmp_cluster.mean /= nproduct;
        tmp_cluster.reps.push_back(it->first);
        k_cluster[ncluster++] = tmp_cluster;
    }


    cout << "-> make_k_cluster ...ok!" << endl;
    return k_cluster;
}

HEAP Cure::make_minheap(){
    extern map<string, vector<double> > score;
    HEAP minheap;
    unsigned int cluster_num = score.size();

    // caculate shortest cluster pair for every cluster
    for(unsigned int i=0; i < cluster_num; i++){
        CLUSTER_PAIR cluster_pair;
        cluster_pair.src_id = i; cluster_pair.dst_id = (i + 1) % cluster_num; cluster_pair.distance = 1e9;
        for(unsigned int j=0; j < cluster_num; j++){
            if(i == j )continue;
            if(clusters_dist[i][j] < cluster_pair.distance ){
                cluster_pair.dst_id = j;
                cluster_pair.distance = clusters_dist[i][j];
            }
        }
        minheap.push_back(cluster_pair);
    }
    make_heap(minheap.begin(), minheap.end());
    cout << "-> make_minheap ...ok!" << endl;
    return minheap;

}

void Cure::compute_all_clusters_distance(K_CLUSTER k_cluster){
    unsigned int nclusters = k_cluster.size();
    memset(users_dist, -1, sizeof(users_dist));
    for(unsigned int cluster_i=0; cluster_i < nclusters; cluster_i++){
        for(unsigned int cluster_j=cluster_i+1; cluster_j < nclusters; cluster_j++){
            clusters_dist[cluster_i][cluster_j] = clusters_dist[cluster_j][cluster_i] = distance_from_cluster_pair(k_cluster[cluster_i], k_cluster[cluster_j]);
        }
    }
    cout <<"-> compute_all_clusters_distance ...ok!" << endl;
}

CLUSTER Cure::merge_clusters(CLUSTER u, CLUSTER v){
    extern map<string, vector<double> > score;
    CLUSTER w = u;
    w.reps.clear();
    USER user;
    // merge two clusters' users
    for(unsigned int i=0; i<v.users.size(); i++){
        user.name = v.users[i].name;
        user.mean = v.users[i].mean;
        w.users.push_back(user);
    }
    // calculate mean
    w.mean = (u.users.size() * u.mean + v.users.size() * v.mean) / (u.users.size() + v.users.size());

    // choose reps
    vector<string> tmpset;
    unsigned int nreps = min(w.users.size(), max_nreps_);
    for(unsigned int i=0; i < nreps ; i++){
        double tmp_distance;
        double max_distance = 0;
        string max_user_id;
        for(unsigned int j=0; j<w.users.size(); j++){
            if(i == 0){
                // caculate distance between user j and w.mean by
                // average difference between the product score judged by j and mean
                string username_j = w.users[j].name;
                unsigned int nproduct = score[username_j].size();
                double nscore = 0;
                for(unsigned int product_id = 0; product_id < nproduct; product_id++){
                    if(score[username_j][product_id] + 1e-7 >= 0){
                        tmp_distance += score[username_j][product_id];
                        nscore++;
                    }
                }
                tmp_distance /= nscore;
            }else {
                // caculate the point which is furthest from tmpset
                CLUSTER tmp_cluster;
                for(unsigned int j=0; j<tmpset.size(); j++){
                    tmp_cluster.reps.push_back(tmpset[j]);
                }
                for(unsigned int j=0; j<tmpset.size(); j++){
                    CLUSTER point_cluster;
                    point_cluster.reps.push_back(tmpset[j]);
                    tmp_distance = distance_from_cluster_pair(tmp_cluster, point_cluster);
                }
            }
            if(tmp_distance > max_distance){
                max_distance = tmp_distance;
                max_user_id = w.users[j].name;
            }
        }
        tmpset.push_back(max_user_id);
    }

    // shrink representative points
    for(unsigned int i=0; i<tmpset.size(); i++){
        string username_i = tmpset[i];
        unsigned int nproduct = score[username_i].size();
        for(unsigned int product_id=0; product_id < nproduct; product_id++){
            if(score[username_i][product_id] + 1e-7 > 0){
                score[username_i][product_id] = score[username_i][product_id] + ALPHA * (w.mean - score[username_i][product_id]);
            }
        }
        w.reps.push_back(username_i);
    }
    //cout << "merge two clusters ok! " << endl;
    return w;
}

K_CLUSTER Cure::cure(){
    // init data structure
    K_CLUSTER k_cluster = make_k_cluster();
    compute_all_clusters_distance(k_cluster);
    HEAP minheap = make_minheap();
    cout << "-> Start to run CURE Algorithm:" << endl;

    int new_cluster_id = k_cluster.size();
    while(minheap.size() > k_){
        cout << "-> Number of clusters = " << k_cluster.size() << endl;
        // extract closest cluster_pair
        int u_id, v_id;
        struct CLUSTER_PAIR u_pair = minheap.front();
        u_id = u_pair.src_id; v_id = u_pair.dst_id;
        pop_heap(minheap.begin(), minheap.end());
        minheap.pop_back(); // delete u cluster

        // merge cluster u and cluster v
        CLUSTER w = merge_clusters(k_cluster[u_id], k_cluster[v_id]);

        // assign new_id to w, remove u, v cluster from k_cluster and insert w into k_cluster
        unsigned int w_id = new_cluster_id++;
        k_cluster.erase(u_id); k_cluster.erase(v_id); k_cluster[w_id] = w;

        // update all cluster pair distance
        minheap = update_clusters_distance(w, minheap, k_cluster, u_id, v_id, w_id);

    }

    //show_result(k_cluster);
    cout << "-> CURE ...ok!" << endl;

    return k_cluster;
}

HEAP Cure::update_clusters_distance(CLUSTER w, HEAP &minheap, K_CLUSTER &k_cluster, int u_id, int v_id, int w_id){
    CLUSTER_PAIR w_pair;
    w_pair.src_id = w_id; w_pair.dst_id = minheap[0].src_id;  w_pair.distance = 1e9;

    // update clusters' distance
    for(unsigned int i=0; i<minheap.size(); i++){
        CLUSTER_PAIR tmp_cluster_pair = minheap[i];
        if(tmp_cluster_pair.src_id == v_id){
            // delete v_cluster
            minheap.erase(minheap.begin() + i);
            i--;
        }else {
            // update distance from every cluster
            CLUSTER tmp_cluster = k_cluster[tmp_cluster_pair.src_id];
            clusters_dist[w_id][tmp_cluster_pair.src_id] = clusters_dist[tmp_cluster_pair.src_id][w_id] = distance_from_cluster_pair(w, tmp_cluster);

            // update shortest w.closest cluster
            if(clusters_dist[w_id][tmp_cluster_pair.src_id] < w_pair.distance){
                w_pair.dst_id = tmp_cluster_pair.src_id;
                w_pair.distance = clusters_dist[w_id][tmp_cluster_pair.src_id];
            }

            // update every closest pair cluster in minheap
            if(tmp_cluster_pair.dst_id == u_id ||  tmp_cluster_pair.dst_id == v_id ){
                // 1. x.closest is u or v
                if(clusters_dist[tmp_cluster_pair.src_id][tmp_cluster_pair.dst_id] < clusters_dist[tmp_cluster_pair.src_id][w_id]){
                    // dist(x, w) > dist(x, x.closest), elaborate all clusters to compute distance from x
                    minheap[i].distance = tmp_cluster_pair.distance = 1e9;
                    map<int, CLUSTER> :: iterator it;
                    for(it = k_cluster.begin(); it != k_cluster.end(); it++){
                        int cluster_id = it->first;
                        if(cluster_id != tmp_cluster_pair.src_id && clusters_dist[tmp_cluster_pair.src_id][cluster_id] < tmp_cluster_pair.distance ){
                            minheap[i].dst_id = tmp_cluster_pair.dst_id = cluster_id;
                            minheap[i].distance = tmp_cluster_pair.distance = clusters_dist[tmp_cluster_pair.src_id][cluster_id];
                        }
                    }

                }else {
                    // dist(x, w) < dist(x, x.closet), w directly becomes shortest cluster pair
                    minheap[i].dst_id = tmp_cluster_pair.dst_id = w_id;
                    minheap[i].distance = tmp_cluster_pair.distance = clusters_dist[tmp_cluster_pair.src_id][w_id];
                }
            }else{
                // 2. x.closest is not u or v, so compare dist(x, w) and dist(x, x.closest)
                // dist(x, w) < dist(x, x.closest) , x.closest = w
                if(clusters_dist[tmp_cluster_pair.src_id][w_id] < tmp_cluster_pair.distance){
                    minheap[i].dst_id = tmp_cluster_pair.dst_id = w_id;
                    minheap[i].distance = tmp_cluster_pair.distance = clusters_dist[tmp_cluster_pair.src_id][w_id];
                }
            }
        }
    }
    minheap.push_back(w_pair);
    sort_heap (minheap.begin(), minheap.end());
    return minheap;
}

double Cure::distance_from_cluster_pair(CLUSTER u, CLUSTER v){
    // elaborate two representative points in cluster_i and cluster_j
    unsigned int nreps_cluster_u = u.reps.size();
    unsigned int nreps_cluster_v = v.reps.size();
    double res = 1e9;
    extern map<string, vector<double> > score;
    for(unsigned int user_u=0; user_u < nreps_cluster_u; user_u++){
        for(unsigned int user_v=0; user_v < nreps_cluster_v; user_v++){
            string username_u = u.reps[user_u];
            string username_v = v.reps[user_v];
            if(users_dist[product_name_to_id[username_u]][product_name_to_id[username_v]] + 1e-7 > 0){
                // already know distance between u and v
                res = min(res, users_dist[product_name_to_id[username_u]][product_name_to_id[username_v]]);
            }else {
                // caculate distance between u and v
                unsigned int product_size = min(score[username_u].size(), score[username_v].size());
                double distance = 0;
                double ncommon_product = 0;

                // compute two user distance
                for(unsigned int product_id = 0; product_id < product_size; product_id++){
                    if( (score[username_u][product_id] + 1e-7) > 0 && (score[username_v][product_id]) + 1e-7 > 0){
                        distance = distance + abs(score[username_u][product_id] - score[username_v][product_id]);
                        ncommon_product++;
                    }
                }
                if(ncommon_product > 1e-7){
                    distance /= ncommon_product;
                    res = min(res, distance);
                    users_dist[product_name_to_id[username_u]][product_name_to_id[username_v]] =
                    users_dist[product_name_to_id[username_v]][product_name_to_id[username_u]] = distance;

                }
            }
        }
    }
    return res;
}
