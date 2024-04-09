use std::sync::Arc;

pub(crate) trait Exposable<T> {
    fn to_exposed(&self) -> T;
    fn to_threadsafe_exposed(&self) -> Arc<T>;
}
