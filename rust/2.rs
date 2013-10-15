
use std::os;

fn main() {
  let args : ~[~str] = os::args();
  for s in args {
    println(s);
  }
}
