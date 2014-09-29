#include "FastShapelet.h"

#define MAX_CHAR_PER_LINE       200000
#define SH_MIN_LEN              5
#define INF                     1e8

#define DEBUG 0

/// Default Parameters
#define MAX_PURITY_SPLIT        0.90
#define MIN_PERCENT_OBJ_SPLIT   0.1
#define EXTRA_TREE_DEPTH        2

int MIN_OBJ_SPLIT;
/**************************************/
/*******  Variable Declaration  *******/
/**************************************/


int num_class, num_obj, subseq_len;

Data_type       Org_Data, Data;
vector<int>     Org_Label, Label;
vector<int>     Org_Class_Freq, Class_Freq;
USAX_Map_type   USAX_Map;
Score_List_type Score_List;
double          class_entropy;

Node_Obj_set_type   Node_Obj_List;
int                 node_id;
Classify_type       Classify_list;          //-1: Body, otherwise (logical):class

unordered_map<int,int>  Real_Class_Label;
vector<Shapelet>        Final_Sh;

/**************************************/
/*******  Function Declaration  *******/
/**************************************/

void error(int type)
{
    switch(type)
    {
        case 0:
                printf(" Please check the input parameters.\n\n");
                printf(" == Syntax ==\n");
                printf(" shapelet.exe  Train_Data   Number_of_Class  Number_of_Instance  Max_Length  Min_Length  Step [R] [top_k] [output_tree_file] [output_time_file]\n\n");
                printf(" == Examples ==\n");
                printf(" shapelet.exe  ECG200_TRAIN  2  100  96  1  1  \n");
                printf(" shapelet.exe  ECG200_TRAIN  2  100  96  1  1 10  \n");
                printf(" shapelet.exe  ECG200_TRAIN  2  10   30  20 1 10  tree_ECG.txt  time_ECG.txt\n");
                printf("\n");
                break;
        case 1:
                printf("Error!! Input file  not found.");
                break;
        case 2:
                printf("Error!! The input Dataset has more class than define. Please check.\n");
                break;
        case 3:
                printf("Error!! Memory can't be allocated!!!\n\n");
                break;
        case 4:
                printf("Error!! bsf_shapelet is empty.\n");
                break;
        case 5:
                printf("Error!! Max_Length < Min_Length.\n");
                break;
    }
    exit(0);
}


/// Read the Tran data and keep it into 2D vector of double)
void ReadTrainData(char* filename)
{
    FILE *f;
    f = fopen(filename, "r");
    if (f==NULL)    error(1);



    char buff[MAX_CHAR_PER_LINE];
    char *tmp;
    unordered_map<int,int> relabel;
    unordered_map<int,int>::const_iterator it;
    int class_id = 0, cur_class, real_class_id, tmp_class_id;

    for(int i=0; i<num_class; i++)
    {   Org_Class_Freq.push_back(0);
    }

    /// Note that Each line must have the same number of objects
    for(int i=0; i<num_obj; i++)
    {
        vector <double>  cur_ts;
        fgets(buff, MAX_CHAR_PER_LINE, f);
        tmp = strtok(buff, " \r\n");

        tmp_class_id = (int)atof(tmp);
        it = relabel.find(tmp_class_id);

        if (it == relabel.end())
        {   /// New label
            real_class_id = tmp_class_id;
            relabel[real_class_id] = class_id;
            cur_class = class_id;
            class_id++;
            if (class_id > num_class)    error(2);
        }
        else
        {   cur_class = it->second;
        }

        Org_Label.push_back(cur_class);
        Org_Class_Freq[cur_class]++;

        tmp = strtok(NULL, " \r\n");
        while(tmp != NULL)
        {
            cur_ts.push_back(atof(tmp));
            tmp = strtok(NULL," \r\n");
        }
        Org_Data.push_back(cur_ts);
    }

    /// Keep the real labels which are read from data, i.e., Real_Class_Label[new_label] = real_label
    for(int i=0; i<=num_class; i++)
        Real_Class_Label[i]=-1;
    for(it=relabel.begin(); it!=relabel.end(); it++)
        Real_Class_Label[it->second] = it->first;

#if DEBUG>=0
    printf("relabel [new,real] => ");
    for(it=relabel.begin(); it!=relabel.end(); it++)
        printf("[%d,%d]  ", it->second, it->first);
    printf("\n");
#endif
}

