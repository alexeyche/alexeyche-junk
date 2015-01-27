
source('ucr_ts.R')
setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")
source('../../scripts/eval_dist_matrix.R')

ts_dir = '~/prog/sim/ts'

sample_size = 60

data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, sample_size, ts_dir)

data = list()
data$distance_matrix = vector("list", length(train_dataset))

for(ti in 1:length(train_dataset)) {
    for(tj in 1:length(train_dataset)) {
        data$distance_matrix[[ti]] = cbind(data$distance_matrix[[ti]], sum((train_dataset[[ti]]$data - train_dataset[[tj]]$data)^2)/1000)
    }
}
data$rates = rep(target_rate, 100)
data$labels = sapply(train_dataset, function(x) x$label)
calculate_criterion(data)