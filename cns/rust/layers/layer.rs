
#[deriving(Show)]
struct Neuron {
    id : uint,
    x : Vec<f32>,
    y : f32,
    id_conns : Vec<uint>,
    w : Vec<uint>,
}

impl Neuron {
    fn new(id: uint) -> Neuron {
        Neuron { x : vec!{}, y : 0.0, id : id, id_conns : vec!{}, w : vec!{} }
    }
}

#[deriving(Show)]
pub struct Layer {
    id : uint, 
    neurons : Vec<Neuron>,
}

impl Layer {
    pub fn new(size: uint, id : uint, glob_id: &mut uint) -> Layer {
       let mut l = Layer { neurons : vec!{}, id : id };
       for i in range(0u, size) {
           l.neurons.push( Neuron::new(*glob_id) );
           *glob_id+=1;
       }
       return l;
    }
}


