use super::APIC_VBASE;

#[repr(C, packed(4096))]
pub struct LocalApicRegisterMap {
  res0: [u32; 4],
  res1: [u32; 4],

  pub idreg: [u32; 4],
  pub verreg: [u32; 4],

  res2: [[u32; 4]; 4],

  pub taskprreg: [u32; 4],
  pub arbprreg: [u32; 4],
  pub procprreg: [u32; 4],
  pub eoireg: [u32; 4],
  pub remoterreg: [u32; 4],
  pub logdestreg: [u32; 4],
  pub destfmtreg: [u32; 4],
  pub sivreg: [u32; 4],

  pub isr: [[u32; 4]; 8],
  pub tmr: [[u32; 4]; 8],
  pub irr: [[u32; 4]; 8],

  pub errstatereg: [u32; 4],

  res3: [[u32; 4]; 6],

  pub lvt_cmcireg: [u32; 4],

  pub icr: [[u32; 4]; 2],

  pub lvt_timerreg: [u32; 4],
  pub lvt_thermalreg: [u32; 4],
  pub lvt_perfmonreg: [u32; 4],
  pub lvt_lint0reg: [u32; 4],
  pub lvt_lint1reg: [u32; 4],
  pub lvt_errreg: [u32; 4],

  pub initcountreg: [u32; 4],
  pub currcountreg: [u32; 4],

  res4: [[u32; 4]; 4],

  pub divcfgreg: [u32; 4],

  res5: [[u32; 4]; 193],
}

impl LocalApicRegisterMap {
  pub fn get<'a>() -> &'a mut LocalApicRegisterMap {
    unsafe { APIC_VBASE.as_mut_ptr::<Self>().as_mut() }.unwrap()
  }

  pub fn timer_setup(&mut self, vector: u8, mode: TimerMode, divpow: usize) {
    self.lvt_timerreg[0] = vector as u32 | (mode as u32) << 17;
    self.divcfgreg[0] = match divpow {
      0 => 0b1011,
      n if n <= 7 => ((n - 1) & 0b100 << 3 | (n - 1) & 0b11) as u32,
      other => panic!("Unsupported timer division power {}.", other),
    }
  }

  pub fn timer_current(&self) -> usize {
    (unsafe { (&self.currcountreg[0] as *const u32).read_volatile() }) as usize
  }

  pub fn timer_reset(&mut self, init: usize) {
    self.initcountreg[0] = init as u32;
  }

  pub fn spurious_setup(&mut self, vector: u8) {
    self.sivreg[0] = vector as u32 | 1 << 8;
  }
  pub fn error_setup(&mut self, vector: u8) {
    self.lvt_errreg[0] = vector as u32;
  }

  pub fn lint_setup(&mut self, lint: usize, flags: u16) {
    let reg = match lint {
      0 => &mut self.lvt_lint0reg[0],
      1 => &mut self.lvt_lint1reg[0],
      other => panic!("Invalid LINT#{}", other),
    };
    *reg = flags as u32;
  }

  pub fn eoi(&mut self) {
    self.eoireg[0] = 0;
  }
}

#[repr(u8)]
#[allow(dead_code)]
pub enum TimerMode {
  OneShot,
  Periodic,
  TSCDeadline,
}
