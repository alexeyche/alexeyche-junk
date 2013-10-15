#!/usr/local/bin/rustc

struct Neuron {
    u : f32
}
impl Neuron {
    pub fn new() -> Neuron {
        Neuron { u : -70.0f32 }
    }
}

fn main() {
    let n = Neuron::new();
    println(fmt!("Neuron instance: %?", n)); 
}

