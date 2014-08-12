#![feature(macro_rules)]

extern crate regex;
extern crate getopts;
use std::os;

use layers::layer::Layer;

mod layers {
    pub mod layer;
}
mod args;
mod constants;


fn main() {
    let l = Layer::new(10);        
    let a = os::args();
    let args = match args::parse_args(a) {
        Some(v) => v,
        None => return,
    };
    println!("{}", args.to_string());
    let c = match constants::parse_constants(args.constantFilename) {
        Some(v) => v,
        None => return,
    };
    println!("{}", c.to_string());
}
