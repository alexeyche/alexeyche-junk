

require(Rdnn)
setwd("/home/alexeyche/dnn/runs/sim/test_run")
mi0 = accum_from_epochs("OptimalStdp_mi_stat",1:60,"%s_stat.pb")
plot(mi0, col="black", type="l", ylim=c(0, 4e-03))

setwd("/home/alexeyche/dnn/runs/sim/test_run_wd_null")
mi2 = accum_from_epochs("OptimalStdp_mi_stat",1:60,"%s_stat.pb")
lines(mi2, col="red", type="l")

setwd("/home/alexeyche/dnn/runs/sim/test_run_no_vals")
mi3 = accum_from_epochs("OptimalStdp_mi_stat",1:60,"%s_stat.pb")
lines(mi3, col="blue", type="l")

