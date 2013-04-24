
require(RSQLite)

con <- dbConnect(dbDriver("SQLite"), "/home/alexeyche/ml.db")

load_matrices <- function(names, iter) {
    df_all <- dbGetQuery(con,paste("SELECT name, colNum, rowNum, value FROM matrices WHERE iter =",iter))
    for(name in names) {
        f <- df_all[df_all$name == name,2:4]        
        mat <- sparseMatrix(i = f$rowNum,
                        j = f$colNum,
                        x = f$value, index1=FALSE)
        assign(name, mat, envir = .GlobalEnv)
    }          
}

get_names <- function() {
    dbGetQuery(con,"SELECT DISTINCT name FROM matrices")$name
}
get_iters <- function() {
    dbGetQuery(con,"SELECT DISTINCT iter FROM matrices")$iter
}
last_iter <- max(get_iters())
