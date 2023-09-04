use crate::vis::VisVersion;
use sdk_resources::gmod::GmodDto;
use std::{collections::HashMap, str::FromStr};

pub struct Gmod {
    version: VisVersion,
    index: HashMap<String, u32>,
    children: Vec<Vec<u32>>,
    parents: Vec<Vec<u32>>,
    nodes: Vec<GmodNode>,
}

#[derive(Debug, Clone)]
pub struct GmodNode {
    code: String,
    location: String,
    metadata: GmodNodeMetadata,
}

#[derive(Debug, Clone)]
pub struct GmodNodeMetadata {
    category: String,
    node_type: String,
    name: Option<String>,
    common_name: Option<String>,
    definition: Option<String>,
    common_definition: Option<String>,
    install_substructure: Option<bool>,
    normal_assignment_names: HashMap<String, String>,
}

impl GmodNode {
    pub fn code(&self) -> &str {
        self.code.as_str()
    }

    pub fn location(&self) -> &str {
        self.location.as_str()
    }

    pub fn with_location(&self, location: String) -> GmodNode {
        let mut result = self.clone();
        result.location = location;
        result
    }

    pub fn without_location(&self) -> GmodNode {
        let mut result = self.clone();
        result.location = Default::default();
        result
    }

    pub fn product_type<'a>(&self, gmod: &'a Gmod) -> Option<&'a GmodNode> {
        gmod.get_product_type_for(self)
    }

    pub fn product_selection<'a>(&self, gmod: &'a Gmod) -> Option<&'a GmodNode> {
        gmod.get_product_selection_for(self)
    }

    pub fn is_mappable(&self) -> bool {
        todo!()
    }
}

#[derive(PartialEq)]
pub enum TraversalHandlerResult {
    Stop,
    SkipSubtree,
    Continue,
}

impl Gmod {
    pub(crate) fn new(dto: &GmodDto) -> Gmod {
        let mut index = HashMap::with_capacity(1024 * 4);
        let mut nodes = Vec::with_capacity(1024 * 4);
        let mut children: Vec<Vec<u32>> = Vec::with_capacity(1024 * 4);
        let mut parents: Vec<Vec<u32>> = Vec::with_capacity(1024 * 4);

        for i in 0..dto.items.len() {
            let node = &dto.items[i];

            index.insert(node.code.to_string(), i as u32);

            nodes.push(GmodNode {
                code: node.code.to_string(),
                location: Default::default(),
                metadata: GmodNodeMetadata {
                    category: node.category.to_string(),
                    node_type: node.node_type.to_string(),
                    name: node.name.clone(),
                    common_name: node.common_name.clone(),
                    definition: node.definition.clone(),
                    common_definition: node.common_definition.clone(),
                    install_substructure: node.install_substructure,
                    normal_assignment_names: node.normal_assignment_names.clone().unwrap_or_default(),
                },
            });

            children.push(Vec::with_capacity(1));
            parents.push(Vec::with_capacity(1));
        }

        for [parent, child] in &dto.relations {
            let parent = index.get(parent).unwrap();
            let child = index.get(child).unwrap();

            let children = &mut children[(*parent) as usize];
            let parents = &mut parents[(*child) as usize];

            children.push(*child);
            parents.push(*parent);
        }

        Gmod {
            version: VisVersion::from_str(&dto.vis_release).expect("Should always be valid"),
            index,
            children,
            parents,
            nodes,
        }
    }

    pub fn version(&self) -> VisVersion {
        self.version
    }

    pub fn root_node(&self) -> &GmodNode {
        let index = self.index.get("VE").unwrap();
        &self.nodes[(*index) as usize]
    }

    pub fn get_parents(&self, node: &GmodNode) -> impl Iterator<Item = &GmodNode> {
        let index = self.index.get(&node.code).unwrap();
        self.parents[(*index) as usize]
            .iter()
            .map(|parent| &self.nodes[(*parent) as usize])
    }

    pub fn get_children(&self, node: &GmodNode) -> impl Iterator<Item = &GmodNode> {
        let index = self.get_index(node);
        self.children[index].iter().map(|child| &self.nodes[(*child) as usize])
    }

    pub fn get_product_type_for(&self, node: &GmodNode) -> Option<&GmodNode> {
        let index = self.index[&node.code];
        let children = &self.children[index as usize];
        if children.len() != 1 {
            return None;
        }
        if !node.metadata.category.contains("FUNCTION") {
            return None;
        }

        let child = &self.nodes[children[0] as usize];
        if child.metadata.category != "PRODUCT" || child.metadata.node_type != "TYPE" {
            return None;
        }

        Some(child)
    }

