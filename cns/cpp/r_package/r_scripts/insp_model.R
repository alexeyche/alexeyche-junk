require(Rdnn)

setwd("~/cpp/build")

m = RProto$new("model.pb")$read()