use crate::vis::VisVersion;
use sdk_resources::gmod::GmodDto;
use std::{str::FromStr, collections::{HashMap, HashSet}};

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

            nodes.push(GmodNode { code: node.code.to_string() });

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
        self.parents[(*index) as usize].iter().map(|parent| &self.nodes[(*parent) as usize])
    }

    pub fn get_children(&self, node: &GmodNode) -> impl Iterator<Item = &GmodNode> {
        let index = self.index.get(&node.code).unwrap();
        self.children[(*index) as usize].iter().map(|child| &self.nodes[(*child) as usize])
    }

    pub fn traverse<F>(&self, f: F)
    where
        F : Fn(&dyn Iterator<Item = &GmodNode>, &GmodNode) -> TraversalHandlerResult,
    {
        let root_node = self.root_node();
        let mut context = TraversalContext {
            ids: HashSet::new(),
            parents: Vec::new(),
        };

        let index = self.index.get(&root_node.code).unwrap();
        let mut stack = Vec::new();
        stack.push(index);

        while !stack.is_empty() {
            let index = stack.pop().unwrap();

            if context.has(index) {
                continue;
            }

            let parents = context.parents.iter().map(|i| &self.nodes[(*i) as usize]);
            let result = f(&parents, &self.nodes[(*index) as usize]);
            
            if result == TraversalHandlerResult::Stop || result == TraversalHandlerResult::SkipSubtree {
                return;
            }
            
            context.push(index);

            for child in &self.children[(*index) as usize] {
                stack.push(child);
            }
        }

        // context.push(index);

    }
}

struct TraversalContext {
    pub ids: HashSet<u32>,
    pub parents: Vec<u32>,
}

impl TraversalContext {
    pub fn has(&self, index: &u32) -> bool {
        self.ids.contains(index)
    }

    pub fn push(&mut self, index: &u32) {
        self.ids.insert(*index);
        self.parents.push(*index);
    }

    pub fn pop(&mut self) {
        let parent = self.parents.pop().unwrap();
        self.ids.remove(&parent);
    }
}

#[cfg(test)]
mod tests {
    use crate::vis::Vis;
    use super::*;

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
}
