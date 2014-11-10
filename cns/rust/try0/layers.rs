
use common::get_global_id;
use spike::Spike;

use std::fmt;
use std::rand;
use std::rand::distributions::{IndependentSample, Range};
//use std::sync::mpsc_queue::Queue;



#[deriving(Show)]
struct Synapse {
    id_pre: uint,
    x: f32,
    w: f32,
//    input_q: Queue<Spike>,
//    net_q: Queue<Spike>,
}

impl Synapse {
    fn new( id_pre : uint ) -> Synapse {
        Synapse { 
            id_pre : id_pre, 
            x : 0.0, 
            w : 0.0, 
//            input_q : Queue<Spike>::new(), 
//            net_q : Queue<Spike>::new() 
        }
    }
}

#[deriving(Show)]
struct BasicNeuron {
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
        return AdaptNeuron { base: Neuron::new(id), qa : 0.0 };
    }
}

trait Neuron {
    fn new( id : uint ) -> Self;
    fn add_connection(&mut self, syn: Synapse);
    fn print_str(&self) -> String;
}


impl Neuron for BasicNeuron {
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


trait Layer {
    fn new( id: uint, size: uint ) -> Self;
    fn add_connection(&mut self, dst: &Layer, prob: f32);
}

pub struct BasicLayer<'a, T:'a> {
    id : uint,
    neurons: Vec<&'a T>
}

impl<'a, T: Neuron+fmt::Show> Layer for BasicLayer<'a, T> {
    fn new( id: uint, size: uint ) -> Layer<'a> {
        let mut l = Layer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n: T = Neuron::new(get_global_id());
            l.neurons.push(n);
        }
        return l;
    }
//    fn print_str(&self) -> String {
//        let mut s : String = format!("\nLayer of {} neurons:\n\t", self.neurons.len()).to_string();
//        for i in range(0u, self.neurons.len()) {
//            s = s + self.neurons[i].print_str();
//            if (i+1) % 5 == 0 && i > 0 { // 5 per string
//                s = s + "\n\t";
//            } else {
//                s = s + ", ";
//            }
//        }                
//        return s;
//    }
    fn add_connection(&mut self, dst: &Layer, prob: f32) {
//        let runif = Range::new(0f32, 1f32);
//        let mut rng = rand::task_rng();
//
//        for i in range(0u, self.neurons.len()) {
//            for j in range(0u, dst.neurons.len()) {
//                let coin = runif.ind_sample(&mut rng);
//                if coin >= prob {
//                    dst.neurons[j].add_connection(Synapse::new(self.neurons[i].id));
//                }
//            }
//        }
    }
}

//impl<'a> fmt::Show for Box<SimLayer + 'a> {
//    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//        write!(f, "{}", self.print_str())
//    }
//}
//
//impl<'a> fmt::Show for Box<SimNeuron + 'a> {
//    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//        write!(f, "{}", self.print_str())
//    }
//}
//
//impl fmt::Show for Layer {
//    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//        write!(f, "{}", self.print_str())
//    }
//}
//
//impl fmt::Show for Queue<Spike> {
//    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//        write!(f, "[spike queue]")
//    }    
//}

