"""Pytest configuration for Vista SDK tests."""

import pytest


@pytest.fixture(scope="session")
def vis_instance():  # noqa: ANN201
    """Provide VIS instance for all tests."""
    from vista_sdk.vis import VIS

    return VIS().instance


@pytest.fixture(scope="session")
def benchmark_versions():  # noqa: ANN201
    """Provide versions for benchmark tests."""
    from vista_sdk.vis_version import VisVersion

    return [VisVersion.v3_5a, VisVersion.v3_6a, VisVersion.v3_7a, VisVersion.v3_8a]


def pytest_configure(config):  # noqa: ANN201, ANN001
    """Configure custom markers."""
    config.addinivalue_line("markers", "benchmark: mark test as a benchmark test")
    config.addinivalue_line("markers", "load_test: mark test as a load test")


@pytest.fixture(scope="session")
def large_dataset_config():  # noqa: ANN201
    """Configuration for large dataset tests."""
    return {"path_count": 10000, "batch_size": 1000, "max_workers": 4}