/// Fix card = 4 here !!!
SAX_word_type CreateSAXWord(const double* sum_segment, const int* elm_segment, double mean, double std, int sax_len)
{   SAX_word_type word=0, val=0;
    double d = 0;

    for(int i=0; i<sax_len; i++)
    {
        d = (sum_segment[i]/elm_segment[i]-mean)/std;
        if (d < 0)
            if (d < -0.67)  val = 0;
            else            val = 1;
        else
            if (d < 0.67)   val = 2;
            else            val = 3;

        word =  (word << 2) | (val);
    }
    return word;
}


void CreateSAXList(int subseq_len, int sax_len, int w)
{
    double ex, ex2, mean, std;
    double sum_segment[sax_len];
    int elm_segment[sax_len];
    int obj_id,j,j_st,k, slot;
    double d;
    SAX_word_type word, prev_word;
    USAX_elm_type *ptr;

    for(k=0; k<sax_len; k++)
        elm_segment[k] = w;
    elm_segment[sax_len-1] = subseq_len - (sax_len-1)*w;

    for(obj_id=0; obj_id<(int)Data.size(); obj_id++)
    {   ex = ex2 = 0;
        prev_word = -1;

        for(k=0; k<sax_len; k++)
            sum_segment[k] = 0;

        /// Case 1: Initial
        for(j=0; (j<(int)Data[obj_id].size()) && (j<subseq_len); j++)
        {
            d = Data[obj_id][j];
            ex += d;
            ex2 += d*d;
            slot = (int)floor((j)/w);
            sum_segment[slot] += d;
        }

        /// Case 2: Slightly Update
        for(j=j; (j<=(int)Data[obj_id].size()) ; j++)
        {
            j_st = j-subseq_len;

            mean = ex/subseq_len;
            std = sqrt(ex2/subseq_len - mean*mean);

            /// Create SAX from sum_segment
            word = CreateSAXWord(sum_segment, elm_segment, mean, std, sax_len);

            if (word != prev_word)
            {   prev_word = word;
                ptr = &USAX_Map[word];
                ptr->obj_set.insert(obj_id);
                ptr->sax_id.push_back( std::move(make_pair(obj_id, j_st)) );
            }


            /// For next update
            if (j<(int)Data[obj_id].size())
            {
                ex -= Data[obj_id][j_st];
                ex2 -= Data[obj_id][j_st]*Data[obj_id][j_st];

                for (k=0; k<sax_len-1; k++)
                {
                    sum_segment[k] -= Data[obj_id][j_st+(k)*w];
                    sum_segment[k] += Data[obj_id][j_st+(k+1)*w];
                }
                sum_segment[k] -= Data[obj_id][j_st+(k)*w];
                sum_segment[k] += Data[obj_id][j_st+min((k+1)*w, subseq_len)];

                d = Data[obj_id][j];
                ex += d;
                ex2 += d*d;
            }
        }
    }
}


/// create mask word (two random may give same position, we ignore it)
SAX_word_type CreateMaskWord(int num_mask, int word_len)
{   SAX_word_type a,b;
    a = 0;
    for(int i=0; i<num_mask; i++)
    {
        b = 1 << (rand()%word_len);
        a = a | b;
    }
    return a;
}


/// Count the number of occurrences
void RandomProjection(int R, double percent_mask, int sax_len)
{
    Hash_Mark_type  Hash_Mark;

    USAX_Map_type::iterator it;
    SAX_word_type word, mask_word, new_word;
    Obj_set_type *obj_set, *ptr;
    Obj_set_type::iterator o_it;

    int num_mask = ceil(percent_mask * sax_len);

    for(int r=0; r<R; r++)
    {   mask_word = CreateMaskWord(num_mask, sax_len);

        /// random projection and mark non-duplicate object
        for(it=USAX_Map.begin(); it!=USAX_Map.end(); it++)
        {
            word = it->first;
            obj_set = &(it->second.obj_set);

            new_word = word | mask_word;
            ptr = &Hash_Mark[new_word];
            ptr->insert(obj_set->begin(), obj_set->end());
        }

        /// hash again for keep the count
        for(it=USAX_Map.begin(); it!=USAX_Map.end(); it++)
        {
            word = it->first;
            new_word = word | mask_word;
            obj_set = &(Hash_Mark[new_word]);
            for (o_it=obj_set->begin(); o_it!=obj_set->end(); o_it++)
            {
                (it->second.obj_count[*o_it])++;
            }
        }
        Hash_Mark.clear();
    }
}


