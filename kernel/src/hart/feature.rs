use super::ThisHart;
use crate::{error, sync::Once};
use core::{
  arch::x86_64::{CpuidResult, __cpuid, __cpuid_count},
  fmt::{Debug, Display},
};

static BOOTSTRAP_FEATURESET: Once<FeatureSet> = Once::new();

#[derive(Clone, Copy)]
pub enum FeatureSet {
  Sufficient {
    vendor: SupportedVendor,
    context_id: bool,
    inv_context_id: bool,
    shadow_stack: bool,
    pk_user: bool,
    pk_super: bool,
    noexec: bool,
  },
  Insufficient(InsufficientReason),
}

#[derive(Debug, Clone, Copy)]
pub enum SupportedVendor {
  GenuineIntel,
  AuthenticAMD,
}

#[derive(Clone, Copy)]
pub enum InsufficientReason {
  UnsupportedVendor([u8; 12]),
  UnreachableCpuidLeaf(usize),
  UnreachableCpuidExtendedLeaf(usize),
  NoHugePages,
  NoApic,
  NoSyscall,
}

impl FeatureSet {
  pub fn from_current() -> Self {
    let CpuidResult {
      eax: max_basic,
      ebx: vendor0,
      edx: vendor1,
      ecx: vendor2,
    } = unsafe { __cpuid(0) };
    let max_basic = max_basic as usize;
    let max_extended = unsafe { __cpuid(0x8000_0000) }.eax as usize;

    let vendor = match SupportedVendor::try_from({
      let mut vendor = [0; 12];
      vendor[0..4].copy_from_slice(&vendor0.to_ne_bytes());
      vendor[4..8].copy_from_slice(&vendor1.to_ne_bytes());
      vendor[8..12].copy_from_slice(&vendor2.to_ne_bytes());
      vendor
    }) {
      Ok(vendor) => vendor,
      Err(unsupported) => {
        return FeatureSet::Insufficient(InsufficientReason::UnsupportedVendor(
          unsupported,
        ))
      }
    };

    vendor.feature_detect(vendor, max_basic, max_extended)
  }
}

impl FeatureSet {
  pub fn init_current() {
    if ThisHart::is_bootstrap() {
      BOOTSTRAP_FEATURESET.init(|| Self::from_current());
    } else {
      todo!()
    }
  }

  /// # Panic
  /// Will panic if called before the executing hart has initialized their
  /// FeatureSet.
  pub fn instance() -> &'static FeatureSet {
    if ThisHart::is_bootstrap() {
      BOOTSTRAP_FEATURESET.get().unwrap()
    } else {
      todo!()
    }
  }

  pub fn ensure_enough() {
    FeatureSet::init_current();
    let features = FeatureSet::instance();
    if let FeatureSet::Insufficient(insufficient_reason) = features {
      error!(
        "Processor has insufficient feature support: {}",
        insufficient_reason
      );
      ThisHart::die()
    }
  }
}

impl SupportedVendor {
  fn feature_detect(
    &self,
    vendor: SupportedVendor,
    max_basic: usize,
    max_extended: usize,
  ) -> FeatureSet {
    let detector = match self {
      SupportedVendor::GenuineIntel | SupportedVendor::AuthenticAMD => {
        Self::intel_amd_detect_features
      }
    };
    detector(vendor, max_basic, max_extended)
  }
}

