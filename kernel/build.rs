fn main() {
  let assemblies = ["src/proc/syscall.asm", "src/interrupts/lapic/timer.asm"];
  for assembly in assemblies {
    println!("cargo::rerun-if-changed={assembly}");
  }
  nasm_rs::compile_library_args("libhelium-asm.a", &assemblies, &["-felf64"])
    .unwrap();
  println!("cargo::rustc-link-lib=helium-asm")
}
