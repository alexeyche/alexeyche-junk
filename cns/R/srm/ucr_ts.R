

read_ts_file <- function(ts_name, sample_size, dir=dir) {
  process_datamatrix <- function(m) {
    l = ncol(m)
    out = list()
    for(ri in 1:nrow(m)) {
      out[[ri]] = list(data = m[ri,2:l], label = m[ri,1])    
    }  
    return(out)
  }
  if(!is.na(sample_size)) {
    ts_file_train = sprintf("%s/ts/%s/%s_TRAIN_%s",dir, ts_name, ts_name, sample_size)
    ts_file_test = sprintf("%s/ts/%s/%s_TEST_%s",dir, ts_name, ts_name, sample_size)
  } else {
      ts_file_train = sprintf("%s/ts/%s/%s_TRAIN",dir, ts_name, ts_name)
      ts_file_test = sprintf("%s/ts/%s/%s_TEST",dir, ts_name, ts_name)
  }
  nlines_train = as.numeric(system(sprintf("grep -c ^ %s", ts_file_train), intern=TRUE))
  nlines_test = as.numeric(system(sprintf("grep -c ^ %s", ts_file_test), intern=TRUE))
  ts_train = scan(ts_file_train)
  ts_test = scan(ts_file_test)
  
  ts_train = matrix(ts_train, nrow=nlines_train, byrow=TRUE)
  ts_test = matrix(ts_test, nrow=nlines_test, byrow=TRUE)
  return( list(process_datamatrix(ts_train), process_datamatrix(ts_test)) )
}

ucr_test <- function(train, test, Classification_Algorithm, verbose=TRUE) {
  conf_m = list()
  l = length(train)
  lt = length(test)
  correct = 0
  for(i in 1:length(test)) {    
    predicted_class = Classification_Algorithm(train, test[[i]]$data)
    if(predicted_class == test[[i]]$label) {
      correct = correct +1
    }
    conf_m[[i]] = list(pred=predicted_class, true=test[[i]]$label)
  }
  rate = (lt-correct)/lt
  if(verbose)
      cat("The error rate is ", rate, "\n")
  return(list(rate=rate, prob_tc=conf_m))
}

eucl_dist_alg <- function(train, unknown_object) {
  best_so_far = Inf
  class = -1
  for(i in 1:length(train)) {
    compare_to_this = train[[i]]$data
    dist = sqrt(sum( (compare_to_this-unknown_object)^2) )
    if(dist < best_so_far) {
      class = train[[i]]$label
      best_so_far = dist
    }
  }
  return(class)
}

cross_entropy_alg <- function(train, unknown_object) {
  best_so_far = Inf
  class = -1
  for(i in 1:length(train)) {
    dist = sum( (1-kernelCrossEntropy(train[[i]], list(data=unknown_object, label="0"), kernel_options)$data) )/length(train[[i]])
    if(dist < best_so_far) {
      class = train[[i]]$label
      best_so_far = dist
    }
  }
  return(class)

}

cross_corr_alg <- function(train, unknown_object) {
  best_so_far = Inf
  class = -1
  for(i in 1:length(train)) {
    dist = -sum( (kernelCrossCorr(train[[i]], list(data=unknown_object, label="0"), kernel_options)$data)^2 )/length(train[[i]])
    if(dist < best_so_far) {
      class = train[[i]]$label
      best_so_far = dist
    }
  }
  return(class)

}

van_rossum_alg <- function(train, unknown_object) {
  best_so_far = Inf
  class = -1
  for(i in 1:length(train)) {
    dist = sum( (kernelVanRossumDist(train[[i]], list(data=unknown_object, label="0"), kernel_options)$data)^2 )
    if(dist < best_so_far) {
      class = train[[i]]$label
      best_so_far = dist
    }
  }
  return(class)

}

test = function() {

  name = synth
  c(train, test) := read_ts_file(name)

  ucr_test(train, test, eucl_dist_alg)
}

synth = "synthetic_control"
ecg = "ECG200"
face = "FaceAll"
starlight_curves = "StarLightCurves"
