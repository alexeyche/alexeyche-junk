#include "Classify.h"

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define abs(x)   ((x) >0 ?(x):-(x))

#define MAX_CHAR_PER_LINE       200000

#define DEBUG 0

using namespace std;

/**************************************/
/*********  Type  Declaration  ********/
/**************************************/
typedef vector< vector<double> > Data_type;
typedef vector< vector<int> >    CFMatrix_Type;


/*****************************************/
/*******  Variables  Declaration  ********/
/*****************************************/
Data_type           Data;
vector<int>         Label;
vector<Shapelet>    Final_Sh;
vector<bool>        Leaf_list;
vector<int>         Classify_list;
unordered_map<int,int> Local_Label;     /// map from real_class  -> local_class
vector<int>         Real_Label;         /// map from local_class ->  real_class

int num_class, num_obj, subseq_len;
int max_node_id;


void error(int type)
{
    switch(type)
    {
        case 0:
                printf(" Please check the input parameters.\n\n");
                printf(" == Syntax ==\n");
                printf(" Classify.exe  Test_Data  Number_of_Class  Number_of_Instance  [input_tree_file] [output_acc_file] \n\n");
                printf(" == Examples ==\n");
                printf(" Classify.exe  ECG200_TRAIN   2  100  \n");
                printf(" Classify.exe  CBF_TEST       3  900  tree_CBF\n");
                printf(" Classify.exe  GUN_POINT_TEST 2  150  tree_GUN  acc_GUN\n");
                printf("\n");
                break;
        case 1:
                printf("Error!! Input file  not found.");
                break;
        case 2:
                printf("Error!! Number of Classes is more than input parameters. Please check.\n");
                break;
    }
    exit(0);
}


/// Read the Tran data and keep it into 2D vector of double)
void ReadData(char* filename)
{
    FILE *f;
    f = fopen(filename, "r");
    if (f==NULL)    error(1);
    char buff[MAX_CHAR_PER_LINE];
    char *tmp;
    int real_class_id;

    /// Note that Each line must have the same number of objects
    for(int i=0; i<num_obj; i++)
    {
        vector <double>  cur_ts;
        fgets(buff, MAX_CHAR_PER_LINE, f);

        tmp = strtok(buff, " \r\n");
        real_class_id = (int)atof(tmp);
        Label.push_back(real_class_id);

        tmp = strtok(NULL, " \r\n");
        while(tmp != NULL)
        {
            cur_ts.push_back(atof(tmp));
            tmp = strtok(NULL," \r\n");
        }
        Data.push_back(cur_ts);
    }

    int local_class_id = 0;
    unordered_map<int,int>::const_iterator it;
    for(int i=0; i<num_class; i++)
        Real_Label.push_back(0);
    for(unsigned int i=0; i<Label.size(); i++)
    {   real_class_id = Label[i];
        it = Local_Label.find(real_class_id);
        if (it == Local_Label.end())
        {   /// New label
            Local_Label[real_class_id] = local_class_id;
            Real_Label[local_class_id] = real_class_id;
            local_class_id++;
            if (local_class_id > num_class)
                error(2);
        }
    }

    fclose(f);
}

void ReadShapeletTree(char* result_file_name)
{
    FILE *f;
    f = fopen(result_file_name, "r");
    if (f==NULL)    error(1);
    char buff[MAX_CHAR_PER_LINE];
    char *tmp;

//    fprintf(fo,"Node %2d    %2s    %3d  %3d  %3d  %7.4f ",i,"--",sh.obj,sh.pos,sh.len,sh.dist_th);
    /// Note that Each line must have the same number of objects

    Shapelet sh;
    unsigned int tree_size = 0;
    int id, obj, pos, len, label, m;
    double dist_th, d;

    while (fgets(buff, MAX_CHAR_PER_LINE, f)!=NULL)
    {
        //printf("buff = %s\n",buff);

        tmp = strtok(buff, " \r\n");
        if ((tmp==NULL)||(tmp[0] == ';'))  ///comment
        {   continue;
        }else if(tmp[0] == 'T') /// "TreeSize" means last node id in Final_Sh
        {   tmp = strtok(NULL," \r\n");
            tree_size = atoi(tmp);
            max_node_id = tree_size;
            printf("==> Tree Size = %d\n",tree_size);
            while(Final_Sh.size() < tree_size)
            {
                 Final_Sh.push_back(sh);
                 Leaf_list.push_back(false);
                 Classify_list.push_back(-1);
            }
        }
        else if(tmp[0] == 'L') /// "Leaf" means leaf-node
        {
            //NonL  6    --      6    5   70   1.9373 ==>   0   7  /   7   8
            //Leaf  7    -1
            tmp = strtok(NULL," \r\n");  id = atoi(tmp);
            tmp = strtok(NULL," \r\n");  label = atoi(tmp);
            Leaf_list[id]=true;
            Classify_list[id]=label;

        }else if(tmp[0] == 'N') /// "NonL" means non-leaf node
        {
            tmp = strtok(NULL," \r\n");  id = atoi(tmp);
            tmp = strtok(NULL," \r\n");
            tmp = strtok(NULL," \r\n");  obj = atoi(tmp);
            tmp = strtok(NULL," \r\n");  pos = atoi(tmp);
            tmp = strtok(NULL," \r\n");  len = atoi(tmp);
            tmp = strtok(NULL," \r\n");  dist_th = atof(tmp);
            //printf("id=%d, obj=%d, pos=%d, len=%d, dist_th=%f\n",id,obj,pos,len,dist_th);
            Leaf_list[id]=false;
            Final_Sh[id].setValueTree(obj,pos,len,dist_th);
        }else if(tmp[0] == 'S') /// Shapelet Subsequence
        {
            tmp = strtok(NULL," \r\n");  id = atoi(tmp);
            m = Final_Sh[id].len;
            vector<double> query(m);
            for(int j=0; j<m; j++)
            {
                tmp = strtok(NULL," \r\n");  d = atof(tmp);
                query[j] = d;
            }
            Final_Sh[id].setTS(query);
        }else
        {
            printf("Error!!! Result file is messed up.    First letter is [0]=%c \n",tmp[0]);
        }
    }
    fclose(f);
}


