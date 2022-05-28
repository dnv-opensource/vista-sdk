use crate::vis::VisVersion;
use sdk_resources::gmod::GmodDto;
use std::{
    collections::{HashMap},
    str::FromStr,
};

pub enum GmodRelation {
    Parent,
}

pub struct Gmod {
    pub version: VisVersion,
    index: HashMap<String, u32>,
    children: Vec<Vec<u32>>,
    parents: Vec<Vec<u32>>,
    nodes: Vec<GmodNode>,
}

#[derive(Debug, Clone)]
pub struct GmodNode {
    pub code: String,
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
            });

            children.push(Vec::with_capacity(1));
            parents.push(Vec::with_capacity(1));
        }

        for [parent, child] in &dto.relations {
            let parent = index.get(parent).unwrap();
            let child = index.get(child).unwrap();

            let children = &mut children[(*parent) as usize];
            let parents = &mut parents[(*child) as usize];

            children.push((*child) as u32);
            parents.push((*parent) as u32);
        }

        Gmod {
            version: VisVersion::from_str(&dto.vis_release).expect("Should always be valid"),
            index,
            children,
            parents,
            nodes,
        }
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
        let index = self.index.get(&node.code).unwrap();
        self.children[(*index) as usize]
            .iter()
            .map(|child| &self.nodes[(*child) as usize])
    }

    fn traverse_node<TState, THandler>(
        &self,
        context: &mut TraversalContext<TState, THandler>,
        index: u32,
    ) -> TraversalHandlerResult
    where
        THandler: FnMut(&mut TState, &dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        if context.has(index) {
            return TraversalHandlerResult::Continue;
        }

        let parents = context.parents.iter().map(|i| &self.nodes[(*i) as usize]);
        let node = &self.nodes[index as usize];
        let result = (context.handler)(&mut context.state, &parents, node);
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
        let index = self.index.get("VE").unwrap();
        self.traverse_from(&self.nodes[(*index) as usize], handler)
    }

    pub fn traverse_from<THandler>(&self, from_node: &GmodNode, mut handler: THandler) -> bool
    where
        THandler: FnMut(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let handler = 
            |_state: &mut (), parents: &dyn Iterator<Item = &GmodNode>, node: &GmodNode| (handler)(parents, node);
        self.traverse_with_from(&mut (), from_node, handler)
    }

    pub fn traverse_with<TState, THandler>(&self, state: &mut TState, handler: THandler) -> bool
    where
        THandler: FnMut(&mut TState, &dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let index = self.index.get("VE").unwrap();
        self.traverse_with_from(state, &self.nodes[(*index) as usize], handler)
    }

    pub fn traverse_with_from<TState, THandler>(&self, state: &mut TState, from_node: &GmodNode, handler: THandler) -> bool
    where
        THandler: FnMut(&mut TState, &dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let index = self.index.get(&from_node.code).unwrap();
        let mut context = TraversalContext::<TState, THandler> {
            ids: VecSet::new(16),
            parents: Vec::with_capacity(16),
            handler: handler,
            state: state,
        };

        let result = self.traverse_node(&mut context, *index);
        result == TraversalHandlerResult::Continue
    }
}

struct TraversalContext<'a, TState, THandler>
where
    THandler: FnMut(&mut TState, &dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
{
    ids: VecSet,
    pub parents: Vec<u32>,
    pub handler: THandler,
    pub state: &'a mut TState,
}

impl<'a, TState, THandler> TraversalContext<'a, TState, THandler>
where
    THandler: FnMut(&mut TState, &dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
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

        let mut count = 0;
        let reached_end = gmod.traverse(|_parents, _node| {
            count += 1;

            TraversalHandlerResult::Continue
        });

        assert!(reached_end);
        assert!(count > 0, "Count should increase after traversal");
    }

    #[test]
    fn traverse_from() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);

        let root = gmod.root_node();

        let mut count = 0;
        let reached_end = gmod.traverse_from(root, |_parents, _node| {
            count += 1;

            TraversalHandlerResult::Continue
        });

        assert!(reached_end);
        assert!(count > 0, "Count should increase after traversal");
    }

    #[test]
    fn traverse_with() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);

        let mut count = 0;
        let reached_end = gmod.traverse_with(&mut count, |count, _parents, _node| {
            (*count) += 1;

            TraversalHandlerResult::Continue
        });

        assert!(reached_end);
        assert!(count > 0, "Count should increase after traversal");
    }

    #[test]
    fn traverse_with_from() {
        let instance = Vis::instance();
        let gmod = instance.get_gmod(VisVersion::v3_4a);

        let root = gmod.root_node();

        let mut count = 0;
        let reached_end = gmod.traverse_with_from(&mut count, root, |count, _parents, _node| {
            (*count) += 1;

            TraversalHandlerResult::Continue
        });

        assert!(reached_end);
        assert!(count > 0, "Count should increase after traversal");
    }
}
