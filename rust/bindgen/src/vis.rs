use std::sync::Arc;

use crate::{internal::common::Exposable, Gmod, Vis};
use vista_sdk::vis::{VisVersion, INSTANCE as VIS_INSTANCE};

impl Vis {
    pub fn instance() -> Vis {
        Vis { inner: &VIS_INSTANCE }
    }

    pub fn get_gmod(&self, version: VisVersion) -> Arc<Gmod> {
        self.inner.get_gmod(version).to_threadsafe_exposed()
    }
}

impl Exposable<Gmod> for Arc<vista_sdk::gmod::Gmod> {
    fn to_exposed(&self) -> Gmod {
        Gmod { inner: self.to_owned() }
    }

    fn to_threadsafe_exposed(&self) -> Arc<Gmod> {
        Arc::from(self.to_exposed())
    }
}
