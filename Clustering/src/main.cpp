/*
* CURE - A clustering algorithm
* Copyright (C) 2014 laing20333 <laing20333@gmail.com>
*/
#include<iostream>
#include<cstdio>
#include<memory.h>
#include<Cure.h>
#include<Misc.h>
#include<ReadWrite_data.h>

using namespace std;

char input_path[MAX_PATHNAME_LEN];
char output_path[MAX_PATHNAME_LEN];
char answer_path[MAX_PATHNAME_LEN];
unsigned int max_nreps = 5;
unsigned int k = 5;
map<string, vector<double> > score;

ReadWrite_data rw_data;
Cure cure(max_nreps , k);
Misc misc;

int main(){

    sprintf(input_path, "%s", "data1_2.txt");
    sprintf(output_path, "%s", "predict_answer.txt");
    sprintf(answer_path, "%s", "data1_2_answer.txt");

    // Read input data
    rw_data.read_original_data(input_path);
    cure.product_name_to_id = rw_data.product_name_to_id;

    // Compute all product average_score
    int nproduct = rw_data.product_name_to_id.size();
    rw_data.product_average_score = misc.get_all_product_average_score(nproduct);

    //misc.normalize_user_score();

    // Run CURE Algorithm
    K_CLUSTER k_cluster = cure.cure();
    rw_data.k_cluster = k_cluster;

    //misc.undo_normalize_user_score();

    // Read question's answer and Write predict_answer to file
    rw_data.read_answer_score_data(answer_path);
    rw_data.write_output_file(input_path, output_path);

    // Show results
    misc.show_result(k_cluster);
    cout << "-> RMSE: " << rw_data.RMSE << endl;

    return 0;
}

