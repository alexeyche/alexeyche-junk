
#[deriving(Show)]
struct Neuron {
    id : uint,
    x : Vec<f32>,
    y : f32,
}

impl Neuron {
    fn new(id: uint) -> Neuron {
        Neuron { x : vec!{}, y : 0.0, id : id }
    }
}

#[deriving(Show)]
pub struct Layer {
    neurons : Vec<Neuron>,
}

impl Layer {
    pub fn new(N: uint, glob_id: &mut uint) -> Layer {
       let mut l = Layer { neurons : vec!{} };
       for i in range(0u, N) {
           l.neurons.push( Neuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
    }
}


