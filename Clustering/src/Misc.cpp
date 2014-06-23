#include<Misc.h>
#include<memory.h>
#include<iostream>
#include<cstdio>

using namespace std;

void Misc::normalize_user_score(){
    // 2 3 4 6 -> 0 1 2 4 -> 0 0.25 0.5 1
    extern map<string, vector<double> > score;
    map<string, vector<double> > :: iterator it;
    for(it = score.begin(); it != score.end(); it++){
        double tmp_max = -1;
        double tmp_min = 1e9;
        for(unsigned int p=0; p<it->second.size(); p++){
            if(it->second[p] > 0 ){
                if(it->second[p] > tmp_max){
                    tmp_max = it->second[p];
                }
                if(it->second[p] < tmp_min){
                    tmp_min = it->second[p];
                }
            }
        }
        tmp_max_min[it->first] = make_pair(tmp_max, tmp_min);
        for(unsigned int p=0; p<it->second.size(); p++){
            if(it->second[p] > 0 ){
                (it->second[p]) -= tmp_min ;
                (it->second[p]) /= (tmp_max - tmp_min);
                it->second[p] += 1e-5;
            }
        }
    }
}

void Misc::undo_normalize_user_score(){
    extern map<string, vector<double> > score;
    map<string, vector<double> > :: iterator it;
    for(it = score.begin(); it != score.end(); it++){
        for(unsigned int p=0; p<it->second.size(); p++){
            (it->second[p]) *= (tmp_max_min[it->first].first - tmp_max_min[it->first].second);
            (it->second[p]) += (tmp_max_min[it->first].second);
        }
    }
    return;
}


double* Misc::get_all_product_average_score(int nproduct){
    // caculate average score in every product
    extern map<string, vector<double> > score;
    double *product_average_score = new double[MAX_PRODUCT_NUM];
    memset(product_average_score, 0, sizeof(double) * MAX_PRODUCT_NUM);
    for(int product_id =0; product_id < nproduct; product_id++){
        int njudged = 0;
        map<string, vector<double> > :: iterator it;
        for(it = score.begin(); it != score.end(); it++){
            int nproduct_judged_by_user = it->second.size();
            if(product_id < nproduct_judged_by_user && it->second[product_id] + 1e-7 > 0){
                product_average_score[product_id] += (it->second[product_id]);
                njudged++;
            }
        }
        if(njudged){
            product_average_score[product_id] /= njudged;
        }
    }
    // caculate all product average score
    double all_product_average_score = 0;
    double nscore = 0;
    for(int product_id =0; product_id < nproduct; product_id++){
        if(product_average_score[product_id] - 1e-7 > 0){
            all_product_average_score += product_average_score[product_id];
            nscore++;
        }
    }
    all_product_average_score /= nscore;

    // fill non average score product
    for(int product_id =0; product_id < nproduct; product_id++){
        if(product_average_score[product_id] - 1e-7 < 0){
            product_average_score[product_id] = all_product_average_score;
        }
    }
    return product_average_score;
}


void Misc::show_result(K_CLUSTER k_cluster){
    cout << "-> Result:" << endl;
    map<int, CLUSTER> :: iterator it;
    int cluster_id = 1;
    for(it = k_cluster.begin(); it != k_cluster.end(); it++){
        printf("-> cluster #%d: %d cluster(s)\n", cluster_id++, it->second.users.size());
        for(unsigned int j=0; j<it->second.users.size(); j++){
            cout << it->second.users[j].name << " ";
        }
        cout << endl;
    }

    // average score for every product


    //cout << minheap.size() << endl;
    //cout << minheap.front().src_id << " " << minheap.front().dst_id << " " << minheap.front().distance << endl;

}