/// Score each sax in the matrix
double CalScoreFromObjCount(vector<double> c_in, vector<double> c_out)
{
    // /// 2 classes only
    //return abs((c_in[0]+c_out[1]) - (c_out[0]+c_in[1]));

    /// multi-class
    double diff, sum=0, max_val=-INF, min_val=INF;
    for(int i=0; i<num_class; i++)
    {
        diff = (c_in[i]-c_out[i]);
        if (diff > max_val) max_val=diff;
        if (diff < min_val) min_val=diff;
        sum += abs(diff);
    }
    return (sum-abs(max_val)-abs(min_val)) + abs(max_val-min_val);
}

/// ***Cal***
double CalScore(USAX_elm_type & usax, int  R)
{   double score=-1;
    int cid, count;
    Obj_count_type::iterator o_it;
    vector<double> c_in(num_class,0);       // Count object inside hash bucket
    vector<double> c_out(num_class,0);      // Count object outside hash bucket

    /// Note that if no c_in, then no c_out of that object
    for(o_it=usax.obj_count.begin(); o_it!=usax.obj_count.end(); o_it++ )
    {   cid  = Label[o_it->first];
        count  = o_it->second;
        c_in[cid] += (count);
        c_out[cid] += (R-count);
    }
    score = CalScoreFromObjCount(c_in,c_out);
    return score;
}

/// Score each SAX
void ScoreAllSAX(int R)
{
    USAX_Map_type::iterator it;
    SAX_word_type word;
    double score;
    USAX_elm_type usax;

    for(it=USAX_Map.begin(); it!=USAX_Map.end(); it++)
    {
        word = it->first;
        usax = it->second;
        score = CalScore(usax, R);
        Score_List.push_back( std::move(Pair_score_type(word, score)));
        //Score_List.push_back( Pair_score_type(word, score));
    }
}

/// sort score from high to low
int sortScore(Pair_score_type a, Pair_score_type b)
{   return (a.second > b.second);
}
/// sort nn_dist from low to high
int sortDist(Pair_dist_type a, Pair_dist_type b)
{   return (a.second < b.second);
}

/// Old Function but work
double Entropy(int const& x, int const& y)
{   double en, p;
    if ((x==0) || (y==0))
    {   en = 0;
    }else
    {   p = 1.0*x/(x+y);
        en = -p*log(p) - (1-p)*log(1-p);
    }
    return en;
}

/// new function still in doubt (as in Mueen's paper)
double EntropyArray(vector<int> const& A, int const& total)
{   double en=0;
    double a;
    for(int i=0; i<num_class; i++)
    {   a = (double)A[i]/total;
        if (a>0)
            en -= a*log(a);
    }
    return en;
}

/// new function as in in riva; paper)
double CalInfoGain2(vector<int> const& c_in, vector<int> const& c_out, int const &total_c_in, int const &total_c_out)
{
    return class_entropy - ( (double)(total_c_in)/num_obj*EntropyArray(c_in, total_c_in) + (double)(total_c_out)/num_obj*EntropyArray(c_out, total_c_out) );
}


