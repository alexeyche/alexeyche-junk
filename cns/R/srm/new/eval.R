
source('serialize_to_bin.R')
M = 50
N = 10
run_options = list(T0 = 0, Tmax = 150, dt = 0.5, learning_rate = 0.1, learn_window_size = 15, mode="learn", collect_stat=TRUE)
dir = "/home/alexeyche/prog/sim/R"
wfile = sprintf("%s/%dx%d_lr%3.1f_lwz_%3.1f", dir, M, N, run_options$learning_rate, run_options$learn_window_size)
W = loadMatrix(wfile, 1)
