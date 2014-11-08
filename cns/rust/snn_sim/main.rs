#![feature(macro_rules)]

extern crate getopts;
extern crate serialize;

use std::os;
use args::parse_args;
use constants::Constants;
use constants::parse_const_file;
use sim::Sim;

mod args;
mod constants;
mod sim;
mod layers;
mod common;

macro_rules! trysome(
    ($e:expr) => (
        match $e {
            Some(v) => v,
            None => panic!("Failed operation"),
        };
    );
)


fn main() {
    let a = os::args();
    let args = trysome!(parse_args(a));

    println!("{}", args.to_string());
    let c: Constants = parse_const_file(args.constant_filename);
    println!("{}", c);

    let s = Sim::new();
    println!("{}", s);
}
