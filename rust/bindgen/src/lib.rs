use std::{fmt::Debug, sync::Arc};

use uniffi;
use vista_sdk;

// Mods
mod gmod;
mod gmod_node;
mod internal;
mod vis;
mod vis_version;

/* Exposed in UDL */
use crate::gmod::{TraversalCallback, TraversalError};
use crate::vista_sdk::gmod::TraversalHandlerResult;
use crate::vista_sdk::vis::VisVersion;

pub(crate) struct Vis {
    inner: &'static vista_sdk::vis::Vis,
}

pub(crate) struct Gmod {
    inner: Arc<vista_sdk::gmod::Gmod>,
}

#[derive(Debug, Clone)]
pub(crate) struct GmodNode {
    inner: vista_sdk::gmod::GmodNode,
}

pub(crate) struct VisVersionExtensions {}
pub(crate) struct VisVersions {}

/* End of UDL expose */

uniffi::include_scaffolding!("bindings");
