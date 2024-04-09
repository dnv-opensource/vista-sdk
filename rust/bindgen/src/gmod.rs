use std::{fmt::Debug, sync::Arc};

use uniffi::deps::log::error;
use vista_sdk::{gmod::TraversalHandlerResult, vis::VisVersion};

use crate::{internal::common::Exposable, Gmod, GmodNode};

impl Gmod {
    pub fn version(&self) -> VisVersion {
        self.inner.version()
    }

    pub fn root_node(&self) -> Arc<GmodNode> {
        self.inner.root_node().to_threadsafe_exposed()
    }

    pub fn get_node(&self, code: &str) -> Arc<GmodNode> {
        self.inner.get_node(code).to_threadsafe_exposed()
    }

    pub fn try_get_node(&self, code: &str) -> Option<Arc<GmodNode>> {
        match self.inner.try_get_node(code) {
            Some(inner_node) => Some(inner_node.to_threadsafe_exposed()),
            None => None,
        }
    }

    pub fn get_parents(&self, node: &GmodNode) -> Vec<Arc<GmodNode>> {
        self.inner
            .get_parents(&node.inner)
            .map(|node| node.to_threadsafe_exposed())
            .collect()
    }

    pub fn traverse(&self, callback: Box<dyn TraversalCallback>) -> bool {
        self.inner.traverse(|_parents, _node| {
            callback
                .handler(
                    _parents.map(|node| node.to_threadsafe_exposed()).collect(),
                    _node.to_threadsafe_exposed(),
                )
                .unwrap()
        })
    }
}

pub(crate) trait TraversalCallback: Send + Sync + Debug {
    fn handler(
        &self,
        parents: Vec<Arc<GmodNode>>,
        node: Arc<GmodNode>,
    ) -> Result<TraversalHandlerResult, TraversalError>;
}

#[derive(Debug, thiserror::Error)]
pub(crate) enum TraversalError {
    #[error("Failed")]
    Failed,
}
