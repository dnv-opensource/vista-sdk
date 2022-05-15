#![allow(dead_code)]

use std::env;
use std::fs;
use std::path::Path;

use sdk_resources;
use serde::Deserialize;
use serde::Serialize;

#[derive(Serialize, Deserialize, Debug)]
struct VersionInfo
{
    enum_version: String,
    display_version: String,
}

fn main() {
    println!("cargo:warning=Running build script...");

    let _files = sdk_resources::get_files();

    let all_verisons = sdk_resources::get_vis_versions();
    let versions: Vec<_> = all_verisons
        .iter()
        .map(|v| VersionInfo { enum_version: v.replace("-", "_"), display_version: v.to_string() })
        .collect();

    let out_dir = env::var_os("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("vis.g.rs");

    let template_text = include_str!("vis.g.rs.liquid");

    let template = liquid::ParserBuilder::with_stdlib()
        .build().unwrap()
        .parse(template_text).unwrap();

    let globals = liquid::object!({
        "vis_versions": versions,
    });


    let code = template.render(&globals).unwrap();
    println!("cargo:warning=Code:\n{}\n", code);

    fs::write(
        &dest_path,
        code
    ).unwrap();

    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=vis.g.rs.liquid");
    println!("cargo:rerun-if-changed=../../resources");
}
