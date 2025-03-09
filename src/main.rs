use mtkahypar_r;

fn main() {
    println!("MT-KaHyPar wrapper");
    unsafe {
        mtkahypar_r::mtkahypar_hello();
        mtkahypar_r::mtkahypar_test_partitioner("benches/small.hgr".as_ptr());
    }
}
