use std::str::FromStr;

use vista_sdk::vis::VisVersion;

use crate::{VisVersionExtensions, VisVersions};

impl VisVersionExtensions {
    pub fn new() -> Self {
        VisVersionExtensions {}
    }

    pub fn to_version_string(&self, version: VisVersion) -> String {
        format!("{}", version)
    }
}

impl VisVersions {
    pub fn new() -> Self {
        VisVersions {}
    }

    pub fn parse(&self, input: &str) -> VisVersion {
        VisVersion::from_str(input).unwrap()
    }
    pub fn all(&self) -> Vec<VisVersion> {
        VisVersion::ALL.to_vec()
    }
}
