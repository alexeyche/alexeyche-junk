
require(Rdnn)
bd="/home/alexeyche/dnn/build"
setwd(bd)

# 
ts = list(
    values=matrix(
        c(sin(0.1*seq(0,1000,length.out=1000))
      , sin(0.1*seq(0,1000,length.out=1000))),
    nrow=2, ncol=1000, byrow=TRUE)
)
f="/home/alexeyche/dnn/ts/test.pb"
RProto$new(f)$write(ts, "TimeSeries")

v = RProto$new("res.pb")$read()$values

vi = RProto$new("res_inv.pb")$read()$values


# tf = seq(0, 100, length.out=100)
# f = exp(-tf/20)

# set.seed(2)
# sp = gen_poisson(1, rate=40, binary=TRUE)[1,]
# # f = c(f, rep(0, length(sp)-length(f)))
# # 
# # f_m_sp = (fft(f) * fft(sp))/length(sp)
# # conv = Re(fft(f_m_sp, inverse=TRUE))
# # plot(conv[1:200], type="l")
# # lines(sp[1:200], type="l", col="red")
# 
# 
# file="/home/alexeyche/dnn/build/test.pb"
# RProto$new(file)$write(list(values=sp), "TimeSeries")
# file="/home/alexeyche/dnn/build/filter.pb"
# RProto$new(file)$write(list(values=f), "TimeSeries")
# 
# file="/home/alexeyche/dnn/build/out.pb"
# out = RProto$new(file)$read()
# 
# file="/home/alexeyche/dnn/build/kernel.pb"
# kernel = RProto$new(file)$read()
# 
# file="/home/alexeyche/dnn/build/gram.pb"
# gram = RProto$new(file)$read()
# 
# file="/home/alexeyche/dnn/build/f.csv"
# 
# file="/home/alexeyche/dnn/build/out.pb"
# v = RProto$new(file)$read()
# for(val in v$values) {
#     vf = fft(v$values[[1]])
#     cat(Sys.time(), "\n")
# }
# 
# v$values = list(v$values[[1]])
# v$ts_info = NULL
# #v$ts_info$labels_ids = as.integer(v$ts_info$labels_ids)
# #v$ts_info$labels_timeline = as.integer(v$ts_info$labels_timeline)
# 
# file2="/home/alexeyche/dnn/build/out2.pb"
# 
# RProto$new(file2)$write(v, "TimeSeries")
# 
# 
# 
