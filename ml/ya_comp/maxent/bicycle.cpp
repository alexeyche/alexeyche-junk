#include <string>
#include <list>
#include <cstdio>
#include "maxent.h"

using namespace std;

void train_the_model_with_samples(ME_Model & model)
{
  ME_Sample s1("CAR");
  s1.add_feature("four wheels");              // binary feature
  s1.add_feature("blue");                     // binary feature
  s1.add_feature("length", 4.0);              // real-valued feature
  // This real-valued feature is almost equivalent to the following:
  //   s1.add_feature("length");
  //   s1.add_feature("length");
  //   s1.add_feature("length");
  //   s1.add_feature("length");

  ME_Sample s2("CAR");
  s2.add_feature("four wheels");
  s2.add_feature("red");
  s2.add_feature("length", 5.0);

  ME_Sample s3("BICYCLE");
  s3.add_feature("two wheels");
  s3.add_feature("red");
  s3.add_feature("length", 1.7);
//
//  ME_Sample s4("BICYCLE");
//  s4.add_feature("two wheels");
//  s4.add_feature("yellow");
//  s4.add_feature("length", 1.9);

  model.add_training_sample(s1);
  model.add_training_sample(s2);
  model.add_training_sample(s3);
//  model.add_training_sample(s4);

//  model.use_l1_regularizer(1.0);
  model.train();
}

int main()
{
  ME_Model model;

  train_the_model_with_samples(model);
  
  // Classifying a new sample
  ME_Sample s;
  s.add_feature("four wheels");
  s.add_feature("blue");
  s.add_feature("length", 1.8);

  model.classify(s);
  cout << endl;
  cout << "it's a " << s.label << " !" << endl << endl;

  // You can get the probability distribution of a classification
  vector<double> vp = model.classify(s);
  for (int i = 0; i < model.num_classes(); i++) {
    cout << vp[i] << "\t" <<  model.get_class_label(i) << endl;
  }
  cout << endl;
  
  // You can save the model into a file.
  model.save_to_file("model");
  // You can, of course, load a model from a file.
  // Try replacing train_the_model_with_samples() with model.load_from_file("model").

  // If you want to see the weights of the features,
  list< pair< pair<string, string>, double > > fl;
  model.get_features(fl);
  for (list< pair< pair<string, string>, double> >::const_iterator i = fl.begin(); i != fl.end(); i++) {
    printf("%10.3f  %-10s %s\n", i->second, i->first.first.c_str(), i->first.second.c_str());
  }
}
