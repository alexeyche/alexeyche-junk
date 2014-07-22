extern crate regex;
extern crate getopts;
use std::os;

use layer::Layer;
mod layer;
mod args;
mod constants;

fn main() {
    let l = Layer::new(10);        
    let a = os::args();
    let args = match args::parse_args(a) {
        Some(v) => v,
        None => return,
    };
//    println!("{}", args.to_str());
    let c = match constants::parse_constants(args.constantFilename) {
        Some(v) => v,
        None => return,
    };
}
