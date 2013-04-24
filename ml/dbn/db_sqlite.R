
require(RSQLite)

con <- dbConnect(dbDriver("SQLite"), "/home/alexeyche/ml.db")

load_matrices <- function(names) {
    df_all <- dbGetQuery(con,"SELECT name, colNum, rowNum, value FROM matrices")
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
