use crate::vis::VisVersion;
use sdk_resources::gmod::GmodDto;
use std::{str::FromStr, collections::HashMap};

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
