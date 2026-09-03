from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        name="nova_engine",
        sources=["src/OrderBook.cpp", "src/bindings.cpp"],
        include_dirs=[
            "include",
            pybind11.get_include(),
        ],
        language="c++",
        extra_compile_args=["-std=c++20", "-O3"],
    ),
]

setup(
    name="nova_engine",
    version="0.1.0",
    ext_modules=ext_modules,
)