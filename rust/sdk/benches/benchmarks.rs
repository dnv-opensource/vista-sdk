use std::time::Duration;

use criterion::{criterion_group, criterion_main, Criterion};
use vista_sdk::{gmod::TraversalHandlerResult, vis::Vis, vis::VisVersion};

use pprof::criterion::{Output, PProfProfiler};

pub fn criterion_benchmark(c: &mut Criterion) {
    let instance = Vis::instance();
    let gmod = instance.get_gmod(VisVersion::v3_4a);

    c.bench_function("gmod full traversal", |b| {
        b.iter(|| {
            gmod.traverse(|_parents, _node| TraversalHandlerResult::Continue);
        })
    });
}

criterion_group! {
    name = benches;
    config = Criterion::default()
        .measurement_time(Duration::from_secs(50))
        .sample_size(500)
        .with_profiler(PProfProfiler::new(100, Output::Protobuf));
    targets = criterion_benchmark
}
criterion_main!(benches);
