
#[deriving(Show)]
struct TestStruct {
    val: Vec<Box<int>>,
}


fn main() {
    let mut a = TestStruct { val : vec!{} };
    a.val.push( box 5i );
    println!("{}", a);
}
