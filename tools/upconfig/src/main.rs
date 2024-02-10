use std::{
  collections::HashSet,
  fs,
  hash::Hash,
  io::{self},
};

use serde::Deserialize;
use toml;

#[derive(Debug, Deserialize, PartialEq)]
enum BinType {
  Library,
  Program,
}

#[derive(Deserialize)]
struct BinConfig {
  #[serde(rename = "type")]
  bin_type: BinType,
  bin: Option<String>,
  dep: Option<Vec<String>>,
}

struct Bin {
  name: String,
  path: String,
  bin_type: BinType,
  dep: Vec<String>,
}

impl Hash for Bin {
  fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
    self.name.hash(state);
  }
}

impl PartialEq for Bin {
  fn eq(&self, other: &Self) -> bool {
    self.name == other.name
  }
}

impl Eq for Bin {}

impl From<(&str, BinConfig)> for Bin {
  fn from((name, config): (&str, BinConfig)) -> Self {
    Self {
      name: config.bin.unwrap_or(name.into()),
      path: name.into(),
      dep: config.dep.unwrap_or(Vec::new()),
      bin_type: config.bin_type,
    }
  }
}

impl Bin {
  pub fn with_bin(bin: String) -> Self {
    Self {
      name: bin,
      dep: vec![],
      bin_type: BinType::Program,
      path: "".into(),
    }
  }
}

fn main() -> io::Result<()> {
  let mut bins = HashSet::<Bin>::new();
  for entry in
    fs::read_dir("userspace/").expect("Could not read entries in 'userspace/'")
  {
    let entry = entry.expect("Error while reading entries in 'userspace/'");
    let path = {
      let mut path = entry.path();
      path.push("config.toml");
      path
    };

    let config = match fs::read_to_string(path) {
      Err(_) => continue,
      Ok(content) => content,
    };
    let config: BinConfig = toml::from_str(&config).expect(&format!(
      "Could not parse config file for binary {}",
      entry.file_name().to_string_lossy()
    ));
    let bin: Bin =
      (&*entry.file_name().to_string_lossy().to_string(), config).into();
    if bins.contains(&bin) {
      let original = bins.get(&bin).unwrap();
      panic!(
        "Binary name '{}' is duplicated. First found in '{}', then in '{}'",
        bin.name, original.path, bin.path
      );
    }
    bins.insert(bin);
  }

  // Check dependencies
  for bin in bins.iter() {
    for dep in bin.dep.iter() {
      match bins.get(&Bin::with_bin(dep.clone())) {
        None => panic!(
          "{:?} '{}@{}' depends on library '{}' which could not be found.",
          bin.bin_type, bin.name, bin.path, dep
        ),
        Some(b) if b.bin_type == BinType::Program => panic!(
          "{:?} '{}@{}' depends on '{}@{}' which is a program not a library.",
          bin.bin_type, bin.name, bin.path, b.name, b.path
        ),
        Some(_) if bin.bin_type == BinType::Library => {
          panic!("Libraries cannot currently have dependencies.")
        }
        _ => {}
      };
    }
  }

  // Generate makefile
  let lib_template = fs::read_to_string("buildsys/template/upconfig_lib.mk")
    .expect("Library make template not found!");
  let prog_template = fs::read_to_string("buildsys/template/upconfig_prog.mk")
    .expect("Program make template not found!");

  fs::create_dir_all("build/buildsys/")
    .expect("Could not create directory 'build/buildsys/'");

  // Remove existing makefiles
  for entry in fs::read_dir("build/buildsys/")
    .expect("Could not read entries in 'build/buildsys'")
  {
    let entry = entry.expect("Could not read entry in 'build/buildsys'");
    if !entry
      .file_type()
      .expect("Could not get entry type")
      .is_file()
    {
      continue;
    }
    fs::remove_file(entry.path()).expect(&format!(
      "Could not remove file '{}'",
      entry.file_name().to_string_lossy()
    ));
  }
  for bin in bins.iter() {
    eprintln!("Generating makefile for {}", bin.name);
    let template = match bin.bin_type {
      BinType::Library => &lib_template,
      BinType::Program => &prog_template,
    };

    let dependency_names = bin.dep.join(" ");
    let dependency_paths = bin
      .dep
      .iter()
      .filter_map(|dep| bins.get(&Bin::with_bin(dep.clone())))
      .map(|dep| dep.path.clone())
      .collect::<Vec<_>>()
      .join(" ");

    let output = template
      .replace("<path>", &bin.path)
      .replace("<name>", &bin.name)
      .replace("<depname>", &dependency_names)
      .replace("<deppath>", &dependency_paths);

    fs::write(format!("build/buildsys/upconfig_{}.mk", bin.name), output)
      .expect(&format!(
        "Could not write make file for '{}@{}'",
        bin.name, bin.path
      ));
  }

  Ok(())
}
