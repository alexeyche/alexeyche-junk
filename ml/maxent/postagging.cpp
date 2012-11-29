#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include "maxent.h"

using namespace std;

struct Token
{
  string str;
  string pos;
  Token(const string & s, const string & p) : str(s), pos(p) {}
};

ME_Sample sample(const vector<Token> & vt, int i)
{
  ME_Sample sample;

  sample.label = vt[i].pos;

  const string & w0 = vt[i].str;
  const string wp1 = i > 0 ? vt[i - 1].str : "BOS";
  const string wp2 = i > 1 ? vt[i - 2].str : "BOS";
  const string wm1 = i < (int)vt.size() - 1 ? vt[i + 1].str : "EOS";
  const string wm2 = i < (int)vt.size() - 2 ? vt[i + 2].str : "EOS";

  sample.add_feature("W0_"  + w0);
  sample.add_feature("W-1_" + wm1);
  sample.add_feature("W+1_" + wp1);
  sample.add_feature("W-2_" + wm2);
  sample.add_feature("W+2_" + wp2);

  sample.add_feature("W-10_"  + wm1 + "_" + w0);
  sample.add_feature("W0+1_"  + w0  + "_" + wp1);
  sample.add_feature("W-1+1_" + wm1 + "_" + wp1);

  char buf[1000];
  for (unsigned int j = 1; j <= 10; j++) {
    if (w0.size() >= j) {
      sprintf(buf, "SUF_%s", w0.substr(w0.size() - j).c_str());
      sample.add_feature(buf);
    }
    if (w0.size() >= j) {
      sprintf(buf, "PRE_%s", w0.substr(0, j).c_str());
      sample.add_feature(buf);
    }
  }
  
  return sample;
}

vector<Token> read_line(const string & line) 
{
  vector<Token> vs;
  istringstream is(line);
  string w;
  while (is >> w) {
    string::size_type d = w.find_last_of('/');
    vs.push_back(Token(w.substr(0, d), w.substr(d + 1)));
  }
  return vs;
}

void train(ME_Model & model, const string & filename)
{
  ifstream ifile(filename.c_str());
  
  if (!ifile) {
    cerr << "error: cannot open " << filename << endl; 
    exit(1); 
  }

  string line;
  int n = 0;
  while (getline(ifile, line)) {
    vector<Token> vs = read_line(line);
    for (int j = 0; j < (int)vs.size(); j++) {
      ME_Sample mes = sample(vs, j);
      model.add_training_sample(mes);
    }
    if (n++ > 10000) break;
  }    

  model.use_l1_regularizer(1.0);
//    model.use_l2_regularizer(1.0);
  //  model.use_SGD();
  model.set_heldout(100);
  model.train();
  model.save_to_file("model");
}

void test(const ME_Model & model, const string & filename) 
{
  ifstream ifile(filename.c_str());
  
  if (!ifile) {
    cerr << "error: cannot open " << filename << endl; 
    exit(1); 
  }

  int num_correct = 0;
  int num_tokens = 0;
  string line;
  while (getline(ifile, line)) {
    vector<Token> vs = read_line(line);
    for (int j = 0; j < (int)vs.size(); j++) {
      ME_Sample mes = sample(vs, j);
      model.classify(mes);
      if (mes.label == vs[j].pos) num_correct++;
      num_tokens++;
    }
  }    
  cout << "accuracy = " << num_correct << " / " << num_tokens << " = " 
       << (double)num_correct / num_tokens << endl;
}

int main()
{
  ME_Model m;

  train(m, "./sample_data/train.pos");
  test(m,  "./sample_data/dev.pos");
}
