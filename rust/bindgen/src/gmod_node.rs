use core::fmt;
use std::sync::Arc;

use crate::internal::common::Exposable;
use crate::GmodNode;

impl GmodNode {
    pub fn code(&self) -> String {
        self.inner.code().to_owned()
    }

    pub fn location(&self) -> String {
        self.inner.location().to_owned()
    }

    pub fn with_location(&self, location: String) -> Arc<GmodNode> {
        self.inner.with_location(location).to_threadsafe_exposed()
    }
}

impl fmt::Display for GmodNode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self)
    }
}

impl Exposable<GmodNode> for vista_sdk::gmod::GmodNode {
    fn to_exposed(&self) -> GmodNode {
        GmodNode { inner: self.to_owned() }
    }
    fn to_threadsafe_exposed(&self) -> Arc<GmodNode> {
        Arc::from(self.to_exposed())
    }
}
