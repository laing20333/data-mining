#include<ReadWrite_Data.h>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<sstream>
#include<cstring>
#include<cmath>

using namespace std;


ReadWrite_data::ReadWrite_data(){

}

ReadWrite_data::~ReadWrite_data(){
    delete [] product_average_score;
}

void ReadWrite_data::read_original_data(char input_path[]){
    extern map<string, vector<double> > score;
    //FILE *fin = fopen("small_data.txt", "r");
    FILE *fin = fopen(input_path, "r");
    if(fin == false){
        cerr << "-> Error: Fail to open the file" << endl;
        exit(0);
    }

    char line[MAX_LINE_LEN];

    // store product_id_to_name
    fgets(line, sizeof(line), fin);
    stringstream ss(line);
    string product_name;
    getline(ss, product_name, ','); // ignore attribute name
    int idx = 0;
    while(getline(ss, product_name, ',') != NULL){
        product_name_to_id[product_name] = idx++;
    }

    // store rating matrix
    char username[MAX_ID_LEN];
    char score_line[MAX_LINE_LEN];
    while(fgets(line, sizeof(line), fin) != NULL){
        sscanf(line, "%[^,]%s", username, score_line);
        int sz = strlen(score_line);
        for(int i=0; i<sz; i++){
            string product_score;
            product_score += score_line[i];
            if(isdigit(score_line[i]) || score_line[i] == '.'){
                while(i+1 < sz && (isdigit(score_line[i+1]) || score_line[i+1] == '.')){
                    product_score += score_line[i+1];
                    i++;
                }
            }
            if(product_score == "?"){
                score[username].push_back(QUESTION_MARK);
            }else if(product_score == ","){
                score[username].push_back(NON_SCORE);
            }else{
                double product_score_double;
                sscanf(product_score.c_str(), "%lf", &product_score_double);
                score[username].push_back(product_score_double);
            }
        }
    }

    fclose(fin);
    cout << "-> read_original_data ...ok!" << endl;
}

void ReadWrite_data::read_answer_score_data(char answer_path[]){
    static FILE *fin = fopen(answer_path, "r");
    if(fin == false){
        cerr << "-> Error: Fail to open the file" << endl;
        return;
    }

    int user_id = 0;
    char line[MAX_LINE_LEN];

    // ignore first line
    fgets(line, sizeof(line), fin);
    while(fgets(line, sizeof(line), fin) != NULL){
        char username[MAX_ID_LEN];
        char score_line[MAX_LINE_LEN];
        sscanf(line, "%[^,]%s", username, score_line);
        int score_len = strlen(score_line);
        int product_id = 0;

        for(int idx=0; idx < score_len; idx++){
            string product_score;
            product_score += score_line[idx];
            // get complete score
            if(isdigit(score_line[product_id]) || score_line[product_id] == '.'){
                while(idx+1 < score_len && (isdigit(score_line[idx+1]) || score_line[idx+1] == '.') ){
                    product_score += score_line[idx+1];
                    idx++;
                }
            }
            // put score into answer_score array
            if(isdigit(product_score[0])){
                sscanf(product_score.c_str(), "%lf", &answer_score[user_id][product_id]);
            }
            product_id++;
        }
        user_id++;
    }
    fclose(fin);
    cout << "-> read_answer_score_data ...ok!" << endl;
    return;

}

void ReadWrite_data::write_output_file(char input_path[], char output_path[]){
    FILE *fin = fopen(input_path, "r");
    FILE *fout = fopen(output_path, "w");
    if(fin == false || fout == false){
        cerr << "-> Error: Fail to open the file" << endl;
        return;
    }

    // get user's cluster_id table
    make_user_to_cluster_id_table(k_cluster);

    // output product name
    char line[MAX_LINE_LEN];
    fgets(line, sizeof(line), fin);
    fputs(line, fout);

    // user and user score
    char username[MAX_ID_LEN];
    char score_line[MAX_LINE_LEN];
    int user_id = 0;
    double sum_of_difference_square = 0;
    while(fgets(line, sizeof(line), fin) != NULL){
        sscanf(line, "%[^,]%s", username, score_line);
        fputs(username, fout);
        int sz = strlen(score_line);
        int product_id = 0;
        for(int idx=0; idx < sz; idx++){
            string product_score;
            product_score += score_line[idx];

            // get complete score
            if(isdigit(score_line[product_id]) || score_line[product_id] == '.'){
                while(idx+1 < sz && (isdigit(score_line[idx+1]) || score_line[idx+1] == '.') ){
                    product_score += score_line[idx+1];
                    idx++;
                }
            }

            if(product_score == "?"){
                // deal with Question mark
                double predict_answer = fill_question_mark(username, product_id, k_cluster);
                fprintf(fout, "%lf", predict_answer);

                // update RMSE
                sum_of_difference_square += (abs(predict_answer - answer_score[user_id][product_id]) * abs(predict_answer - answer_score[user_id][product_id]));
                nquestion_mark++;

            }else {
                // ',' or already has score, directly output it
                fputs(product_score.c_str(), fout);
            }
            product_id++;
        }
        user_id++;
        fputs("\n", fout);
    }

    // compute RMSE
    RMSE = sqrt(sum_of_difference_square / nquestion_mark);

    fclose(fin);
    fclose(fout);
    cout << "-> output_answer_file ...ok!" << endl;
    return;


}

double ReadWrite_data::fill_question_mark(string username, int product_id, K_CLUSTER k_cluster){
    double res = 0;
    double ncommon = 0;
    CLUSTER cluster = k_cluster[user_to_cluster_id_table[username]];
    int cluster_size = cluster.users.size();
    extern map<string, vector<double> > score;

    // caculate average product score by this user's cluster users
    for(int i=0; i<cluster_size; i++){
        string tmp_username = cluster.users[i].name;
        if(score[tmp_username][product_id] + 1e-7 > 0){
            res += score[tmp_username][product_id];
            ncommon++;
        }
    }
    if(ncommon){
        // some people judge this product
        res /= ncommon;
    }else {
        // no one judge this product
        res = product_average_score[product_id];
    }
    return res;
}

void ReadWrite_data::make_user_to_cluster_id_table(K_CLUSTER k_cluster){
    K_CLUSTER :: iterator it;

    for(it = k_cluster.begin(); it != k_cluster.end(); it++){
        int user_size = it->second.users.size();
        for(int i=0; i<user_size; i++){
            user_to_cluster_id_table[it->second.users[i].name] = it->first;
        }
    }
    return ;
}
