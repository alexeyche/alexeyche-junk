use std::rand;
use std::rand::distributions::{IndependentSample, Range};

static mut GLOBAL_ID: uint = 0;

pub fn getGlobalId() -> uint {
    let mut glob_id;
    unsafe {
        glob_id = GLOBAL_ID;
        GLOBAL_ID += 1;
    }
    return glob_id;
}


pub trait NetNeuron {
    fn new(id: uint) -> Self;
    fn add_connection(&mut self, id : uint, w : f32) -> ();
    fn getId(&self) -> uint;
}

pub trait NetLayer {
    fn new(size: uint, id : uint) -> Self;
    fn getNeurons(&mut self) -> &mut Vec<Box<NetNeuron>>;
    
    fn connect_feed_forward(&mut self, l: &mut NetLayer, conn_prob: f32) {
        let runif = Range::new(0f32, 1f32);
        let mut rng = rand::task_rng();
        for pre in self.getNeurons().iter() {
            for post in l.getNeurons().mut_iter() {
                let coin = runif.ind_sample(&mut rng);
                if coin >= conn_prob {
                    println!("{} >= {}",  coin, conn_prob);
                    post.add_connection(pre.getId(), 0.0);
                }
            }
        }            
    }

}


