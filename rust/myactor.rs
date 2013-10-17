
use std::comm::{stream,Port,SharedChan};
// messages to control an Actor
enum ActorMessage{
    Execute,
    Stop,
}
struct BaseActor{
    // stores SharedChans from other Actors in a vector
    receiver: ~[SharedChan<ActorMessage>],

    // the port for receiving an ActorMessage.
    port: Port<ActorMessage>,
    // a SharedChan to pass it to other Actors.
    chan: SharedChan<ActorMessage>,
}

trait Actor{

    // getters that have to implemented on the client 
    fn get_port<'r>(&'r self) -> &'r Port<ActorMessage>;
    fn get_chan<'r>(&'r self) -> &'r SharedChan<ActorMessage>;
    fn get_receiver<'r>(&'r mut self) -> &'r mut ~[SharedChan<ActorMessage>];

    // adds a chan in the actor, so that the Actor can communicate with other Actors.
    fn add_receiver(&mut self,a: &SharedChan<ActorMessage>){
        self.get_receiver().push(a.clone());
    }
    //starts the actor, it will listen to incoming messages.
    fn start(&self){
        println("Starting Actor");
        loop{
            if(self.get_port().peek()){
                self.listen_for_messages();
                self.on_receive();
            }  
        }
    }
    // client controlled execute fn.
    fn execute(&self);
    // will be called when a message is received.
    fn on_receive(&self); 
    // match the ActorMessages
    fn listen_for_messages(&self) {
        println("processing message");
        let m = self.get_port().recv();
        match m {
            Execute => self.execute(),
            // doesn't work atm, can't make this fn mutable because
            // I would also need to make .start mutable which makes problems in closures
            // see fn create actor. Any fixes or workarounds?
            Stop => fail!("Task was stopped by callee.")
        }
    }
}

impl Actor for BaseActor {
    fn get_port<'r>(&'r self) -> &'r Port<ActorMessage>{
        &'r(self.port)
    }   
    fn get_receiver<'r>(&'r mut self) -> &'r mut ~[SharedChan<ActorMessage>]{
        &'r mut (self.receiver)
    }
    fn get_chan<'r>(&'r self) -> &'r SharedChan<ActorMessage>{
        &'r(self.chan)
    }
    fn on_receive(&self){
        println("Received an ActorMessage")
    }
    fn execute(&self){
        println("Actor says: \"Hello\"");
    }
}

impl BaseActor {
    fn new() -> (BaseActor,SharedChan<ActorMessage>, SharedChan<ActorMessage>) {
        println("Creating Actor");
        let (port, chan) = stream::<ActorMessage>();
        let chan1 = SharedChan::new(chan);
        let chan2 = SharedChan::new(chan);
        let actor = BaseActor { receiver: ~[chan2.clone()], chan: chan1.clone(), port: port };
        
        (actor,chan, chan2)
    }
}

fn create_actor() -> SharedChan<ActorMessage>{
    let (actor,chan, chan2) = BaseActor::new();
    do std::task::spawn_unlinked{
       actor.start();
    }
    chan
}

fn main() {
// you control Actors with ActorMessages
let chan = create_actor(); 
chan.send(Execute);
chan.send(Stop);

//let chan1 = create_actor(); 
//chan1.send(Execute);
//chan1.send(Stop);
}
