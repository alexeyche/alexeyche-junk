options(prompt="R> ")
.First <- function()
{
 if( Sys.getenv("R_CUSTOM") != "" )
 {
  library(utils)
  library(base)
  library(graphics)
  for ( filename in strsplit(Sys.getenv("R_CUSTOM")," ")[[1]] )   {
   source( filename )
  }
 }
}  
