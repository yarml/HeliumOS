#![feature(generic_arg_infer)]

fn main() {
  let assemblies: [&str; _] = [];
  for assembly in assemblies {
    println!("cargo::rerun-if-changed={assembly}");
  }
  nasm_rs::compile_library_args("libhelium-asm.a", &assemblies, &["-felf64"])
    .unwrap();
  println!("cargo::rustc-link-lib=helium-asm")
}
