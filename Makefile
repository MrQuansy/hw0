# NOTE: on MacOS you need to add an addition flag: -undefined dynamic_lookup
default:
	c++ -O3 -Wall -shared -std=c++14 -fPIC -undefined dynamic_lookup $$(python3 -m pybind11 --includes) src/simple_ml_ext.cpp -o src/simple_ml_ext.so
