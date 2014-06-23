#ifndef Definition_H
#define Definition_H

#include<string>
#include<vector>
#include<map>
using namespace std;

#define MAX_LINE_LEN 100001
#define MAX_PRODUCT_NUM 10001
#define MAX_ID_LEN 1001
#define MAX_USER_NUM 1001
#define MAX_CLUSTER_NUM 1001
#define MAX_PATHNAME_LEN 1001
#define NON_SCORE -1
#define QUESTION_MARK -2
#define ALPHA 0.5

struct USER{
    string name;
    double mean;
};
struct CLUSTER{
    vector<USER> users;
    vector<string> reps;
    double mean;
};

struct CLUSTER_PAIR{
    int src_id;
    int dst_id;
    double distance;
};

typedef map<int, CLUSTER> K_CLUSTER;
typedef vector<CLUSTER_PAIR> HEAP;
typedef map<string, int> USER_TO_CLUSTER_ID_TABLE;

#endif // Definition_H


