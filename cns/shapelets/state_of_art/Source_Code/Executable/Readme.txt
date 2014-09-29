== How to run the code ==

1. Create the shapelet tree from train data by using command "FastShapelet.exe". The shapelet tree will be written in tree.txt (by default).
2. Using the tree to classify the test data by using command "Classify.txt".

For example on Gun/Point dataset, which has 2 classes, 50 train data, 150 test data, minimum length of all time series is 150. 
We can use this command to run:
C:\> FastShapelet.exe Gun_Point_Train 2 50 150 10 1

Then test the accuracy with this command:
C:\> Classify.exe Gun_Point_Test 2 150


== More Detail ==
Note that you can type command "FastShapelet.exe" or "Classify.exe" for help.

Command "FastShapelet.exe" receives 5 necessary parameters which are
  number_of_classes   number_of_train_time_series   maximum_lengh_of_the_final_shapelet    minimum_length_of_the_shapelet   step_for_search
and also 4 optional parameters which are
  R   K   tree_file   time_file
  
-- Necessary parameters --
	number_of_classes:  number of classes in the dataset. Can be any integer more than 1.
	number_of_train_time_series: number of train data
	maximum_lengh_of_the_final_shapelet:  shaplet length is at most the lenght of the shortest input time series
	minimum_length_of_the_shapelet: usually we set to 10.
	step_for_search: If you allow to skip some shapelet length, you can set this number larger than 1. If set to 10, mean we may search shapelet of length 10, 20, 30, and so on. 
	
-- Opional Parameters --
  R: number_of_random_iteration (default 10)
  K: size_of_candidates  (default 10)
  tree_file:  file for keeping the shapelet tree (default tree.txt)
  time_file:  file for keeping the time (default time.txt)