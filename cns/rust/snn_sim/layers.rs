
use common::get_global_id;
use std::fmt;

use std::rand;
use std::rand::distributions::{IndependentSample, Range};


#[deriving(Show)]
struct Synapse {
    id_pre: uint,
    x: f32,
    w: f32,
}

impl Synapse {
    fn new( id_pre : uint ) -> Synapse {
        Synapse { id_pre : id_pre, x : 0.0, w : 0.0 }
    }
}

#[deriving(Show)]
struct Neuron {
    id : uint,
    y : f32,
    syns: Vec<Synapse>,
    qr : f32
}

#[deriving(Show)]
struct AdaptNeuron {
    base: Neuron,
    qa: f32,
}

impl AdaptNeuron {
    pub fn new( id : uint ) -> AdaptNeuron {
        return AdaptNeuron { base: SimNeuron::new(id), qa : 0.0 };
    }
}

trait SimNeuron for Sized? {
    fn new( id : uint ) -> Self;
    fn add_connection(&mut self, syn: Synapse);
    fn print_str(&self) -> String;
}


impl SimNeuron for Neuron {
    fn add_connection(&mut self, syn: Synapse) {
        self.syns.push(syn);
    }
    fn new( id : uint ) -> Neuron {
        return Neuron { id : id, y : 0.0, syns : vec!{}, qr : 0.0 };
    }
    fn print_str(&self) -> String {
        format!("Neuron ( id: {}, y: {}, qr: {} )", self.id, self.y, self.qr)
    }
}


pub trait SimLayer {
    fn new( id: uint, size: uint ) -> Self;
    fn print_str(&self) -> String;
    fn connect_to(&self, dst: &mut Self, prob: f32);
}

pub struct Layer {
    id : uint,
    neurons: Vec<Neuron>
}

impl SimLayer for Layer {
    fn new( id: uint, size: uint ) -> Layer {
        let mut l = Layer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n: Neuron = SimNeuron::new(get_global_id());
            l.neurons.push(n);
        }
        return l;
    }
    fn print_str(&self) -> String {
        let mut s : String = format!("\nLayer of {} neurons:\n\t", self.neurons.len()).to_string();
        for i in range(0u, self.neurons.len()) {
            s = s + self.neurons[i].print_str();
            if (i+1) % 5 == 0 && i > 0 { // 5 per string
                s = s + "\n\t";
            } else {
                s = s + ", ";
            }
        }                
        return s;
    }
    fn connect_to(&self, dst: &mut Layer, prob: f32) {
        let runif = Range::new(0f32, 1f32);
        let mut rng = rand::task_rng();

        for i in range(0u, self.neurons.len()) {
            for j in range(0u, dst.neurons.len()) {
                let coin = runif.ind_sample(&mut rng);
                if coin >= prob {
                    dst.neurons[j].add_connection(Synapse::new(self.neurons[i].id));
                }
            }
        }
    }
}

impl<'a> fmt::Show for Box<SimLayer + 'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.print_str())
    }
}

impl<'a> fmt::Show for Box<SimNeuron + 'a> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.print_str())
    }
}

impl fmt::Show for Layer {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.print_str())
    }
}