void Classification(int &count_correct, int &count_wrong, CFMatrix_Type &Matrix)
{
    int node_id, m, predicted_label;
    double d, dist_th;
    count_correct=0; count_wrong=0;

    for(unsigned int obj=0; obj<Data.size(); obj++)
    {
        /// start at the top node
        node_id = 1;
        while((Leaf_list[node_id]==false)||(node_id > max_node_id))
        {

            m=Final_Sh[node_id].len;
            double* Q = (double *)malloc(sizeof(double)*m);
            int* order = (int *)malloc(sizeof(int)*m);
            if( Q == NULL )     error(3);
            if( order == NULL ) error(3);

//            printf("query => ");
//            for(int k=0; k<3; k++)
//                printf("%.4f ",Final_Sh[node_id].ts[k]);
//            printf("\n");

            d = NearestNeighborSearch(Final_Sh[node_id].ts, Data[obj], 0, Q, order);
            dist_th = Final_Sh[node_id].dist_th;
            free(Q);
            free(order);

            //printf("  node_id: %d,  d=%f   dist_th=%f\n",node_id,d,dist_th);

            if (d <=dist_th)
                node_id = 2*node_id;
            else
                node_id = 2*node_id + 1;

        }

        if (node_id > max_node_id)
            printf("Error!!!, node_id > max_node_id");

        predicted_label = Classify_list[node_id];
        if (predicted_label == Label[obj])
            count_correct++;
        else
            count_wrong++;

#if DEBUG >=1
        printf("obj: %3d,   real: %2d,    predict: %2d \n",obj,Label[obj],predicted_label);
#endif

        int predicted_local_label = Local_Label[predicted_label];
        int real_local_label = Local_Label[Label[obj]];
        Matrix[predicted_local_label][real_local_label]++;
    }
}

void printMatrix(FILE *fo, CFMatrix_Type Matrix)
{
    fprintf(fo,"\n");
    fprintf(fo,";   == Confusion Matrix ==\n");
    fprintf(fo,";   real |");
    for(unsigned int i=0; i<Matrix.size(); i++)
        fprintf(fo,"%4d ",Real_Label[i]);
    fprintf(fo,"\n");
    fprintf(fo,";   -----+");
    for(unsigned int i=0; i<Matrix.size(); i++)
        fprintf(fo,"------");
    fprintf(fo,"\n");

    for(unsigned int i=0; i<Matrix.size(); i++)
    {
        fprintf(fo,";   p %2d |",Real_Label[i]);
        for(unsigned int j=0; j<Matrix[i].size(); j++)
            fprintf(fo,"%4d ",Matrix[i][j]);
        fprintf(fo,"\n");
    }
    fprintf(fo,"\n");
}

void printCommand(FILE *fo, int argc, char** argv)
{
    fprintf(fo,"; command =");
    for(int i=0; i<argc; i++)
        fprintf(fo," %s",argv[i]);
    fprintf(fo,"\n");
}

int main(int argc, char* argv[])
{
    char* test_file_name;

    /********************************/
    /******* Input Parameters *******/
    /********************************/
    if (argc < 4)    error(0);
    test_file_name = argv[1];
    num_class = atoi(argv[2]);
    num_obj   = atoi(argv[3]);

    char tree_file_name[100] = "tree.txt";
    if (argc > 4)   strcpy(tree_file_name, argv[4]);

    char time_file_name[100] = "time.txt";
    if (argc > 5)   strcpy(time_file_name, argv[5]);

    CFMatrix_Type Matrix;
    for(int i=0; i<num_class; i++)
    {   vector <int> tmp;
        for (int j=0; j<num_class; j++)
            tmp.push_back(0);
        Matrix.push_back(tmp);
    }

    printf("num_class=%d, num_obj=%d   %s  %s\n\n",num_class,num_obj, tree_file_name, time_file_name);

    ReadShapeletTree(tree_file_name);
    ReadData(test_file_name);

    int count_correct=0, count_wrong=0;
    Classification(count_correct, count_wrong, Matrix);

    FILE* f_time = fopen(time_file_name,"a");
    FILE* f_tree = fopen(tree_file_name,"a");

    printCommand(f_time,argc,argv);
    printCommand(f_tree,argc,argv);

    fprintf(f_time,"; accuracy= %.3f   Correct:%d , Wrong: %d\n\n",(double)count_correct/(count_correct+count_wrong)*100,count_correct,count_wrong);
    fprintf(f_tree,"; accuracy= %.3f   Correct:%d , Wrong: %d\n\n",(double)count_correct/(count_correct+count_wrong)*100,count_correct,count_wrong);
    fprintf(stderr,"; accuracy= %.3f   Correct:%d , Wrong: %d\n\n",(double)count_correct/(count_correct+count_wrong)*100,count_correct,count_wrong);

    printMatrix(stderr, Matrix);
    printMatrix(f_tree, Matrix);

    fclose(f_time);
    fclose(f_tree);
}
