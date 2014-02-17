

get_unique_ids <- function(n) {
  if(!exists('ID_MAX')) { assign('ID_MAX', 0, envir=.GlobalEnv) }
  id_max = get('ID_MAX',.GlobalEnv)
  assign('ID_MAX', id_max+n, envir=.GlobalEnv)
  return((id_max+1):(id_max+n))
}