    pub fn get_product_selection_for(&self, node: &GmodNode) -> Option<&GmodNode> {
        let index = self.index[&node.code];
        let children = &self.children[index as usize];
        if children.len() != 1 {
            return None;
        }
        if !node.metadata.category.contains("FUNCTION") {
            return None;
        }

        let child = &self.nodes[children[0] as usize];
        if !child.metadata.category.contains("PRODUCT") || child.metadata.node_type != "SELECTION" {
            return None;
        }

        Some(child)
    }

    fn get_index(&self, node: &GmodNode) -> usize {
        self.index[&node.code] as usize
    }

    fn traverse_node<THandler>(&self, context: &mut TraversalContext<THandler>, index: u32) -> TraversalHandlerResult
    where
        THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        if context.has(index) {
            return TraversalHandlerResult::Continue;
        }

        let parents = context.parents.iter().map(|i| &self.nodes[(*i) as usize]);
        let node = &self.nodes[index as usize];
        let result = (context.handler)(&parents, node);
        if result == TraversalHandlerResult::Stop || result == TraversalHandlerResult::SkipSubtree {
            return result;
        }

        context.push(index);

        let children = &self.children[index as usize];
        for (_, child) in children.iter().enumerate() {
            let result = self.traverse_node(context, *child);
            if result == TraversalHandlerResult::Stop {
                return result;
            } else if result == TraversalHandlerResult::SkipSubtree {
                continue;
            }
        }

        context.pop();
        TraversalHandlerResult::Continue
    }

    pub fn traverse<THandler>(&self, handler: THandler) -> bool
    where
        THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let index = self.index["VE"];
        self.traverse_from(&self.nodes[index as usize], handler)
    }

    pub fn traverse_from<THandler>(&self, from_node: &GmodNode, handler: THandler) -> bool
    where
        THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let index = self.index[&from_node.code];
        let mut context = TraversalContext {
            ids: VecSet::new(16),
            parents: Vec::with_capacity(16),
            handler,
        };

        let result = self.traverse_node(&mut context, index);
        result == TraversalHandlerResult::Continue
    }
}

struct TraversalContext<THandler>
where
    THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
{
    ids: VecSet,
    pub parents: Vec<u32>,
    pub handler: THandler,
}

impl<THandler> TraversalContext<THandler>
where
    THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
{
    pub fn has(&self, index: u32) -> bool {
        self.ids.contains(index)
    }

    pub fn push(&mut self, index: u32) {
        self.ids.insert(index);
        self.parents.push(index);
    }

    pub fn pop(&mut self) {
        let parent = self.parents.pop().unwrap();
        self.ids.remove(parent);
    }
}

// We just wrap a Vector, sets are slower
// for our usecase due to hashing
// 40%~ speedup
struct VecSet {
    inner: Vec<u32>,
}

impl VecSet {
    pub fn new(cap: usize) -> Self {
        VecSet {
            inner: Vec::with_capacity(cap),
        }
    }

    pub fn insert(&mut self, value: u32) {
        if self.contains(value) {
            return;
        }

        self.inner.push(value);
    }

    pub fn remove(&mut self, value: u32) {
        self.inner.retain(|v| *v != value);
    }

    pub fn contains(&self, value: u32) -> bool {
        self.inner.contains(&value)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::vis::Vis;

    #[test]
    fn ve_is_root() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);
        assert_eq!("VE", gmod.root_node().code);
    }

    #[test]
    fn ve_has_300a() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);
        let root_node = gmod.root_node();

        assert!(gmod.get_children(root_node).any(|n| n.code == "300a"))
    }

    #[test]
    fn ve_has_no_parents() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);
        let root_node = gmod.root_node();

        let parent_count = gmod.get_parents(root_node).count();
        assert_eq!(parent_count, 0);
    }

    #[test]
    fn traverse() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);

        let traverse_count = {
            let mut count = 0;
            let reached_end = gmod.traverse(|_parents, _node| {
                count += 1;

                TraversalHandlerResult::Continue
            });
            assert!(reached_end);
            assert!(count > 0, "Count should increase after traversal");
            println!("Gmod traversal count: {}", count);
            count
        };

        let traverse_from_count = {
            let root = gmod.root_node();

            let mut count = 0;
            let reached_end = gmod.traverse_from(root, |_parents, _node| {
                count += 1;

                TraversalHandlerResult::Continue
            });

            assert!(reached_end);
            assert!(count > 0, "Count should increase after traversal");
            println!("Gmod traversal count: {}", count);
            count
        };

        assert_eq!(traverse_count, traverse_from_count);
    }
}
