#![allow(non_upper_case_globals)] // Due to RustEmbed below

use rust_embed::RustEmbed;

#[derive(RustEmbed)]
#[folder = "../../resources"]
pub struct Asset;
