use asset::Asset;
use gmod::deserialize_gmod_dto;
use once_cell::sync::Lazy;

pub(crate) mod asset;
pub mod gmod;
pub(crate) mod result;

static FILES: Lazy<Vec<String>> = Lazy::new(|| Asset::iter().map(|v| v.to_string()).collect());

static VIS_VERSIONS: Lazy<Vec<String>> = Lazy::new(|| {
    Asset::iter()
        .filter_map(
            |f| match f.starts_with("gmod") && !f.starts_with("gmod-vis-versioning") {
                true => Some(deserialize_gmod_dto(f).unwrap().vis_release),
                false => None,
            },
        )
        .collect()
});

pub fn get_vis_versions() -> &'static [String] {
    VIS_VERSIONS.as_slice()
}

pub fn get_files() -> &'static [String] {
    FILES.as_slice()
}

#[cfg(test)]
mod tests {
    use crate::gmod::get_gmod_dto;

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

        assert!(!versions.is_empty());
        assert!(versions.iter().map(AsRef::as_ref).any(|v: &str| v == "3-4a"));
    }

    #[test]
    fn get_gmod_works() {
        let expected_version = "3-4a";
        let gmod_dto = get_gmod_dto(expected_version);
        assert!(gmod_dto.is_ok());

        let gmod_dto = gmod_dto.unwrap();
        let version = gmod_dto.vis_release;
        assert_eq!(expected_version, version);
    }
}
