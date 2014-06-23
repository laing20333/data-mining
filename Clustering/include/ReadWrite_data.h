#ifndef ReadWrite_data_H
#define ReadWrite_data_H

#include<map>
#include<Definition.h>

using namespace std;

class ReadWrite_data{
public:
    ReadWrite_data();
    ~ReadWrite_data();
    void read_original_data(char input_path[]);
    void read_answer_score_data(char answer_path[]);
    void write_output_file(char input_path[], char output_path[]);
    double fill_question_mark(string username, int product_id, K_CLUSTER k_cluster);
    void make_user_to_cluster_id_table(K_CLUSTER k_cluster);

    map<string, int> product_name_to_id;
    double *product_average_score;
    K_CLUSTER k_cluster;
    double RMSE;

private:
    USER_TO_CLUSTER_ID_TABLE user_to_cluster_id_table;
    double answer_score[MAX_USER_NUM][MAX_PRODUCT_NUM];
    double nquestion_mark;
};

#endif // ReadWrite_data_H