impl SupportedVendor {
  fn intel_amd_detect_features(
    vendor: SupportedVendor,
    max_basic: usize,
    max_extended: usize,
  ) -> FeatureSet {
    const REQ_MAX_BASIC: usize = 1;
    const REQ_MAX_EXT: usize = 0x8000_0001;

    if max_basic < REQ_MAX_BASIC {
      return FeatureSet::Insufficient(
        InsufficientReason::UnreachableCpuidLeaf(REQ_MAX_BASIC),
      );
    }
    if max_extended < REQ_MAX_EXT {
      return FeatureSet::Insufficient(
        InsufficientReason::UnreachableCpuidExtendedLeaf(REQ_MAX_EXT),
      );
    }

    let cpuid1 = unsafe { __cpuid(1) };
    let cpuidext1 = unsafe { __cpuid(0x8000_0001) };

    let has_huge_pages = (cpuidext1.edx >> 26) & 1 == 1;
    let has_apic = (cpuid1.edx >> 9) & 1 == 1;
    let has_syscall = (cpuidext1.edx >> 11) & 1 == 1;

    if !has_huge_pages {
      return FeatureSet::Insufficient(InsufficientReason::NoHugePages);
    }
    if !has_apic {
      return FeatureSet::Insufficient(InsufficientReason::NoApic);
    }
    if !has_syscall {
      return FeatureSet::Insufficient(InsufficientReason::NoSyscall);
    }

    let noexec = (cpuidext1.edx >> 20) & 1 == 1;

    // I couldn't find in AMD manual where PCID functionality is
    // However they do mention in APM Volume 2 Section 5.5.1 page 158 (March 2024)
    // that it can be found with CPUID 01.ECX[PCID], without mentioning the bit
    // However APM Volume 3 Appendix E pages 602-603 (March 2024) do not mention any PCID bit
    // in ECX for CPUID 01. Assuming they are just like Intel where
    // PCID is CPUID 01.ECX[bit 17], even though it is marked as reserved in the
    // Aforementioned APM Volume 3
    let context_id = (cpuid1.ecx >> 17) & 1 == 1;

    // Yet strangely, AMD talks about INVPCID without ambiguity...
    let (inv_context_id, shadow_stack, pk_user, pk_super) = if max_basic >= 7 {
      let cpuid7_0 = unsafe { __cpuid_count(7, 0) };
      let inv_context_id = (cpuid7_0.ebx >> 10) & 1 == 1;
      let shadow_stack = (cpuid7_0.ecx >> 7) & 1 == 1;
      let pk_user = (cpuid7_0.ecx >> 3) & 1 == 1;
      let pk_super = match vendor {
        SupportedVendor::GenuineIntel => (cpuid7_0.ecx >> 31) & 1 == 1,
        SupportedVendor::AuthenticAMD => false, // I couldn't find any mention of PKS, and bit 31 is marked as reserved
      };
      (inv_context_id, shadow_stack, pk_user, pk_super)
    } else {
      (false, false, false, false)
    };

    FeatureSet::Sufficient {
      vendor,
      context_id,
      inv_context_id,
      shadow_stack,
      pk_user,
      pk_super,
      noexec,
    }
  }
}

impl TryFrom<[u8; 12]> for SupportedVendor {
  type Error = [u8; 12];

  fn try_from(value: [u8; 12]) -> Result<Self, Self::Error> {
    match &value {
      b"GenuineIntel" => Ok(Self::GenuineIntel),
      b"AuthenticAMD" => Ok(Self::AuthenticAMD),
      _ => Err(value),
    }
  }
}

impl Display for InsufficientReason {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    match self {
      InsufficientReason::UnsupportedVendor(vendor) => {
        match vendor.as_ascii() {
          Some(vendor) => write!(f, "Unsupported Vendor: {}", vendor.as_str()),
          None => write!(f, "Unsupported Vendor: {vendor:?}"),
        }
      }
      InsufficientReason::UnreachableCpuidLeaf(leaf) => {
        write!(f, "Does not support CPUID leaf {leaf:02x}")
      }
      InsufficientReason::UnreachableCpuidExtendedLeaf(leaf) => {
        write!(f, "Does not support CPUID extended leaf {leaf:08x}")
      }
      InsufficientReason::NoHugePages => {
        write!(f, "Does not support huge pages (1 GiB)")
      }
      InsufficientReason::NoApic => write!(f, "Does not support APIC"),
      InsufficientReason::NoSyscall => write!(f, "Does not support syscalls"), // Dinosaure
    }
  }
}

impl Display for SupportedVendor {
  fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
    Debug::fmt(&self, f)
  }
}