/// From top-k-score SAX
/// Calculate Real Infomation Gain
Shapelet FindBestSAX(int top_k)
{
    Dist_List_type Dist(num_obj);
    SAX_word_type word;
    double gain, dist_th, gap;
    vector<int> bsf_c_in(num_class), bsf_c_out(num_class);
    int q_obj, q_pos;
    USAX_elm_type usax;
    int label, kk, total_c_in, num_diff;

    Shapelet sh, bsf_sh;
    vector<int> c_in(num_class);
    vector<int> c_out(num_class);
    vector<double> query(subseq_len);

    if( top_k > 0)
        sort(Score_List.begin(), Score_List.end(), sortScore);
    top_k = abs(top_k);

    for(int k=0; k<min(top_k, (int)Score_List.size()); k++)
    {   word = Score_List[k].first;
        usax  = USAX_Map[word];
        for(kk=0; kk < min((int)usax.sax_id.size(), 1); kk++)
        {   q_obj = usax.sax_id[kk].first;
            q_pos = usax.sax_id[kk].second;

            for(int i=0; i<num_class; i++)
            {   c_in[i]=0;
                c_out[i]=Class_Freq[i];
            }
            for(int i=0; i<subseq_len; i++)
            {   query[i] = Data[q_obj][q_pos+i];
            }

            double dist;
            int m=query.size();
            double* Q = (double *)malloc(sizeof(double)*m);
            int* order = (int *)malloc(sizeof(int)*m);
            if( Q == NULL )     error(3);
            if( order == NULL ) error(3);
            for(int obj=0; obj<num_obj; obj++)
            {   dist = NearestNeighborSearch(query, Data[obj], obj, Q, order);
                Dist[obj] = Pair_dist_type(obj,dist);
            }
            free(Q);
            free(order);

            sort(Dist.begin(), Dist.end(), sortDist);

            total_c_in = 0;
            for(int i=0; i<(int)Dist.size()-1; i++)
            {   dist_th = (Dist[i].second + Dist[i+1].second)/2;
                //gap = Dist[i+1].second - dist_th;
                gap = double(Dist[i+1].second - dist_th)/sqrt(subseq_len);
                label = Label[Dist[i].first];
                c_in[label]++;
                c_out[label]--;
                total_c_in++;
                num_diff = abs(num_obj-2*total_c_in);
                //gain = CalInfoGain1(c_in, c_out);
                gain = CalInfoGain2(c_in, c_out, total_c_in, num_obj-total_c_in);

                sh.setValueFew(gain,gap,dist_th);
                if (bsf_sh < sh)
                {
                    bsf_sh.setValueAll(gain,gap,dist_th,q_obj,q_pos,subseq_len,num_diff,c_in,c_out);
                }
            }
        }
    }
#if DEBUG >= 2
    bsf_sh.printLong();
#else
    printf(".");
    if(subseq_len%50==0)
        printf("\n");
#endif

    return bsf_sh;
}

/// Set variables for next node. They are Data, Label, Class_Freq, num_obj
void SetCurData(int node_id)
{
    if (node_id==1)
    {
        Data = Org_Data;
        Label = Org_Label;
        Class_Freq = Org_Class_Freq;
    }else
    {
        Obj_list_type::iterator it;
        num_obj = Node_Obj_List[node_id].size();
        Data.clear();
        Label.clear();
        for(int i=0; i<num_class; i++)
            Class_Freq[i]=0;
        int cur_class;

        for(it=Node_Obj_List[node_id].begin(); it!=Node_Obj_List[node_id].end(); it++)
        {   cur_class = Org_Label[*it];
            if ((cur_class < 0)||(cur_class >= num_class))
                printf("cur_class = %d\n",cur_class);   //xxxxx

            Data.push_back( Org_Data[*it] );
            Label.push_back( cur_class);
            Class_Freq[cur_class]++;
        }
    }
    class_entropy = EntropyArray(Class_Freq, num_obj);


    printf("\n=== Node %d ===\n",node_id);
#if DEBUG >= 1
    /// Global Entropy for the original state
    printf("max entropy = %.4f  <-- ",class_entropy);
    for(int i=0; i<num_class; i++)
        printf(" %d ",Class_Freq[i]);
    printf("  [num_obj=%d][num_class=%d]\n",num_obj,num_class);
#endif
}

