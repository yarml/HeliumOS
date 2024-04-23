fn main() {
  let assemblies = ["src/start.asm"];
  for assembly in assemblies {
    println!("cargo::rerun-if-changed={assembly}");
  }
  nasm_rs::compile_library_args("libhelium-start.a", &assemblies, &["-felf64"])
    .unwrap();
  println!("cargo::rustc-link-lib=helium-start")
}
