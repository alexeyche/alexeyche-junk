
extern crate getopts;
use std::os;

use layer::Layer;
mod layer;
mod args;


fn main() {
    let l = Layer::new(10);        
    let a = os::args();
    let args = args::parseArgs(a);
}