/// Create List of Object on next level in the decision tree
/// if the accuracy is high, not create node
void SetNextNodeObj(unsigned int node_id, Shapelet sh)
{
    int q_obj = sh.obj;
    int q_pos = sh.pos;
    int q_len = sh.len;
    double dist_th = sh.dist_th;
    vector<double> query(q_len);
    vector<int> next_level(num_obj);
    int left_num_obj=0, right_num_obj=0;
    unsigned int left_node_id = node_id*2;
    unsigned int right_node_id = node_id*2+1;
    int real_obj;

    /// Memory Allocation
    Obj_list_type tmp;
    Shapelet tmp_sh;
    while(Node_Obj_List.size() <= right_node_id)
    {   Node_Obj_List.push_back(tmp);
        Classify_list.push_back(-2);
        Final_Sh.push_back(tmp_sh);

        if (Node_Obj_List.size()==2)
        {   /// Note that Node_Obj_List[0] is not used
            for(int i=0; i<num_obj; i++)
                Node_Obj_List[1].push_back(i);
        }
    }
    Final_Sh[node_id] = sh;

    /// Use the shapelet on previous Data
    for(int i=0; i<q_len; i++)
    {   query[i] = Data[q_obj][q_pos+i];
    }
    double dist;
    int m=query.size();
    double* Q = (double *)malloc(sizeof(double)*m);
    int* order = (int *)malloc(sizeof(int)*m);
    if( Q == NULL )     error(3);
    if( order == NULL ) error(3);
    for(int obj=0; obj<num_obj; obj++)
    {   dist = NearestNeighborSearch(query, Data[obj], obj, Q, order);
        if (dist <= dist_th)
        {
            left_num_obj++;
            next_level[obj]=1;  ///left

            real_obj = Node_Obj_List[node_id][obj];
            Node_Obj_List[left_node_id].push_back( real_obj );
        }else
        {   right_num_obj++;
            next_level[obj]=2;  ///right

            real_obj = Node_Obj_List[node_id][obj];
            Node_Obj_List[right_node_id].push_back( real_obj );
        }
    }

    free(Q);
    free(order);

    /// If left/right is pure, or so small, stop spliting
    int max_c_in=-1, sum_c_in=0;
    int max_c_out=-1, sum_c_out=0;
    int max_ind_c_in=-1, max_ind_c_out=-1;
    for(unsigned int i=0; i<sh.c_in.size(); i++)
    {
        sum_c_in += sh.c_in[i];
        if (max_c_in < sh.c_in[i])
        {   max_c_in = sh.c_in[i];
            max_ind_c_in = i;
        }
        sum_c_out += sh.c_out[i];
        if (max_c_out < sh.c_out[i])
        {   max_c_out = sh.c_out[i];
            max_ind_c_out = i;
        }
    }

    bool left_is_leaf=false;
    bool right_is_leaf=false;
    MIN_OBJ_SPLIT = ceil( double(MIN_PERCENT_OBJ_SPLIT*num_obj)/num_class );
    if  ((sum_c_in<=MIN_OBJ_SPLIT) || ((double)max_c_in/sum_c_in  >= MAX_PURITY_SPLIT))
        left_is_leaf = true;
    if ((sum_c_out<=MIN_OBJ_SPLIT) || ((double)max_c_out/sum_c_out  >= MAX_PURITY_SPLIT))
        right_is_leaf = true;

    int max_tree_dept = EXTRA_TREE_DEPTH + ceil(log(num_class)/log(2));
    if (node_id >= pow(2, max_tree_dept))
    {   left_is_leaf = true;
        right_is_leaf = true;
    }

    Classify_list[node_id] = -1;
    if (left_is_leaf)
    {   Classify_list[left_node_id] = max_ind_c_in;
    }else
    {   Classify_list[left_node_id] = -1;
    }
    if (right_is_leaf)
    {   Classify_list[right_node_id] = max_ind_c_out;
    }else
    {   Classify_list[right_node_id] = -1;
    }

#if DEBUG >= 2
    printNodeSetList();
#endif
}



void printNodeSetList()
{
    Obj_list_type::iterator it;
    printf("\n Node Set List ==> \n");
    for(unsigned int i=1; i<Node_Obj_List.size(); i++)
    {
        if (Node_Obj_List[i].size()==0)
            continue;
        printf(" Node %2d : [c:%2d] : [%2d] ",i,Classify_list[i], Node_Obj_List[i].size());
        for(it=Node_Obj_List[i].begin(); it!=Node_Obj_List[i].end(); it++)
            printf("%d ",*it);
        printf("\n");
    }
}


