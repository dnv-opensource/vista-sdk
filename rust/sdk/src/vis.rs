use std::sync::Arc;
use std::time::Duration;

use moka::sync::Cache;
use once_cell::sync::Lazy;

use crate::gmod::Gmod;
use sdk_resources::gmod::get_gmod_dto;

include!(concat!(env!("OUT_DIR"), "/vis.g.rs"));

static INSTANCE: Lazy<Box<Vis>> = Lazy::new(|| {
    Box::new(Vis {
        gmod_cache: Cache::builder()
            .max_capacity(2)
            .time_to_live(Duration::from_secs(60))
            .build(),
    })
});

pub struct Vis {
    gmod_cache: Cache<VisVersion, Arc<Gmod>>,
}

#[derive(Debug, Clone)]
pub enum VisError {
    FailedToLoad(String),
}

impl std::fmt::Display for VisError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            VisError::FailedToLoad(e) => write!(f, "{}", e),
        }
    }
}

pub type Result<T> = std::result::Result<T, VisError>;

impl Vis {
    pub fn instance() -> &'static Vis {
        &INSTANCE
    }

    pub fn get_gmod(&self, version: VisVersion) -> Arc<Gmod> {
        self.gmod_cache.get_with(version, move || {
            let dto = get_gmod_dto(version.to_string().as_str())
                .map_err(|e| VisError::FailedToLoad(e.to_string()))
                .expect("We should always end up having a valid gmod mapped to the binary");

            Arc::new(Gmod::new(&dto))
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn can_init() {
        _ = Vis::instance()
    }

    #[test]
    fn can_get_gmod() {
        let instance = Vis::instance();
        let version = VisVersion::v3_4a;
        let gmod = instance.get_gmod(version);
        assert_eq!(gmod.version(), version);
    }

    #[test]
    fn vis_versions() {
        assert_eq!(VisVersion::v3_4a.to_string(), "3-4a");
        assert_eq!(VisVersion::v3_4a, "3-4a".parse().expect("Should be parseable"));
    }
}
