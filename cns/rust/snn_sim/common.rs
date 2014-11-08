
static mut GLOBAL_ID: uint = 0;

pub fn get_global_id() -> uint {
    let mut glob_id;
    unsafe {
        glob_id = GLOBAL_ID;
        GLOBAL_ID += 1;
    }
    return glob_id;
}
