use layers::net_layer::{NetNeuron, NetLayer, getGlobalId};



#[deriving(Show)]
struct InputNeuron {
    id : uint,
}

impl NetNeuron for InputNeuron {
    fn new(id: uint) -> InputNeuron {
        InputNeuron { id : id }
    }
    fn add_connection(&mut self, id : uint, w : f32) -> () {
        println!("InputLayer is not supposed to get connections");
    }
    fn getId(&self) -> uint {
        return self.id;
    }
}

pub struct InputLayer {
    id : uint,
    neurons: Vec<Box<NetNeuron>>,
}

impl NetLayer for InputLayer {
    fn new(size: uint, id : uint) -> InputLayer {
        let mut l = InputLayer { neurons : vec!{}, id : id };
        for i in range(0u, size) {
            let n: InputNeuron = NetNeuron::new(getGlobalId());
            l.neurons.push(box n);
        }
        return l;
    }
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
        return &mut self.neurons;
    }
}


//impl NetLayer for InputLayer {
//    fn new(size: uint, id : uint, glob_id: &mut uint) -> InputLayer {
//       let mut l = InputLayer { neurons : vec!{}, id : id };
//       for i in range(0u, size) {
//           let n: InputNeuron = NetNeuron::new(*glob_id);
//           l.neurons.push(box n);
//           *glob_id+=1;
//       }
//       return l;
//    }
//    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>> {
//        return &mut self.neurons;
//    }
//}


