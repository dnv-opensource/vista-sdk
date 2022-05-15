use sdk_resources::GmodDto;
use std::str::FromStr;
use crate::vis::VisVersion;

pub struct Gmod {
    pub version: VisVersion,
}

impl Gmod {
    pub(crate) fn new(dto: &GmodDto) -> Gmod {
        Gmod {
            version: VisVersion::from_str(&dto.vis_release).expect("Should always be valid"),
        }
    }
}
