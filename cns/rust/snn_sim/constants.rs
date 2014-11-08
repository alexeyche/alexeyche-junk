
use serialize::{json, Decodable, Decoder};
use std::io::File;

// Net Layers constants
#[deriving(Encodable, Decodable, Show)]
struct IaFLayerC {
    tau_refr : f32,
    amp_refr : f32,
    u_rest : f32
}

#[deriving(Show)]
struct NetLayersC {
    iaf_layer: IaFLayerC 
}

// Input Layers constants
#[deriving(Encodable, Decodable, Show)]
struct SigmaTCLayerC {
    sigma: f32
}

#[deriving(Show)]
struct InputLayersC {
    sigma_tc_layer: SigmaTCLayerC
}
// synapses constants

// Input Layers constants
#[deriving(Encodable, Decodable, Show)]
struct SynapseC {
    epsp_decay: f32,
    amp: f32,
}

#[deriving(Encodable, Decodable, Show)]
struct InhSynapseC {
    epsp_decay: f32,
    amp: f32,
}

#[deriving(Show)]
struct SynapsesC {
    synapse: SynapseC,
    inh_synapse: InhSynapseC,
    stochastic: bool,
}


#[deriving(Show)]
pub struct Constants {
    net_layers: NetLayersC,
    input_layers: InputLayersC,
    synapses: SynapsesC,
}


impl<S: Decoder<E>, E> Decodable<S, E> for Constants {
    fn decode(decoder: &mut S) -> Result<Constants, E> {
        decoder.read_struct("root", 0, |decoder| {
            Ok(Constants { 
                net_layers : try!(decoder.read_struct_field("net_layers", 0, |decoder| {
                    Ok(NetLayersC {
                        iaf_layer : try!(decoder.read_struct_field("IaFLayer", 0, |decoder| Decodable::decode(decoder)))
                    })
                })),
                input_layers : try!(decoder.read_struct_field("input_layers", 0, |decoder| {
                    Ok(InputLayersC {
                        sigma_tc_layer : try!(decoder.read_struct_field("SigmaTCLayer", 0, |decoder| Decodable::decode(decoder)))
                    })
                })),    
                synapses : try!(decoder.read_struct_field("synapses", 0, |decoder| {
                    Ok(SynapsesC {
                        synapse : try!(decoder.read_struct_field("Synapse", 0, |decoder| Decodable::decode(decoder))),
                        inh_synapse : try!(decoder.read_struct_field("InhSynapse", 0, |decoder| Decodable::decode(decoder))),
                        stochastic : try!(decoder.read_struct_field("stochastic", 0, |decoder| Decodable::decode(decoder))),
                    })
                }))    
            })            
        })
    }        
}


pub fn parse_const_file(constant_filename: String) -> Constants {
    let path = Path::new(constant_filename);
    let display = path.display();
    let mut cfile = match File::open(&path) {
        Err(why) => panic!("couldn't open {}: {}", display, why.desc),
        Ok(file) => file,
    };
    let const_str = match cfile.read_to_string() {
        Err(why) => panic!("couldn't read {}: {}", display, why.desc),
        Ok(string) => string,
    };
    let c: Constants = json::decode(const_str.as_slice()).unwrap();
    return c;
}

