use std::borrow::Cow;
use std::{io::Read};

use std::{collections::HashMap};
use serde::{Deserialize, Serialize};

use flate2::read::GzDecoder;
use rust_embed::RustEmbed;

use std::ops::Deref;

/// A container for values that can only be deref'd immutably.
pub struct Immutable<T> {
    value: T,
}

impl<T> Immutable<T> {
    pub fn new(value: T) -> Self {
        Immutable { value }
    }
}

impl<T> Deref for Immutable<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}


#[derive(RustEmbed)]
#[folder = "../../resources"]
struct Asset;

#[derive(Debug)]
pub enum LoadResourceError {
    ResourceNotFound,
    ReadError(std::io::Error),
    DeserializationError(serde_json::Error),
}

impl std::fmt::Display for LoadResourceError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            LoadResourceError::ResourceNotFound => write!(f, "resource not found"),
            LoadResourceError::ReadError(e) => e.fmt(f),
            LoadResourceError::DeserializationError(e) => e.fmt(f),
        }
    }
}

pub type Result<T> = std::result::Result<T, LoadResourceError>;

pub fn get_gmod_dto(vis_version: &str) -> Result<Immutable<GmodDto>> {
    let gmod_file_name = match Asset::iter().find(|f| f.contains("gmod") && f.contains(vis_version)) {
        Some(f) => f,
        None => return Err(LoadResourceError::ResourceNotFound),
    };

    deserialize_gmod_dto(gmod_file_name)
}

fn deserialize_gmod_dto(gmod_file_name: Cow<str>) -> Result<Immutable<GmodDto>> {
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

    match gmod_json {
        Ok(v) => Ok(Immutable::new(v)),
        Err(e) => Err(LoadResourceError::DeserializationError(e)),
    }
}

pub fn get_vis_versions() -> Box<[String]> {
    Asset::iter().filter_map(|f| match f.contains("gmod") {
        true => Some(deserialize_gmod_dto(f).unwrap().vis_release.to_string()),
        false => None,
    }).collect::<Vec<String>>().into_boxed_slice()
}

pub fn get_files() -> impl Iterator<Item = String> {
    Asset::iter().map(|v| v.to_string())
}
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


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn has_embedded_resources() {
        assert!(Asset::iter().count() > 0);
        for file in Asset::iter() {
            assert!(Asset::get(file.as_ref()).is_some());
            println!("{}", file);
        }
    }
    
    #[test]
    fn can_get_vis_versions() {
        let versions = get_vis_versions();

        assert!(versions.len() > 0);
        assert!(versions.contains(&"3-4a".to_string()));
    }
    
    #[test]
    fn get_gmod_works() {
        let expected_version = "3-4a";
        let gmod_dto = get_gmod_dto(expected_version);
        assert!(gmod_dto.is_ok());
    
        let version = &gmod_dto.unwrap().vis_release;
        assert_eq!(expected_version, version);
    }
}
