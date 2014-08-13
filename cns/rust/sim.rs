

use layers::layer::Layer;
use constants::Constants;

#[deriving(Show)]
struct GlobContext {
    pub last_global_id: uint,
}

#[deriving(Show)]
pub struct Sim {
    pub globCtx: GlobContext, 
    pub layers: Vec<Layer>,
}



impl Sim {
    pub fn new(c: &Constants) -> Sim {
        let mut s = Sim { layers : vec!{}, globCtx: GlobContext { last_global_id : 0 } };

        let mut inputIDs = vec!{};
        let mut net_size = 0;
        for layer_size in c.in_lc.iter().map(|in_lc| in_lc.size) {
            inputIDs.grow_fn(layer_size, |i| i + net_size); 
            net_size += layer_size;
        }
        s.globCtx.last_global_id = net_size;
        
        for layer_size in c.lc.iter().map(|lc| lc.size) {
            let layer_id = s.layers.len();
            s.layers.push( Layer::new(layer_size, layer_id, &mut s.globCtx.last_global_id) );
        }
        println!("{}", inputIDs);
        return s;
    }
}
