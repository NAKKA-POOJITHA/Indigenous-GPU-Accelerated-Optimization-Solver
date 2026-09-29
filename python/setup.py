from setuptools import setup, find_packages

setup(
    name="hunters_solver",
    version="0.1.0",
    description="Hunters Indigenous Optimization Solver — Sovereign Alternative to CPLEX/Xpress",
    author="Team Hunters",
    packages=find_packages(),
    python_requires=">=3.8",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Topic :: Scientific/Engineering :: Mathematics",
    ],
)
