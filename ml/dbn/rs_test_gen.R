#!/usr/bin/RScript

i <- 1

n <- 50
m <- 1000

d <- 20

values <- NULL
for(m_i in seq(1,m)) {
    row <- array(0, dim=c(1,n))
    row[1,i] <- d
    if ((i == 24)|(i == 28)) {
        i <- i +1
    }
    for(n_i in seq(1,n)) {
        
        val  <- d/(1+abs(n_i - i))
        if (val<1) { val <- 0 }
        row[1,n_i] <- round(val)
    }
    values <- rbind(values, row)
    i<-i+1
    if(i>n) {
        i <- 1
    }
}
d <- 30
i <- 1
for(m_i in seq(1,50)) {
    row <- array(0, dim=c(1,n))
    row[1,i] <- d
    if ((i == 3)|(i == 9)) {
        i <- i +1
        }
    for(n_i in seq(3,n)) {
        
        val  <- d/(1+abs(n_i - i))
        if (val<1) { val <- 0 }
        row[1,n_i] <- round(val)
    }
    values <- rbind(values, row)
    i<-i+1
    if(i>n) {
        i <- 1
    }
}

write.table(values,"test_data_rs.csv", sep=",", row.names=FALSE, col.names=FALSE)
