require(Rsnn)
require(rpart)

spikes_bin = spikes_bin[1:10,]
n_names = paste0("n",1:M)

spikes_bin = t(spikes_bin)
colnames(spikes_bin) <- n_names

dur = timeline[2]-timeline[1]
cl = NULL
for(patt_i in 1:length(timeline)) {
    cl = c(cl, rep(labels[patt_i], dur))
}

sp_df = data.frame(cbind(spikes_bin, cl))

f = paste("cl ~ ", paste(n_names, collapse= "+"))
fit <- rpart(formula(f), method="class", data=sp_df)