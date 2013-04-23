
require(RSQLite)

con <- dbConnect(dbDriver("SQLite"), "/home/alexeyche/ml.db")

load_matrix <- function(name) {
    colMax <- dbGetQuery(con,paste("SELECT max(colNum) as v FROM matrices WHERE name='",name,"'",sep=""))$v
    rowMax <- dbGetQuery(con,paste("SELECT max(rowNum) as v FROM matrices WHERE name='",name,"'",sep=""))$v   
    
    W <- matrix(0, nrow=rowMax+1, ncol=colMax+1)
    if (colMax<=rowMax) {
        W_df <- dbGetQuery(con,paste("SELECT colNum, rowNum, value FROM matrices WHERE name='",name,"' order by rowNum",sep=""))
        for(i in seq(0, colMax)) {
            i_shift <- i*rowMax
            W[,i+1] <- W_df$value[(i_shift+1):(i_shift+rowMax+1)]
            
        }
    } else {
        W_df <- dbGetQuery(con,paste("SELECT colNum, rowNum, value FROM matrices WHERE name='",W,"' order by colNum",sep=""))
        for(i in seq(0, rowMax)) {
            W[i+1,] <- W_df$value[(i_shift+1):(i_shift+rowMax+1)]            
        }
    }
    return(W)
}