/// Important Note: write the Shapelet Tree to the file
void printTree(FILE *fo, int argc, char** argv, int seed)
{
    Obj_list_type::iterator it;

    fprintf(fo,";Shapelet Tree \n");
    fprintf(fo,"\n");
    fprintf(fo,";random seed:%d, rand()=%d, rand()=%d\n",seed,rand(),rand());
    fprintf(fo,";command = ");
    for(int i=0; i<argc; i++)
        fprintf(fo,"%s ",argv[i]);
    fprintf(fo,"\n");
    fprintf(fo,"\n");

    fprintf(fo,";Local Class:  ");
    for(int i=0; i<num_class; i++)
        fprintf(fo,"%2d ",i);
    fprintf(fo,"\n");

    fprintf(fo,";Real  Class:  ");
    for(int i=0; i<num_class; i++)
        fprintf(fo,"%2d ",Real_Class_Label[i]);
    fprintf(fo,"\n");
    fprintf(fo,"\n");
    fprintf(fo,";Node ID class   obj  pos  len  dist_th \n");

    fprintf(fo,"TreeSize: %d \n",Node_Obj_List.size());
    int real_label;

    Shapelet sh;
    for(unsigned int i=1; i<Node_Obj_List.size(); i++)
    {
        if (Node_Obj_List[i].size()==0)
            continue;

        sh = Final_Sh[i];
        if (Classify_list[i] < 0)   // non-leaf
        {
            fprintf(fo,"NonL %2d    %2s    %3d  %3d  %3d  %7.4f ",i,"--",sh.obj,sh.pos,sh.len,sh.dist_th);

            fprintf(fo,"==> ");
            for (int j=0; j<num_class; j++)     fprintf(fo,"%3d ", sh.c_in[j]);
            fprintf(fo," / ");
            for (int j=0; j<num_class; j++)     fprintf(fo,"%3d ", sh.c_out[j]);
            fprintf(fo,"\n");

        }else
        {   real_label = Real_Class_Label[Classify_list[i]];
            fprintf(fo,"Leaf %2d    %2d    \n",i,real_label);
        }
    }

    if (fo==stdout)
    {   fprintf(fo,"\n");
        return;
    }

    fprintf(fo,"\n");
    fprintf(fo,";Shapelet id   <data>\n");
    for(unsigned int i=1; i<Node_Obj_List.size(); i++)
    {
        if (Node_Obj_List[i].size()==0)
            continue;

        sh = Final_Sh[i];
        if (Classify_list[i] < 0)   // non-leaf
        {
            fprintf(fo,"Shapelet %3d  ", i);
            for (unsigned int j=0; j<sh.ts.size(); j++)
                fprintf(fo,"%9.6f ", sh.ts[j]);
            fprintf(fo,"\n");
        }
    }
    fprintf(fo,"\n\n");
}

void printTime(FILE *fo, double* time_arr)
{
    fprintf(fo,"; time =>  ");
    for(int i=0; i<5; i++)
        fprintf(fo,"%7.3f ",time_arr[i]/CLOCKS_PER_SEC);
    fprintf(fo,"\n");
}

void printCommand(FILE *fo, int argc, char** argv)
{
    fprintf(fo,"; cmd =");
    for(int i=0; i<argc; i++)
        fprintf(fo," %s",argv[i]);
    fprintf(fo,"\n");
}


