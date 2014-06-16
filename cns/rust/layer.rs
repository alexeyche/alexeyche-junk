
struct Neuron {
    id : uint,
    x : Vec<f32>,
    y : f32,
    fired : bool,
    syn_fired : Vec<bool>,
}

impl Neuron {
    fn new() -> Neuron {
        Neuron { x : vec!{}, y : 0.0, fired : false, syn_fired : vec!{}, id : 0 }
    }
}

pub struct Layer {
    neurons : Vec<Neuron>,
}

impl Layer {
    pub fn new(N: uint) -> Layer {
       let mut l = Layer { neurons : vec!{} };
       for i in range(0u, N) {
           l.neurons.push( Neuron::new() );
       }
       return(l);
    }
}


