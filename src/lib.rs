use std::os::raw::{c_char, c_int, c_uint, c_ulong};

use cty::c_float;

#[cxx::bridge(namespace = "mtkahypar")]
mod ffi {
    unsafe extern "C++" {}
}

unsafe extern "C" {
    pub fn mtkahypar_hello();
    pub fn mtkahypar_test_partitioner(filename: *const u8);
    pub fn mtkahypar_partition(
        nvtxs: c_uint,
        nhedges: c_uint,
        hewt: *const c_int,
        vtw: *const c_int,
        eind: *const c_ulong,
        eptr: *const c_ulong,
        part: *mut c_int,
        kway: c_int,
        passes: c_int,
        seed: c_ulong,
        imbalance: c_float,
    );
}

pub fn mtkahypar_test(data: Vec<i32>) {
    println!("Here we are with KaHyPar!");
    for v in &data {
        println!("Data value {}", v);
    }
    unsafe {
        mtkahypar_hello();
    }
}
