
extern mod extra;
use std::vec;

fn partial_sum(start: uint) -> f64 {
    let mut local_sum = 0f64;
    for num in range(start*100000, (start+1)*100000) {
        local_sum += (num as f64 + 1.0).pow(&-2.0);
    }
    local_sum
}


fn main() {
    //let on_the_stack : Point  =  Point { x: 3.0, y: 4.0 };
    //let managed_box  : @Point = @Point { x: 5.0, y: 1.0 };
    //let owned_box    : ~Point = ~Point { x: 7.0, y: 9.0 };
    let mut futures = vec::from_fn(1000, |ind| do extra::future::Future::spawn { partial_sum(ind) });
    let mut final_res = 0f64;
    for ft in futures.mut_iter()  {
        final_res += ft.get();
    }
    println(fmt!("π^2/6 is not far from : %?", final_res));    
}

