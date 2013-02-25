#!/usr/bin/RScript

data <- read.csv('training_feat.csv',sep=',',header=FALSE)
data <- as.matrix(data)
n <- ncol(data)
sd_d <- apply(data,2,sd)
sd_treshold <- 0.015  # near half of data would filtered
m_tresh <- 7000

# sd filter
final_data <- matrix(data[,1], nrow=nrow(data))  # it is answer column
for(i in 2:n) {
    if(sd_d[i] > sd_treshold) {
        final_data <- cbind(final_data, data[,i])
    }
}

# count round:
final_data <- final_data[1:m_tresh,]

write.table(final_data, "training_feat_proc.csv", sep=',', row.names=FALSE, col.names=FALSE)