int main(int argc, char* argv[])
{
    int sax_max_len, sax_len, w, card, R, top_k;
    int max_len, min_len, step;
    char* train_file_name;
    double percent_mask;
    Shapelet sh;

    double t0,t1,t2,t3,t4,t5,t6;
    double s1,s2,s3,s4;
    t0 = clock();
    s1=s2=s3=s4=0;

    int seed=time(NULL);
    srand(seed);


    /********************************/
    /**** Fix/Default Parameters ****/
    /********************************/
    card = 4;
    sax_max_len = 15;
    R = 10;
    percent_mask = 0.25;
    top_k = 10;

    /********************************/
    /******* Input Parameters *******/
    /********************************/
    if (argc < 7)    error(0);
    train_file_name = argv[1];
    num_class = atoi(argv[2]);
    num_obj   = atoi(argv[3]);
    max_len   = atoi(argv[4]);
    min_len   = atoi(argv[5]);
    step      = atoi(argv[6]);

    if (argc > 7)  R = atoi(argv[7]);
    if (argc > 8)  top_k = atoi(argv[8]);

    char result_file_name[100] = "tree.txt";
    if (argc > 9)   strcpy(result_file_name, argv[9]);

    char time_file_name[100] = "time.txt";
    if (argc > 10)   strcpy(time_file_name, argv[10]);

    if (max_len < min_len)  error(5);

    int max_tree_depth = EXTRA_TREE_DEPTH + ceil(log(num_class)/log(2))+2;

    printf("\nrand seed=%d\n",seed);
    printf("num_class=%d, num_obj=%d, R=%d, top_k=%d, percent_mask=%.2f, tree_depth=%d    tree_file=%s  time_file=%s\n\n",num_class,num_obj,R,top_k, percent_mask,int(pow(2,max_tree_depth)),result_file_name,time_file_name);

    ReadTrainData(train_file_name);

    /// Find Shapelet
    for (node_id=1; (node_id==1)||(node_id < (int)Node_Obj_List.size()); node_id++)
    {
        Shapelet bsf_sh = Shapelet();
        if (node_id<=1)
            SetCurData(node_id);
        else if(Classify_list[node_id] == -1)   /// non-leaf node (-1:body node, -2:unused node)
            SetCurData(node_id);
        else
            continue;

        for(subseq_len = min_len; subseq_len <= max_len; subseq_len += step)
        {
            /// Shapelet cannot be too short, e.g. len=1.
            if (subseq_len < SH_MIN_LEN)    continue;

            sax_len = sax_max_len;
            /// Make w and sax_len both integer
            w = (int)ceil(1.0*subseq_len/sax_len);
            sax_len = (int)ceil(1.0*subseq_len/w);

            t1 = clock();
            CreateSAXList(subseq_len, sax_len, w);
            t2 = clock();
            s1 += t2-t1;

            RandomProjection(R, percent_mask, sax_len);
            t3 = clock();
            s2 += t3-t2;

            ScoreAllSAX(R);
            t4 = clock();
            s3 += t4-t3;

            //result  = FindBestSAX(top_k);
            sh  = FindBestSAX(top_k);
            t5 = clock();
            s4 += t5-t4;

            if (bsf_sh < sh)
                bsf_sh = sh;

            USAX_Map.clear();
            Score_List.clear();
        }

        if (bsf_sh.len <= 0)
            printf("Error!! bsf_sh is empty  at node_id=%d \n",node_id);
        if (bsf_sh.len > 0)
        {
            vector<double> query(bsf_sh.len);
            for(int i=0; i<bsf_sh.len; i++)
            {   query[i] = Data[bsf_sh.obj][bsf_sh.pos+i];
            }
            bsf_sh.setTS(query);
            Final_Sh.push_back(bsf_sh);
            bsf_sh.printLong();

            /// post-processing: create tree
            SetNextNodeObj(node_id, bsf_sh);
        }
    }
    t6 = clock();

    //printNodeSetList();

#if DEBUG >= 0
        printf("\n");
        printf("num_class=%d, num_obj=%d, R=%d, top_k=%d, percent_mask=%.2f \n",num_class,num_obj,R,top_k, percent_mask);
        printf("  Time Create SAX   = %8.2f sec\n",s1/CLOCKS_PER_SEC);
        printf("  Time Rand Project = %8.2f sec\n",s2/CLOCKS_PER_SEC);
        printf("  Time Score Matrix = %8.2f sec\n",s3/CLOCKS_PER_SEC);
        printf("  Time Compute Gain = %8.2f sec\n",s4/CLOCKS_PER_SEC);
        printf("Running  Time for this Shapelet= %8.2f sec <<\n",(t6-t0)/CLOCKS_PER_SEC);
#endif
    printf("\n\nTotal Running  Time = %8.2f sec <<\n",(t6-t0)/CLOCKS_PER_SEC);


    FILE* f_tree = fopen(result_file_name, "w");        /// tree file
    FILE* f_time = fopen(time_file_name, "a");          /// time file

    printTree(f_tree, argc, argv,seed);

    double time_arr[] = {t6-t0, s1, s2, s3, s4};
    printCommand(f_time, argc, argv);
    printCommand(f_tree, argc, argv);
    printTime(f_time, time_arr);
    printTime(f_tree, time_arr);

    fclose(f_tree);
    fclose(f_time);
    return 0;
}


