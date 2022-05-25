
#[derive(Debug)]
pub enum LoadResourceError {
    ResourceNotFound,
    ReadError(std::io::Error),
    DeserializationError(serde_json::Error),
}

impl std::fmt::Display for LoadResourceError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            LoadResourceError::ResourceNotFound => write!(f, "resource not found"),
            LoadResourceError::ReadError(e) => e.fmt(f),
            LoadResourceError::DeserializationError(e) => e.fmt(f),
        }
    }
}

pub type Result<T> = std::result::Result<T, LoadResourceError>;
