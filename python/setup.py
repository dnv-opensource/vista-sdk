from setuptools import setup, find_packages

setup(
    name="vista-sdk",
    version="0.0.1",
    author="Anders Fredriksen",
    author_email="anders.fredriksen@dnv.com",
    description="SDKs and tools relating to DNVs Vessel Information Structure (VIS), ISO 19847, ISO 19848 standards",
    url="https://github.com/dnv-opensource/vista-sdk",
    license="MIT",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.8',
)
