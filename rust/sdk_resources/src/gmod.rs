use std::borrow::Cow;
use std::io::Read;

use flate2::read::GzDecoder;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

use crate::{asset::Asset, result::LoadResourceError, result::Result};

#[derive(Serialize, Deserialize, Debug)]
pub struct GmodDto {
    #[serde(alias = "visRelease")]
    pub vis_release: String,
    #[serde(alias = "items")]
    pub items: Vec<GmodNodeDto>,
    #[serde(alias = "relations")]
    pub relations: Vec<[String; 2]>,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct GmodNodeDto {
    #[serde(alias = "category")]
    pub category: String,
    #[serde(alias = "type")]
    pub node_type: String,
    #[serde(alias = "code")]
    pub code: String,
    #[serde(alias = "name")]
    pub name: Option<String>,
    #[serde(alias = "commonName")]
    pub common_name: Option<String>,
    #[serde(alias = "definition")]
    pub definition: Option<String>,
    #[serde(alias = "commonDefinition")]
    pub common_definition: Option<String>,
    #[serde(alias = "installSubstructure")]
    pub install_substructure: Option<bool>,
    #[serde(alias = "normalAssignmentNames")]
    pub normal_assignment_names: Option<HashMap<String, String>>,
}

pub fn get_gmod_dto(vis_version: &str) -> Result<GmodDto> {
    let gmod_file_name = match Asset::iter().find(|f| f.contains("gmod") && f.contains(vis_version))
    {
        Some(f) => f,
        None => return Err(LoadResourceError::ResourceNotFound),
    };

    deserialize_gmod_dto(gmod_file_name)
}

pub fn deserialize_gmod_dto(gmod_file_name: Cow<str>) -> Result<GmodDto> {
    assert!(gmod_file_name.contains("gmod"));

    let gmod_file = match Asset::get(gmod_file_name.as_ref()) {
        Some(f) => f,
        None => return Err(LoadResourceError::ResourceNotFound),
    };

    let mut decoder = GzDecoder::new(gmod_file.data.as_ref());
    let mut gmod_str = String::with_capacity(1024 * 8);
    let gmod_json = match decoder.read_to_string(&mut gmod_str) {
        Ok(_) => serde_json::from_slice::<GmodDto>(gmod_str.as_bytes()),
        Err(e) => return Err(LoadResourceError::ReadError(e)),
    };

    let mut gmod = match gmod_json {
        Ok(v) => v,
        Err(e) => return Err(LoadResourceError::DeserializationError(e)),
    };

    const EXCLUDE_PATTERN: &'static str = "99";
    gmod.items
        .retain(|node| !node.code.ends_with(EXCLUDE_PATTERN));
    gmod.relations.retain(|[parent, child]| {
        !parent.ends_with(EXCLUDE_PATTERN) && !child.ends_with(EXCLUDE_PATTERN)
    });

    Ok(gmod)
}
