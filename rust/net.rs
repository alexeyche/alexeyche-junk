#!/usr/local/bin/rustc

use std::task::spawn;
use std::comm::{stream,Port,Chan,SharedChan};

struct Neuron {
    u : f32,
    input : Port<int>,
    output : Chan<int>,
    output_port : Port<int>,
}

impl Neuron {
    fn new(port_in : Port<int>) -> Neuron {
        let (output_port, output_chan): (Port<int>, Chan<int>) = stream();
        let n = Neuron { u           : -70.0f32, 
                         input       : port_in, 
                         output      : output_chan, 
                         output_port : output_port 
                        };
        do spawn || {
            n.wait_for_input();
        }            
        return n;
    }
    fn wait_for_input(&self) -> () {
        println(fmt!("waiting for input"));
        let t_in = self.input.recv();
        println(fmt!("got input %?",t_in));
    }
}

fn main() {
    //let chan = SharedChan::new(chan);
    let (port, chan): (Port<int>, Chan<int>) = stream();
    let nr = Neuron::new(port); // get spikes from port
    println(fmt!("Neuron instance: %?", nr)); 
    chan.send(700);

}

